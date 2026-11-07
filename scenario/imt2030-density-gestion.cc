/*
 * IMT-2030 (echelle reduite) : zone dense Wi-Fi + gestion heuristique de charge.
 *
 * Politiques :
 *   baseline  - tous les terminaux emettent a debit fixe
 *   adaptive  - le controleur coupes/relance des emissions si la charge AP depasse un seuil
 *
 * Sortie : CSV de synthese dans outputDir (defaut ../projet-visuel-imt2030/sorties/resultats.csv)
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
    Ptr<Application> clientApp;
    uint32_t id;
    bool throttled;
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

static uint64_t
BytesOfferedEstimate()
{
    uint64_t sum = 0;
    for (const auto& s : g_stas)
    {
        sum += s.packetsOffered;
    }
    return sum * g_packetSize;
}

static void
ThrottleSta(StaCtx& s, bool throttle)
{
    if (!s.clientApp)
    {
        return;
    }
    ApplicationContainer one;
    one.Add(s.clientApp);

    if (throttle && !s.throttled)
    {
        one.Stop(Simulator::Now());
        s.throttled = true;
        ++g_throttleEvents;
    }
    else if (!throttle && s.throttled)
    {
        one.Start(Simulator::Now());
        one.Stop(Seconds(g_simTimeEnd));
        s.throttled = false;
        ++g_throttleEvents;
    }
}

static void
DensityControlStep()
{
    if (g_policy == "baseline")
    {
        return;
    }

    uint32_t activeCount = 0;
    uint32_t throttledCount = 0;
    for (const auto& s : g_stas)
    {
        if (s.throttled)
        {
            ++throttledCount;
        }
        else
        {
            ++activeCount;
        }
    }

    const double intervalSec = std::max(1e-6, static_cast<double>(g_intervalMs) / 1000.0);
    const double pktBits = static_cast<double>(g_packetSize) * 8.0;
    const double offeredBps = activeCount * (pktBits / intervalSec);
    const double rho = offeredBps / static_cast<double>(g_capacityBps);

    if (rho > g_highUtil)
    {
        // Couper un quart des terminaux actifs (round-robin par id de fenetre)
        const double win = g_measureWindow.GetSeconds();
        const uint32_t epoch = static_cast<uint32_t>(Simulator::Now().GetSeconds() / win);
        for (auto& s : g_stas)
        {
            if (!s.throttled && ((s.id + epoch) % 4 == 0))
            {
                ThrottleSta(s, true);
            }
        }
    }
    else if (rho < g_lowUtil && throttledCount > 0)
    {
        for (auto& s : g_stas)
        {
            if (s.throttled)
            {
                ThrottleSta(s, false);
                break;
            }
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
        out << "timestamp,policy,gridSide,nSta,simTime_s,capacity_mbps,bytesRx,bytesOfferedEst,"
               "deliveryRatio,throttleEvents,jainFairness,flows\n";
    }
    out << line << "\n";
}

int
main(int argc, char* argv[])
{
    uint32_t gridSide = 8;
    double simTime = 15.0;
    std::string policy = "adaptive";
    std::string outputDir = "../projet-visuel-imt2030/sorties";
    uint32_t packetSize = 512;
    uint32_t packetsPerSta = 40;
    uint32_t intervalMs = 80;
    double capacityMbps = 8.0;
    bool enableAnim = true;

    CommandLine cmd(__FILE__);
    cmd.AddValue("gridSide", "Cote de la grille (nSta = gridSide^2)", gridSide);
    cmd.AddValue("simTime", "Duree de simulation (s)", simTime);
    cmd.AddValue("policy", "Politique : baseline | adaptive", policy);
    cmd.AddValue("outputDir", "Repertoire de sortie", outputDir);
    cmd.AddValue("packetSize", "Taille UDP (octets)", packetSize);
    cmd.AddValue("packetsPerSta", "Paquets max par STA", packetsPerSta);
    cmd.AddValue("intervalMs", "Intervalle entre paquets (ms)", intervalMs);
    cmd.AddValue("capacityMbps", "Capacite de reference pour rho (Mbit/s)", capacityMbps);
    cmd.AddValue("highUtil", "Seuil haut rho", g_highUtil);
    cmd.AddValue("lowUtil", "Seuil bas rho", g_lowUtil);
    cmd.AddValue("enableAnim", "Exporter XML NetAnim (1/0)", enableAnim);
    cmd.Parse(argc, argv);

    if (policy != "baseline" && policy != "adaptive")
    {
        std::cerr << "policy doit etre baseline ou adaptive\n";
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
        UdpClientHelper client(apAddr, port);
        client.SetAttribute("MaxPackets", UintegerValue(packetsPerSta));
        client.SetAttribute("Interval", TimeValue(MilliSeconds(intervalMs)));
        client.SetAttribute("PacketSize", UintegerValue(packetSize));
        ApplicationContainer one = client.Install(staNodes.Get(i));
        one.Start(Seconds(1.0 + (i % 7) * 0.05));
        one.Stop(Seconds(simTime));

        StaCtx ctx;
        ctx.node = staNodes.Get(i);
        ctx.clientApp = one.Get(0);
        ctx.id = i;
        ctx.throttled = false;
        ctx.packetsOffered = packetsPerSta;
        g_stas.push_back(ctx);
    }

    FlowMonitorHelper fmh;
    Ptr<FlowMonitor> monitor = fmh.InstallAll();

    if (policy == "adaptive")
    {
        Simulator::Schedule(Seconds(1.5), &DensityControlStep);
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
    std::cout << "Flux IPv4 suivis      : " << stats.size() << "\n\n";

    const std::string csvPath = outputDir + "/resultats.csv";
    std::ostringstream row;
    row << std::fixed << std::setprecision(4) << Simulator::Now().GetSeconds() << ',' << policy
        << ',' << gridSide << ',' << nSta << ',' << simTime << ',' << capacityMbps << ','
        << bytesRx << ',' << bytesOffered << ',' << delivery << ',' << g_throttleEvents << ','
        << jain << ',' << stats.size();
    AppendCsvRow(csvPath, row.str(), true);

    std::cout << "CSV : " << csvPath << "\n";

    Simulator::Destroy();
    return 0;
}
