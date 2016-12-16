/*!
 *  Project : WIFI standards comparision using ns-3
 *  File    : wifi_comparision.cc
 *  Authors : Nithesh Nagaraj, Nikita Mirchandani, Romil Shah
 *  Course  : EECE 7364 
 *
 *  Description : Wifi standards comparisions with Wifi topology consisting of 1 AP 
 *  and 12 STA nodes working in Insfrastructure mode
 */

#include "ns3/core-module.h"
#include "ns3/propagation-module.h"
#include "ns3/simulator.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WIFI_Comparisions");

/*  Choose the required Wifi standard and set the required 
 *  WifiMacHelper in step 3 and accordingly
/////////////////////////////////////////////////////////////////////////////////////
  WIFI Standards                        Data Rates

  WIFI_PHY_STANDARD_80211a              OfdmRate6Mbps, OfdmRate12Mbps
  WIFI_PHY_STANDARD_80211b              DsssRate5_5Mbps, DsssRate11Mbps
  WIFI_PHY_STANDARD_80211g              ErpOfdmRate6Mbps, ErpOfdmRate12Mbps
  WIFI_PHY_STANDARD_80211n_2_4GHZ       OfdmRate6_5MbpsBW20MHz, OfdmRate13MbpsBW20MHz
  WIFI_PHY_STANDARD_80211n_5GHZ         OfdmRate13_5MbpsBW40MHz

*////////////////////////////////////////////////////////////////////////////////////

//  Set the parameters
WifiPhyStandard	wifi_standard_used   = WIFI_PHY_STANDARD_80211n_2_4GHZ;
std::string		data_rate_for_wifi   = "OfdmRate6_5MbpsBW20MHz";
std::string		wifi_rate_adaptation = "ns3::ConstantRateWifiManager";
std::string		data_rate_for_client = "1Mbps";
std::string		client_data_protocol = "ns3::UdpSocketFactory";
uint16_t		destination_port 	 = 10;
uint16_t	    client_pkt_size      = 1400;
double			distance_loss_value  = 3.2;

bool isRtsCts		= false;    // Enable RTS/CTS
bool isTraceEnabled = false;    // Enable detailed trace

/*

Usage : ./waf --run "wifi_comparision --isRtsCts=false --wifiStaNodesCount=12 --activeStaNodesCount=12 --data_rate_for_wifi=OfdmRate6_5MbpsBW20MHz"

Program Arguments:
    --wifiStaNodesCount:     Set number of Wifi STA Nodes [12]
    --activeStaNodesCount:   Set number of active Wifi STA Nodes [12]
    --data_rate_for_wifi:    Set Data Rate for WIFI Standard [OfdmRate6_5MbpsBW20MHz]
    --data_rate_for_client:  Set Data Rate for Client [1Mbps]
    --isRtsCts:              Set RTS/CTS [false]
*/

int main (int argc, char* argv[])
{
    uint16_t clientStartTime = 1;
    uint16_t clientStopTime  = 3;
    uint16_t serverStartTime = 1;
    uint16_t serverStopTime  = 3;

    uint16_t wifiStaNodesCount   = 12;
    uint16_t activeStaNodesCount = 12;
    CommandLine cmd;
    cmd.AddValue ("wifiStaNodesCount", "Set number of Wifi STA Nodes", wifiStaNodesCount);
    cmd.AddValue ("activeStaNodesCount", "Set number of active Wifi STA Nodes", activeStaNodesCount);
    cmd.AddValue ("data_rate_for_wifi", "Set Data Rate for WIFI Standard", data_rate_for_wifi);
    cmd.AddValue ("data_rate_for_client", "Set Data Rate for Client", data_rate_for_client);
    cmd.AddValue ("isRtsCts", "Set RTS/CTS", isRtsCts);
    cmd.Parse (argc,argv);

    //  Position of STA Nodes
    double Sta_X_coordinate[12] = {50, -50, 0, 0, 40, -40, 30, -30, -30, 30, -40, 40};
    double Sta_Y_coordinate[12] = {0, 0, 50, -50, 30, -30, 40, -40, 40, -40, 30, -30};

    if(isTraceEnabled)
    {
        LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
        LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    }

    if(isRtsCts) Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1000));

	//  Step 1. Nodes Creation
	NodeContainer wifiStaNodes;
	wifiStaNodes.Create (wifiStaNodesCount);

	NodeContainer wifiApNode;
	wifiApNode.Create(1);
	wifiApNode = wifiApNode.Get (0);

	//  Step 2. Create channel for communication
	YansWifiChannelHelper channel;
	channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	channel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel","Exponent", DoubleValue (distance_loss_value));

	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.SetChannel (channel.Create ());

	WifiHelper wifi;
	wifi.SetStandard (wifi_standard_used);
	wifi.SetRemoteStationManager (wifi_rate_adaptation, "DataMode", StringValue (data_rate_for_wifi),
								  "ControlMode", StringValue (data_rate_for_wifi));

    //  Step 3. Use HtWifiMacHelper only for 802.11n network and NqosWifiMacHelper for 802.11a/b/g

    //NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();    // 802.11a/b/g networks.
    HtWifiMacHelper mac = HtWifiMacHelper :: Default ();	    // 802.11n network

	//  Step 4a. Set up MAC for base stations
	Ssid ssid = Ssid ("ns-3-ssid");
	mac.SetType ("ns3::StaWifiMac",	"Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
	NetDeviceContainer staDevices;
	staDevices = wifi.Install (phy, mac, wifiStaNodes);

	//  Step 4b. Set up MAC for AP
	mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "BeaconGeneration", BooleanValue (true), "BeaconInterval", TimeValue (Seconds (5)));
	NetDeviceContainer apDevice;
	apDevice = wifi.Install (phy, mac, wifiApNode);

	//  Step 5. Set mobility of the nodes
	MobilityHelper mobilityStaNodes;
	Ptr<ListPositionAllocator> positionAllocForSta = CreateObject<ListPositionAllocator> ();
	for(int i = 0;i < wifiStaNodesCount; i++) positionAllocForSta->Add(Vector (Sta_X_coordinate[i], Sta_Y_coordinate[i], 0.0));
	mobilityStaNodes.SetPositionAllocator (positionAllocForSta);
	mobilityStaNodes.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobilityStaNodes.Install (wifiStaNodes);

	MobilityHelper mobilityApNode;
	Ptr<ListPositionAllocator> positionAllocForAp = CreateObject<ListPositionAllocator> ();
	positionAllocForAp->Add (Vector (0.0, 0.0, 0.0));
	mobilityApNode.SetPositionAllocator (positionAllocForAp);
	mobilityApNode.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobilityApNode.Install (wifiApNode);

	//  Step 6. Add Internet layers stack
	InternetStackHelper stack; 
	stack.Install (wifiApNode);
	stack.Install (wifiStaNodes);

	//  Step 7. Assign IP address to each device
	Ipv4AddressHelper address;
	Ipv4InterfaceContainer wifiStaInterfaces, wifiApInterface;
	address.SetBase ("10.0.0.0", "255.255.255.0"); 
	wifiApInterface = address.Assign (apDevice);
	wifiStaInterfaces = address.Assign (staDevices);

	//  Step 8. Create Traffic Source and Sink
	PacketSinkHelper sink (client_data_protocol, Address(InetSocketAddress (Ipv4Address::GetAny (), destination_port))); 
	ApplicationContainer app = sink.Install (wifiApNode);

	app.Start (Seconds (serverStartTime));
	app.Stop  (Seconds (serverStopTime));

	ApplicationContainer clientApps;
	double increment_time = 0.01;
	for(int i = 0;i < wifiStaNodesCount; i++)
	{
		OnOffHelper clientOnOff (client_data_protocol, InetSocketAddress (Ipv4Address (wifiApInterface.GetAddress(0)), destination_port));
		clientOnOff.SetAttribute ("PacketSize", UintegerValue (client_pkt_size));
		clientOnOff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		clientOnOff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

		clientOnOff.SetAttribute ("DataRate", StringValue (data_rate_for_client));
		clientOnOff.SetAttribute ("StartTime", TimeValue (Seconds (1.000000+increment_time)));

		if(i < activeStaNodesCount) clientApps.Add (clientOnOff.Install (wifiStaNodes.Get (i)));
	}

	clientApps.Start (Seconds (clientStartTime));
	clientApps.Stop  (Seconds (clientStopTime));

	//  Step 9. Install FlowMonitor on all nodes
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

	Simulator::Stop (Seconds (clientStopTime));
	Simulator::Run ();

	//  Step 10. Print per flow statistics
	monitor->CheckForLostPackets ();

	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    double cumulative_thr = 0;

	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
	{
		if (i->first > 0)
		{
			Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
			std::cout << " Flow: " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
			std::cout << " Tx Packets: " << i->second.txPackets << "\n";
			std::cout << " Tx Bytes: " << i->second.txBytes << "\n";
			std::cout << " TxOffered: " << i->second.txBytes * 8.0 / (clientStopTime - clientStartTime) / 1000 / 1000 << " Mbps\n";
			std::cout << " Rx Packets: " << i->second.rxPackets << "\n";
			std::cout << " Rx Bytes: " << i->second.rxBytes << "\n";
			std::cout << " Throughput: " << i->second.rxBytes * 8.0 / (clientStopTime - clientStartTime) / 1000 / 1000 << " Mbps\n";
			cumulative_thr += i->second.rxBytes * 8.0 / (clientStopTime - clientStartTime) / 1000 / 1000;
		}
	}
	std::cout << "\nCumulative Throughput is : " << cumulative_thr << "Mbps\n";

	Simulator::Destroy ();

	return 0;
}
