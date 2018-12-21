#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"

#include "ns3/netanim-module.h"

using namespace ns3;

int main(int argc,char *argv[]){
	Time::SetResolution(Time::NS);
	uint32_t nSta=5;
	bool verbose=true;

	CommandLine cmd;

	cmd.AddValue("nSta","Numbere of wifi STA devices",nSta);
	cmd.AddValue("verbose","Tell echo applications to log if true",verbose);

	cmd.Parse(argc,argv);

	if(verbose){
		LogComponentEnable("UdpEchoClientApplication",LOG_LEVEL_ALL);
		LogComponentEnable("UdpEchoServerApplication",LOG_LEVEL_ALL);
	}

	NodeContainer staNodes,apNode;
	staNodes.Create(nSta);
	apNode.Create(1);

	YansWifiChannelHelper channel=YansWifiChannelHelper::Default();
	//使用默认的信道模型
	YansWifiPhyHelper phy=YansWifiPhyHelper::Default();
	//使用默认的PHY模型
	//PHY：（Port Physical Layer）端口物理层，属于OSI七层模型中的第一层
	phy.SetChannel(channel.Create());
	//创建通道对象，并把他关联到物理层对象管理器
	
	WifiHelper wifi;
	wifi.SetRemoteStationManager("ns3::AarfWifiManager");

	//配置Mac类型以及基础设置SSID
	//SSID：（Service Set Identifier）服务集标识，就是无线网的名字
	WifiMacHelper mac;
	Ssid ssid=Ssid("ns-3-ssid");
	mac.SetType("ns3::StaWifiMac","Ssid",SsidValue(ssid),"ActiveProbing",BooleanValue(false));

	NetDeviceContainer staDevices;
	staDevices=wifi.Install(phy,mac,staNodes);

	//ap共享物理层属性
	mac.SetType("ns3::ApWifiMac",
			"Ssid",SsidValue(ssid));
	NetDeviceContainer apDevices;
	apDevices=wifi.Install(phy,mac,apNode);

	//加入移动模型，让STA可以移动，AP 固定
	MobilityHelper mobility;
	mobility.SetPositionAllocator("ns3::GridPositionAllocator",
			"MinX",DoubleValue(0.0),
			"MinY",DoubleValue(0.0),
			"DeltaX",DoubleValue(5.0),
			"DeltaY",DoubleValue(10.0),
			"GridWidth",UintegerValue(3),
			"LayoutType",StringValue("RowFirst"));

	mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
			"Bounds",RectangleValue(Rectangle(-50,50,-50,50)));
	mobility.Install(staNodes);

	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(apNode);

	//安装协议栈
	InternetStackHelper stack;
	stack.Install(staNodes);
	stack.Install(apNode);

	Ipv4AddressHelper address;
	address.SetBase("10.1.1.0","255.255.255.0");
	//声明一个地址生成器对象
	//地址从10.1.1.0开始，并以255.255.255.0为子网掩码分配地址
	//地址分配默认是从1开始并单调递增
	Ipv4InterfaceContainer wifiInterfaces;
	wifiInterfaces=address.Assign(staDevices);
	address.Assign(apDevices);

	UdpEchoServerHelper echoServer(9);

	ApplicationContainer serverApps=echoServer.Install(staNodes.Get(0));
	serverApps.Start(Seconds(1.0));
	serverApps.Stop(Seconds(15.0));

	UdpEchoClientHelper echoClient(wifiInterfaces.GetAddress(0),9);
	echoClient.SetAttribute("MaxPackets",UintegerValue(50));
	echoClient.SetAttribute("Interval",TimeValue(Seconds(0.5)));
	echoClient.SetAttribute("PacketSize",UintegerValue(1024));

	ApplicationContainer clientApps=echoClient.Install(staNodes.Get(nSta-1));

	clientApps.Start(Seconds(1.0));
	clientApps.Stop(Seconds(10.0));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();


	phy.EnablePcap("xml/ap",apDevices.Get(0));

	AnimationInterface anim("xml/ap.xml");


	AsciiTraceHelper ascii;
	phy.EnableAsciiAll(ascii.CreateFileStream("xml/ap.tr"));

	Simulator::Stop(Seconds(10.0));

	Simulator::Run();
	Simulator::Destroy();
	return 0;
}
