/*
 * IMT-2030 (echelle reduite) : zone dense Wi-Fi + gestion heuristique de charge.
 *
 * Politiques :
 *   baseline  - tous les terminaux emettent a debit fixe
 *   adaptive  - au plus K terminaux emettent, K fixe calcule sur la capacite de reference
 *   feedback  - K part de la meme valeur, puis monte ou descend selon la livraison mesuree
 *
 * Sortie : CSV dans outputDir (defaut lab01 sorties, voir README lab01)
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Imt2030DensityGestion");

struct StaCtx
{
    Ptr<Node> node;
    Ptr<Socket> socket;
    uint32_t id;
    bool throttled;
    uint32_t sent;
    uint64_t packetsOffered;
};

static Ptr<PacketSink> g_sink;
static std::vector<StaCtx> g_stas;
static std::string g_policy = "adaptive";
static double g_highUtil = 0.82;
static double g_lowUtil = 0.55;
static uint64_t g_capacityBps = 8000000; // ~8 Mbit/s ordre de grandeur (heuristique)
static uint32_t g_throttleEvents = 0;
static Time g_measureWindow = Seconds(0.5);
static uint32_t g_packetSize = 512;
static double g_simTimeEnd = 15.0;
static uint32_t g_intervalMs = 80;
static uint32_t g_feedbackCap = 1;
static uint64_t g_prevRx = 0;
static uint64_t g_prevSentPackets = 0;
static bool g_feedbackReady = false;
static double g_feedbackLow = 0.35;
static double g_feedbackHigh = 0.50;

static uint64_t
BytesOfferedEstimate()
{
    uint64_t sum = 0;
    for (const auto& s : g_stas)
    {
        sum += s.sent;
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
    if (cap > g_stas.size())
    {
        cap = static_cast<uint32_t>(g_stas.size());
    }
    return cap;
}

static void
SendSta(uint32_t index)
{
    if (index >= g_stas.size() || Simulator::Now() >= Seconds(g_simTimeEnd))
    {
        return;
    }
    StaCtx& sta = g_stas[index];
    if (!sta.throttled && sta.sent < sta.packetsOffered && sta.socket)
    {
        sta.socket->Send(Create<Packet>(g_packetSize));
        ++sta.sent;
    }
    if (sta.sent < sta.packetsOffered &&
        Simulator::Now() + MilliSeconds(g_intervalMs) < Seconds(g_simTimeEnd))
    {
        Simulator::Schedule(MilliSeconds(g_intervalMs), &SendSta, index);
    }
}

static void
ApplyAdmission(uint32_t cap)
{
    for (auto& sta : g_stas)
    {
        const bool admit = sta.id < cap;
        if (admit && sta.throttled)
        {
            sta.throttled = false;
            ++g_throttleEvents;
        }
        else if (!admit && !sta.throttled)
        {
            sta.throttled = true;
            ++g_throttleEvents;
        }
    }
}

static uint64_t
CountSentPackets()
{
    uint64_t sum = 0;
    for (const auto& sta : g_stas)
    {
        sum += sta.sent;
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
                else if (ratio > g_feedbackHigh && g_feedbackCap < g_stas.size())
                {
                    const uint32_t step = std::max<uint32_t>(1, g_feedbackCap / 12);
                    const uint32_t grown = g_feedbackCap + step;
                    g_feedbackCap = (grown < g_stas.size()) ? grown : static_cast<uint32_t>(g_stas.size());
                }
                ApplyAdmission(g_feedbackCap);
            }
            g_prevRx = rx;
            g_prevSentPackets = sent;
        }
    }
    Simulator::Schedule(g_measureWindow, &DensityControlStep);
}

static double
JainFairness(const std::map<FlowId, FlowMonitor::FlowStats>& stats)
{
    std::vector<double> rx;
    for (const auto& kv : stats)
    {
        if (kv.second.rxBytes > 0)
        {
            rx.push_back(static_cast<double>(kv.second.rxBytes));
        }
    }
    if (rx.empty())
    {
        return 0.0;
    }
    double sum = 0.0;
    double sumSq = 0.0;
    for (double x : rx)
    {
        sum += x;
        sumSq += x * x;
    }
    const double n = static_cast<double>(rx.size());
    return (sumSq > 0) ? (sum * sum) / (n * sumSq) : 0.0;
}

static void
WriteTopologyCsv(const std::string& path,
                 const NodeContainer& staNodes,
                 const NodeContainer& apNode,
                 const std::string& policy,
                 uint32_t gridSide)
{
    std::ofstream out(path);
    if (!out)
    {
        return;
    }
    out << "node_id,role,x_m,y_m,policy,gridSide\n";
    for (uint32_t i = 0; i < staNodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mob = staNodes.Get(i)->GetObject<MobilityModel>();
        const Vector& p = mob->GetPosition();
        out << staNodes.Get(i)->GetId() << ",STA," << p.x << "," << p.y << "," << policy << ","
            << gridSide << "\n";
    }
    for (uint32_t i = 0; i < apNode.GetN(); ++i)
    {
        Ptr<MobilityModel> mob = apNode.Get(i)->GetObject<MobilityModel>();
        const Vector& p = mob->GetPosition();
        out << apNode.Get(i)->GetId() << ",AP," << p.x << "," << p.y << "," << policy << ","
            << gridSide << "\n";
    }
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
        out << "timestamp,policy,gridSide,nSta,simTime_s,intervalMs,capacity_mbps,bytesRx,bytesOfferedEst,"
               "deliveryRatio,throttleEvents,jainFairness,flows,finalCap\n";
    }
    out << line << "\n";
}

int
main(int argc, char* argv[])
{
    uint32_t gridSide = 8;
    double simTime = 15.0;
    std::string policy = "adaptive";
    std::string outputDir = "../projet-visuel-imt2030/labs/lab01_wifi_dense/sorties";
    uint32_t packetSize = 512;
    uint32_t packetsPerSta = 40;
    uint32_t intervalMs = 80;
    double capacityMbps = 8.0;
    bool enableAnim = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("gridSide", "Cote de la grille (nSta = gridSide^2)", gridSide);
    cmd.AddValue("simTime", "Duree de simulation (s)", simTime);
    cmd.AddValue("policy", "Politique : baseline | adaptive | feedback", policy);
    cmd.AddValue("feedbackLow", "Seuil bas de livraison pour reduire K", g_feedbackLow);
    cmd.AddValue("feedbackHigh", "Seuil haut de livraison pour augmenter K", g_feedbackHigh);
    cmd.AddValue("outputDir", "Repertoire de sortie", outputDir);
    cmd.AddValue("packetSize", "Taille UDP (octets)", packetSize);
    cmd.AddValue("packetsPerSta", "Paquets max par STA", packetsPerSta);
    cmd.AddValue("intervalMs", "Intervalle entre paquets (ms)", intervalMs);
    cmd.AddValue("capacityMbps", "Capacite de reference pour rho (Mbit/s)", capacityMbps);
    cmd.AddValue("highUtil", "Seuil haut rho", g_highUtil);
    cmd.AddValue("lowUtil", "Seuil bas rho", g_lowUtil);
    cmd.AddValue("enableAnim", "Exporter XML NetAnim (1/0)", enableAnim);
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

    const uint32_t nSta = gridSide * gridSide;
    const double spacing = 5.0;

    NodeContainer apNode;
    apNode.Create(1);
    NodeContainer staNodes;
    staNodes.Create(nSta);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("HtMcs7"),
                                 "ControlMode",
                                 StringValue("HtMcs0"));

    WifiMacHelper mac;
    Ssid ssid("imt2030-dense-zone");
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices = wifi.Install(phy, mac, staNodes);
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices = wifi.Install(phy, mac, apNode);

    InternetStackHelper stack;
    stack.Install(apNode);
    stack.Install(staNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.96.0.0", "255.255.255.0");
    Ipv4InterfaceContainer apIf = address.Assign(apDevices);
    address.Assign(staDevices);
    const Ipv4Address apAddr = apIf.GetAddress(0);

    MobilityHelper staMob;
    staMob.SetPositionAllocator("ns3::GridPositionAllocator",
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
    staMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    staMob.Install(staNodes);

    MobilityHelper apMob;
    apMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    apMob.Install(apNode);
    const double cx = (gridSide - 1) * spacing / 2.0;
    apNode.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(cx, -spacing, 0.0));

    const uint16_t port = 7777;
    PacketSinkHelper sink("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer serverApp = sink.Install(apNode.Get(0));
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(simTime + 1.0));
    g_sink = DynamicCast<PacketSink>(serverApp.Get(0));

    g_stas.reserve(nSta);
    for (uint32_t i = 0; i < nSta; ++i)
    {
        Ptr<Socket> sock = Socket::CreateSocket(staNodes.Get(i), UdpSocketFactory::GetTypeId());
        sock->Connect(InetSocketAddress(apAddr, port));

        StaCtx ctx;
        ctx.node = staNodes.Get(i);
        ctx.socket = sock;
        ctx.id = i;
        ctx.throttled = false;
        ctx.sent = 0;
        ctx.packetsOffered = packetsPerSta;
        g_stas.push_back(ctx);
        const double stagger = static_cast<double>(i) * static_cast<double>(intervalMs) /
                               1000.0 / static_cast<double>(nSta);
        Simulator::Schedule(Seconds(1.0 + stagger), &SendSta, i);
    }

    FlowMonitorHelper fmh;
    Ptr<FlowMonitor> monitor = fmh.InstallAll();

    if (policy == "adaptive" || policy == "feedback")
    {
        Simulator::Schedule(Seconds(0.5), &DensityControlStep);
    }

    const std::string topoPath = outputDir + "/topologie.csv";
    WriteTopologyCsv(topoPath, staNodes, apNode, policy, gridSide);

    std::unique_ptr<AnimationInterface> anim;
    if (enableAnim)
    {
        const std::string animPath = outputDir + "/imt2030-animation.xml";
        anim = std::make_unique<AnimationInterface>(animPath);
        anim->UpdateNodeDescription(apNode.Get(0), "AP");
        anim->UpdateNodeColor(apNode.Get(0), 46, 139, 87);
        for (uint32_t i = 0; i < staNodes.GetN(); ++i)
        {
            anim->UpdateNodeDescription(staNodes.Get(i), "STA");
            anim->UpdateNodeColor(staNodes.Get(i), 220, 80, 60);
        }
        anim->EnablePacketMetadata();
        anim->EnableWifiMacCounters(Seconds(0), Seconds(simTime));
        std::cout << "NetAnim : " << animPath << "\n";
        std::cout << "Topologie : " << topoPath << "\n";
    }

    std::cout << "\n=== IMT-2030 zone dense (proxy Wi-Fi) ===\n"
              << "Terminaux : " << nSta << "  (" << gridSide << "x" << gridSide << ")\n"
              << "Politique : " << policy << "  |  duree : " << simTime << " s\n\n";

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();
    const auto stats = monitor->GetFlowStats();
    const double jain = JainFairness(stats);

    const uint64_t bytesRx = g_sink->GetTotalRx();
    const uint64_t bytesOffered = BytesOfferedEstimate();
    const double delivery = (bytesOffered > 0)
                                ? static_cast<double>(bytesRx) / static_cast<double>(bytesOffered)
                                : 0.0;

    std::cout << "Octets recus (AP)     : " << bytesRx << "\n";
    std::cout << "Offre estimee (octets): " << bytesOffered << "\n";
    std::cout << "Taux de livraison     : " << std::fixed << std::setprecision(3) << delivery
              << "\n";
    std::cout << "Evenements throttle   : " << g_throttleEvents << "\n";
    std::cout << "Indice Jain (debit)   : " << std::setprecision(3) << jain << "\n";
    std::cout << "Flux IPv4 suivis      : " << stats.size() << "\n";
    std::cout << "Plafond final         : "
              << ((policy == "baseline") ? nSta : g_feedbackCap) << "\n\n";

    const std::string csvPath = outputDir + "/resultats.csv";
    std::ostringstream row;
    row << std::fixed << std::setprecision(4) << Simulator::Now().GetSeconds() << ',' << policy
        << ',' << gridSide << ',' << nSta << ',' << simTime << ',' << intervalMs << ','
        << capacityMbps << ',' << bytesRx << ',' << bytesOffered << ',' << delivery << ','
        << g_throttleEvents << ',' << jain << ',' << stats.size() << ','
        << ((policy == "baseline") ? nSta : g_feedbackCap);
    AppendCsvRow(csvPath, row.str(), true);

    std::cout << "CSV : " << csvPath << "\n";

    Simulator::Destroy();
    return 0;
}
