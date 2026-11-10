/*
 * Lab 4 : meme trafic et memes politiques que le proxy Wi-Fi, sur une cellule NR.
 *
 * Module : 5G-LENA v3.3.1 (etiquette prevue pour ns-3.42), compile dans cet arbre ns-3.43.
 * Radio figee avant la campagne :
 *   3,5 GHz, 20 MHz, numerologie 0, interieur bureau en visibilite, ombrage coupe,
 *   planificateur OFDMA round-robin, motif TDD a majorite montante.
 * La capacite de 3 Mbit/s reste le parametre de la loi K. Elle ne regle pas la bande NR.
 *
 * Politiques : baseline, adaptive, feedback. Memes seuils et meme budget que les Labs 3 et 5.
 */

#include "ns3/antenna-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/nr-module.h"
#include "ns3/point-to-point-module.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Imt2030NrGestion");

struct UeCtx
{
    Ptr<Node> node;
    Ptr<Socket> socket;
    uint32_t id;
    bool throttled;
    uint32_t sent;
    uint64_t packetsOffered;
    uint64_t rxBytes;
};

static Ptr<PacketSink> g_sink;
static std::vector<UeCtx> g_ues;
static std::map<Ipv4Address, uint32_t> g_ueIdByAddr;
static std::string g_policy = "baseline";
static double g_highUtil = 0.82;
static double g_lowUtil = 0.55;
static uint64_t g_capacityBps = 3000000;
static uint32_t g_throttleEvents = 0;
static Time g_measureWindow = Seconds(0.5);
static uint32_t g_packetSize = 512;
static double g_simTimeEnd = 10.0;
static uint32_t g_intervalMs = 40;
static uint32_t g_feedbackCap = 1;
static uint64_t g_prevRx = 0;
static uint64_t g_prevSentPackets = 0;
static bool g_feedbackReady = false;
static double g_feedbackLow = 0.35;
static double g_feedbackHigh = 0.50;

static const double kFrequencyHz = 3.5e9;
static const double kBandwidthHz = 20e6;
static const uint16_t kNumerology = 0;
static const char* kPattern = "DL|S|UL|UL|UL|DL|S|UL|UL|UL|";

static uint64_t
BytesOfferedEstimate()
{
    uint64_t sum = 0;
    for (const auto& ue : g_ues)
    {
        sum += ue.sent;
    }
    return sum * g_packetSize;
}

static uint32_t
ComputeMaxActive()
{
    const double intervalSec = std::max(1e-6, static_cast<double>(g_intervalMs) / 1000.0);
    const double pktBits = static_cast<double>(g_packetSize) * 8.0;
    const double raw =
        static_cast<double>(g_capacityBps) * intervalSec / pktBits * g_highUtil;
    uint32_t cap = static_cast<uint32_t>(std::floor(raw));
    if (cap < 1)
    {
        cap = 1;
    }
    if (cap > g_ues.size())
    {
        cap = static_cast<uint32_t>(g_ues.size());
    }
    return cap;
}

static void
SendUe(uint32_t index)
{
    if (index >= g_ues.size() || Simulator::Now() >= Seconds(g_simTimeEnd))
    {
        return;
    }
    UeCtx& ue = g_ues[index];
    if (!ue.throttled && ue.sent < ue.packetsOffered && ue.socket)
    {
        Ptr<Packet> packet = Create<Packet>(g_packetSize);
        const int sent = ue.socket->Send(packet);
        if (sent > 0)
        {
            ++ue.sent;
        }
    }
    if (ue.sent < ue.packetsOffered &&
        Simulator::Now() + MilliSeconds(g_intervalMs) < Seconds(g_simTimeEnd))
    {
        Simulator::Schedule(MilliSeconds(g_intervalMs), &SendUe, index);
    }
}

static void
ApplyAdmission(uint32_t cap)
{
    for (auto& ue : g_ues)
    {
        const bool admit = ue.id < cap;
        if (admit && ue.throttled)
        {
            ue.throttled = false;
            ++g_throttleEvents;
        }
        else if (!admit && !ue.throttled)
        {
            ue.throttled = true;
            ++g_throttleEvents;
        }
    }
}

static uint64_t
CountSentPackets()
{
    uint64_t sum = 0;
    for (const auto& ue : g_ues)
    {
        sum += ue.sent;
    }
    return sum;
}

static void
DensityControlStep()
{
    if (g_policy == "adaptive")
    {
        const uint32_t cap = ComputeMaxActive();
        g_feedbackCap = cap;
        ApplyAdmission(cap);
    }
    else if (g_policy == "feedback")
    {
        if (!g_feedbackReady)
        {
            g_feedbackCap = ComputeMaxActive();
            ApplyAdmission(g_feedbackCap);
            g_prevRx = g_sink ? g_sink->GetTotalRx() : 0;
            g_prevSentPackets = CountSentPackets();
            g_feedbackReady = true;
        }
        else
        {
            const uint64_t rx = g_sink->GetTotalRx();
            const uint64_t sent = CountSentPackets();
            const uint64_t deltaRx = (rx >= g_prevRx) ? (rx - g_prevRx) : 0;
            const uint64_t deltaSent =
                (sent >= g_prevSentPackets) ? (sent - g_prevSentPackets) : 0;
            if (deltaSent > 0)
            {
                const double offered = static_cast<double>(deltaSent * g_packetSize);
                const double ratio = static_cast<double>(deltaRx) / offered;
                if (ratio < g_feedbackLow && g_feedbackCap > 1)
                {
                    const uint32_t step = std::max<uint32_t>(1, g_feedbackCap / 6);
                    g_feedbackCap = (g_feedbackCap > step) ? (g_feedbackCap - step) : 1;
                }
                else if (ratio > g_feedbackHigh && g_feedbackCap < g_ues.size())
                {
                    const uint32_t step = std::max<uint32_t>(1, g_feedbackCap / 12);
                    const uint32_t grown = g_feedbackCap + step;
                    g_feedbackCap = (grown < g_ues.size()) ? grown : static_cast<uint32_t>(g_ues.size());
                }
                ApplyAdmission(g_feedbackCap);
            }
            g_prevRx = rx;
            g_prevSentPackets = sent;
        }
    }
    Simulator::Schedule(g_measureWindow, &DensityControlStep);
}

static void
OnSinkRx(Ptr<const Packet> packet, const Address& address)
{
    if (!InetSocketAddress::IsMatchingType(address))
    {
        return;
    }
    const Ipv4Address source = InetSocketAddress::ConvertFrom(address).GetIpv4();
    const auto found = g_ueIdByAddr.find(source);
    if (found == g_ueIdByAddr.end())
    {
        return;
    }
    g_ues[found->second].rxBytes += packet->GetSize();
}

static double
JainFromRx()
{
    double sum = 0.0;
    double sumSq = 0.0;
    uint32_t n = 0;
    for (const auto& ue : g_ues)
    {
        if (ue.rxBytes == 0)
        {
            continue;
        }
        const double value = static_cast<double>(ue.rxBytes);
        sum += value;
        sumSq += value * value;
        ++n;
    }
    if (n == 0 || sumSq <= 0.0)
    {
        return 0.0;
    }
    const double count = static_cast<double>(n);
    return (sum * sum) / (count * sumSq);
}

static uint32_t
CountSendingUes()
{
    uint32_t n = 0;
    for (const auto& ue : g_ues)
    {
        if (ue.sent > 0)
        {
            ++n;
        }
    }
    return n;
}

static void
WriteTopologyCsv(const std::string& path,
                 const NodeContainer& ueNodes,
                 const NodeContainer& gnbNode,
                 const std::string& policy,
                 uint32_t gridSide)
{
    std::ofstream out(path);
    if (!out)
    {
        return;
    }
    out << "node_id,role,x_m,y_m,z_m,policy,gridSide\n";
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mob = ueNodes.Get(i)->GetObject<MobilityModel>();
        const Vector& p = mob->GetPosition();
        out << ueNodes.Get(i)->GetId() << ",UE," << p.x << "," << p.y << "," << p.z << ","
            << policy << "," << gridSide << "\n";
    }
    Ptr<MobilityModel> gnbMob = gnbNode.Get(0)->GetObject<MobilityModel>();
    const Vector& g = gnbMob->GetPosition();
    out << gnbNode.Get(0)->GetId() << ",GNB," << g.x << "," << g.y << "," << g.z << "," << policy
        << "," << gridSide << "\n";
}

static void
AppendCsvRow(const std::string& path, const std::string& line, bool headerIfMissing)
{
    const bool exists = std::ifstream(path).good();
    std::ofstream out(path, std::ios::app);
    if (!out)
    {
        std::cerr << "Impossible d'ecrire " << path << "\n";
        return;
    }
    if (headerIfMissing && !exists)
    {
        out << "timestamp,policy,gridSide,nSta,simTime_s,intervalMs,capacity_mbps,bytesRx,"
               "bytesOfferedEst,deliveryRatio,throttleEvents,jainFairness,flows,finalCap,"
               "bandwidth_mhz,numerology\n";
    }
    out << line << "\n";
}

int
main(int argc, char* argv[])
{
    uint32_t gridSide = 4;
    double simTime = 10.0;
    std::string policy = "baseline";
    std::string outputDir = "../projet-visuel-imt2030/labs/lab04_5g_lena/sorties";
    uint32_t packetSize = 512;
    uint32_t packetsPerSta = 40;
    uint32_t intervalMs = 40;
    double capacityMbps = 3.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("gridSide", "Cote de la grille (nSta = gridSide^2)", gridSide);
    cmd.AddValue("simTime", "Duree de simulation (s)", simTime);
    cmd.AddValue("policy", "Politique : baseline | adaptive | feedback", policy);
    cmd.AddValue("feedbackLow", "Seuil bas de livraison pour reduire K", g_feedbackLow);
    cmd.AddValue("feedbackHigh", "Seuil haut de livraison pour augmenter K", g_feedbackHigh);
    cmd.AddValue("outputDir", "Repertoire de sortie", outputDir);
    cmd.AddValue("packetSize", "Taille UDP (octets)", packetSize);
    cmd.AddValue("packetsPerSta", "Paquets max par UE", packetsPerSta);
    cmd.AddValue("intervalMs", "Intervalle entre paquets (ms)", intervalMs);
    cmd.AddValue("capacityMbps", "Capacite de reference pour la loi K (Mbit/s)", capacityMbps);
    cmd.AddValue("highUtil", "Seuil haut utilise par la loi K", g_highUtil);
    cmd.AddValue("lowUtil", "Parametre conserve, non utilise par les lois", g_lowUtil);
    cmd.Parse(argc, argv);

    if (policy != "baseline" && policy != "adaptive" && policy != "feedback")
    {
        std::cerr << "policy doit etre baseline, adaptive ou feedback\n";
        return 1;
    }
    if (gridSide < 2)
    {
        std::cerr << "gridSide >= 2 requis\n";
        return 1;
    }

    g_policy = policy;
    g_capacityBps = static_cast<uint64_t>(capacityMbps * 1e6);
    g_packetSize = packetSize;
    g_simTimeEnd = simTime;
    g_intervalMs = intervalMs;

    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));

    const uint32_t nUe = gridSide * gridSide;
    const double spacing = 5.0;

    NodeContainer gnbNode;
    gnbNode.Create(1);
    NodeContainer ueNodes;
    ueNodes.Create(nUe);
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);

    MobilityHelper ueMob;
    ueMob.SetPositionAllocator("ns3::GridPositionAllocator",
                               "MinX",
                               DoubleValue(0.0),
                               "MinY",
                               DoubleValue(0.0),
                               "DeltaX",
                               DoubleValue(spacing),
                               "DeltaY",
                               DoubleValue(spacing),
                               "GridWidth",
                               UintegerValue(gridSide),
                               "LayoutType",
                               StringValue("RowFirst"));
    ueMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    ueMob.Install(ueNodes);
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mob = ueNodes.Get(i)->GetObject<MobilityModel>();
        Vector position = mob->GetPosition();
        position.z = 1.5;
        mob->SetPosition(position);
    }

    MobilityHelper gnbMob;
    gnbMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    gnbMob.Install(gnbNode);
    const double cx = (gridSide - 1) * spacing / 2.0;
    gnbNode.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(cx, -spacing, 3.0));

    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> beamforming = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    nrHelper->SetBeamformingHelper(beamforming);
    nrHelper->SetEpcHelper(epcHelper);
    nrHelper->SetAttribute("EnableMimoFeedback", BooleanValue(false));
    nrHelper->SetSchedulerTypeId(NrMacSchedulerOfdmaRR::GetTypeId());
    beamforming->SetAttribute("BeamformingMethod",
                              TypeIdValue(DirectPathBeamforming::GetTypeId()));
    epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));
    nrHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

    CcBwpCreator ccBwpCreator;
    CcBwpCreator::SimpleOperationBandConf bandConf(kFrequencyHz,
                                                   kBandwidthHz,
                                                   1,
                                                   BandwidthPartInfo::InH_OfficeOpen_LoS);
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    nrHelper->InitializeOperationBand(&band);
    BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps({band});

    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(1));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(1));
    nrHelper->SetUeAntennaAttribute("AntennaElement",
                                    PointerValue(CreateObject<IsotropicAntennaModel>()));
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(1));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(1));
    nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                     PointerValue(CreateObject<IsotropicAntennaModel>()));
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(23.0));
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("NGBR_VIDEO_TCP_DEFAULT", UintegerValue(0));
    nrHelper->SetUeBwpManagerAlgorithmAttribute("NGBR_VIDEO_TCP_DEFAULT", UintegerValue(0));

    NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice(gnbNode, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueNodes, allBwps);
    int64_t stream = 1;
    stream += nrHelper->AssignStreams(gnbNetDev, stream);
    stream += nrHelper->AssignStreams(ueNetDev, stream);

    nrHelper->GetGnbPhy(gnbNetDev.Get(0), 0)->SetAttribute("Numerology", UintegerValue(kNumerology));
    nrHelper->GetGnbPhy(gnbNetDev.Get(0), 0)->SetAttribute("TxPower", DoubleValue(30.0));
    nrHelper->GetGnbPhy(gnbNetDev.Get(0), 0)->SetAttribute("Pattern", StringValue(kPattern));
    nrHelper->UpdateDeviceConfigs(gnbNetDev);
    nrHelper->UpdateDeviceConfigs(ueNetDev);

    InternetStackHelper internet;
    internet.Install(remoteHostContainer);
    Ptr<Node> pgw = epcHelper->GetPgwNode();
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(2500));
    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.0)));
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
    const Ipv4Address remoteAddr = internetIpIfaces.GetAddress(1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(ueNetDev);
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
    {
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(ueNodes.Get(i)->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
        g_ueIdByAddr[ueIpIface.GetAddress(i)] = i;
    }

    nrHelper->AttachToClosestGnb(ueNetDev, gnbNetDev);

    const uint16_t port = 7777;
    PacketSinkHelper sink("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer serverApp = sink.Install(remoteHost);
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(simTime + 0.5));
    g_sink = DynamicCast<PacketSink>(serverApp.Get(0));
    g_sink->TraceConnectWithoutContext("Rx", MakeCallback(&OnSinkRx));

    g_ues.reserve(nUe);
    for (uint32_t i = 0; i < nUe; ++i)
    {
        Ptr<Socket> sock = Socket::CreateSocket(ueNodes.Get(i), UdpSocketFactory::GetTypeId());
        sock->Connect(InetSocketAddress(remoteAddr, port));

        UeCtx ctx;
        ctx.node = ueNodes.Get(i);
        ctx.socket = sock;
        ctx.id = i;
        ctx.throttled = false;
        ctx.sent = 0;
        ctx.packetsOffered = packetsPerSta;
        ctx.rxBytes = 0;
        g_ues.push_back(ctx);

        const double stagger = static_cast<double>(i) * static_cast<double>(intervalMs) / 1000.0 /
                               static_cast<double>(nUe);
        Simulator::Schedule(Seconds(1.0 + stagger), &SendUe, i);
    }

    if (policy == "adaptive" || policy == "feedback")
    {
        Simulator::Schedule(Seconds(0.5), &DensityControlStep);
    }

    const std::string topoPath = outputDir + "/topologie.csv";
    WriteTopologyCsv(topoPath, ueNodes, gnbNode, policy, gridSide);

    std::cout << "\n=== Lab 4 cellule NR ===\n"
              << "Terminaux : " << nUe << "  (" << gridSide << "x" << gridSide << ")\n"
              << "Politique : " << policy << "  |  duree : " << simTime << " s\n"
              << "Porteuse : " << (kFrequencyHz / 1e9) << " GHz, bande "
              << (kBandwidthHz / 1e6) << " MHz, numerologie " << kNumerology << "\n"
              << "Motif : " << kPattern << "\n\n";

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    const uint64_t bytesRx = g_sink->GetTotalRx();
    const uint64_t bytesOffered = BytesOfferedEstimate();
    const double delivery =
        (bytesOffered > 0) ? static_cast<double>(bytesRx) / static_cast<double>(bytesOffered) : 0.0;
    const double jain = JainFromRx();
    const uint32_t flows = CountSendingUes();
    const uint32_t finalCap = (policy == "baseline") ? nUe : g_feedbackCap;

    std::cout << "Octets recus           : " << bytesRx << "\n";
    std::cout << "Offre estimee (octets) : " << bytesOffered << "\n";
    std::cout << "Taux de livraison      : " << std::fixed << std::setprecision(3) << delivery
              << "\n";
    std::cout << "Evenements throttle    : " << g_throttleEvents << "\n";
    std::cout << "Indice Jain            : " << std::setprecision(3) << jain << "\n";
    std::cout << "Flux emetteurs         : " << flows << "\n";
    std::cout << "Plafond final          : " << finalCap << "\n\n";

    const std::string csvPath = outputDir + "/resultats.csv";
    std::ostringstream row;
    row << std::fixed << std::setprecision(4) << Simulator::Now().GetSeconds() << ',' << policy
        << ',' << gridSide << ',' << nUe << ',' << simTime << ',' << intervalMs << ','
        << capacityMbps << ',' << bytesRx << ',' << bytesOffered << ',' << delivery << ','
        << g_throttleEvents << ',' << jain << ',' << flows << ',' << finalCap << ','
        << (kBandwidthHz / 1e6) << ',' << kNumerology;
    AppendCsvRow(csvPath, row.str(), true);
    std::cout << "CSV : " << csvPath << "\n";

    Simulator::Destroy();
    return 0;
}
