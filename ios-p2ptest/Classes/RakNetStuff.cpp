//
// Created by 沈冬冬 on 14-2-12.
//
//


#include "RakNetStuff.h"

#include "NetworkIDManager.h"
#include "PlayerReplica.h"
#include "NatTypeDetectionClient.h"

RakPeerInterface* RakNetStuff::rakPeer = NULL;
PlayerReplica* RakNetStuff::playerReplica = NULL;
NetworkIDManager* RakNetStuff::networkIDManager = NULL;
ReplicaManager3Irrlicht* RakNetStuff::replicaManager3 = NULL;
NatTypeDetectionClient* RakNetStuff::natTypeDetectionClient = NULL;
NatPunchthroughClient* RakNetStuff::natPunchthroughClient = NULL;
CloudClient* RakNetStuff::cloudClient = NULL;
FullyConnectedMesh2* RakNetStuff::fullyConnectedMesh2 = NULL;

RakNetStuff::RakNetStuff() {

}

void RakNetStuff::init() {

    static const int MAX_PLAYERS=32;
    static const unsigned short TCP_PORT=0;
    static const RakNet::TimeMS UDP_SLEEP_TIMER=30;

    // Basis of all UDP communications
    rakPeer=RakNet::RakPeerInterface::GetInstance();
    // Using fixed port so we can use AdvertiseSystem and connect on the LAN if the server is not available.
    RakNet::SocketDescriptor sd(1234,0);
    sd.socketFamily=AF_INET; // Only IPV4 supports broadcast on 255.255.255.255
    while (IRNS2_Berkley::IsPortInUse(sd.port, sd.hostAddress, sd.socketFamily, SOCK_DGRAM)==true)
        sd.port++;
    // +1 is for the connection to the NAT punchthrough server
    RakNet::StartupResult sr = rakPeer->Startup(MAX_PLAYERS+1,&sd,1);
    RakAssert(sr==RakNet::RAKNET_STARTED);
    rakPeer->SetMaximumIncomingConnections(MAX_PLAYERS);
    // Fast disconnect for easier testing of host migration
    rakPeer->SetTimeoutTime(5000,UNASSIGNED_SYSTEM_ADDRESS);
    // ReplicaManager3 replies on NetworkIDManager. It assigns numbers to objects so they can be looked up over the network
    // It's a class in case you wanted to have multiple worlds, then you could have multiple instances of NetworkIDManager

    networkIDManager=new NetworkIDManager;
    // Automatically sends around new / deleted / changed game objects
    replicaManager3 = new ReplicaManager3Irrlicht;
    replicaManager3->SetNetworkIDManager(networkIDManager);
    rakPeer->AttachPlugin(replicaManager3);

    // Automatically destroy connections, but don't create them so we have more control over when a system is considered ready to play
    replicaManager3->SetAutoManageConnections(false,true);
    // Create and register the network object that represents the player
    playerReplica = new PlayerReplica;
    replicaManager3->Reference(playerReplica);

    //NAT类型检测
    natTypeDetectionClient = new NatTypeDetectionClient;
    rakPeer->AttachPlugin(natTypeDetectionClient);
    natTypeDetectionClient->DetectNATType();

    //NAT穿透
    natPunchthroughClient=new NatPunchthroughClient;
    rakPeer->AttachPlugin(natPunchthroughClient);

    // Uploads game instance, basically client half of a directory server
    // Server code is in NATCompleteServer sample
    cloudClient=new CloudClient;
    rakPeer->AttachPlugin(cloudClient);

    //p2p 通信用的插件
    fullyConnectedMesh2=new FullyConnectedMesh2;
    fullyConnectedMesh2->SetAutoparticipateConnections(false);
    fullyConnectedMesh2->SetConnectOnNewRemoteConnection(false, "");
    rakPeer->AttachPlugin(fullyConnectedMesh2);

    //连接到公共的穿墙服务器
    ConnectionAttemptResult car = rakPeer->Connect(DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP, DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT,0,0);
//    ConnectionAttemptResult car = rakPeer->Connect("172.26.192.159", DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT,0,0);
    RakAssert(car==CONNECTION_ATTEMPT_STARTED);

    // Advertise ourselves on the lAN if the NAT punchthrough server is not available
    //for (int i=0; i < 8; i++)
    //	rakPeer->AdvertiseSystem("255.255.255.255", 1234+i, 0,0,0);

}

RakNetStuff::~RakNetStuff() {

}