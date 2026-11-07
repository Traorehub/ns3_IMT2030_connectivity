/*
 * Demo ns-3 minimale : 1 point d'acces Wi-Fi + N terminaux qui envoient des UDP.
 * A la fin : affichage console du nombre d'octets recus sur l'AP.
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"

#include <iostream>

using namespace ns3;

int
main(int argc, char* argv[])
{
    uint32_t nSta = 4;
    double simTime = 5.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nSta", "Nombre de terminaux Wi-Fi", nSta);
    cmd.AddValue("simTime", "Duree (secondes)", simTime);
    cmd.Parse(argc, argv);

    if (nSta < 1)
    {
        std::cerr << "nSta doit etre >= 1\n";
        return 1;
    }

    NodeContainer apNode;
    apNode.Create(1);
    NodeContainer staNodes;
    staNodes.Create(nSta);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    WifiMacHelper mac;
    Ssid ssid("demo-hotzone");

    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices = wifi.Install(phy, mac, staNodes);

    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices = wifi.Install(phy, mac, apNode);

    InternetStackHelper stack;
    stack.Install(apNode);
    stack.Install(staNodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.0.0", "255.255.255.0");
    Ipv4InterfaceContainer staIfaces = address.Assign(staDevices);
    Ipv4InterfaceContainer apIfaces = address.Assign(apDevices);
    Ipv4Address apAddr = apIfaces.GetAddress(0);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNode);
    mobility.Install(staNodes);

    apNode.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(0.0, 0.0, 0.0));
    for (uint32_t i = 0; i < nSta; ++i)
    {
        double x = 5.0 + static_cast<double>(i) * 3.0;
        staNodes.Get(i)->GetObject<MobilityModel>()->SetPosition(Vector(x, 0.0, 0.0));
    }

    const uint16_t port = 5000;
    PacketSinkHelper sink("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer serverApp = sink.Install(apNode.Get(0));
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(simTime));

    ApplicationContainer clientApps;
    for (uint32_t i = 0; i < nSta; ++i)
    {
        UdpClientHelper client(apAddr, port);
        client.SetAttribute("MaxPackets", UintegerValue(20));
        client.SetAttribute("Interval", TimeValue(MilliSeconds(100)));
        client.SetAttribute("PacketSize", UintegerValue(512));
        ApplicationContainer one = client.Install(staNodes.Get(i));
        one.Start(Seconds(1.0 + 0.1 * i));
        one.Stop(Seconds(simTime));
        clientApps.Add(one);
    }

    std::cout << "Demo : " << nSta << " STA -> 1 AP, " << simTime << " s\n";

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    Ptr<PacketSink> sinkApp = DynamicCast<PacketSink>(serverApp.Get(0));
    std::cout << "Octets recus sur l'AP : " << sinkApp->GetTotalRx() << "\n";

    Simulator::Destroy();
    return 0;
}
