//
// Created by 沈冬冬 on 14-2-12.
//
//


#include "RakNetStuff.h"

#include "NetworkIDManager.h"
#include "PlayerReplica.h"
#include "NatTypeDetectionClient.h"
#include "P2PConnectManager.h"

RakPeerInterface* RakNetStuff::rakPeer = NULL;
PlayerReplica* RakNetStuff::playerReplica = NULL;
NetworkIDManager* RakNetStuff::networkIDManager = NULL;
ReplicaManager3Irrlicht* RakNetStuff::replicaManager3 = NULL;
CloudClient* RakNetStuff::cloudClient = NULL;
FullyConnectedMesh2* RakNetStuff::fullyConnectedMesh2 = NULL;

RakNetStuff::RakNetStuff() {
    this->initRakPeer();
    this->init();
}

void RakNetStuff::clearInfo()
{
    if(rakPeer == NULL)
        return;
    if(RakNetStuff::replicaManager3)
        rakPeer->DetachPlugin(RakNetStuff::replicaManager3);
    if(RakNetStuff::cloudClient)
        rakPeer->DetachPlugin(RakNetStuff::cloudClient);
    if(RakNetStuff::fullyConnectedMesh2)
        rakPeer->DetachPlugin(RakNetStuff::fullyConnectedMesh2);
    RakPeerInterface::DestroyInstance(rakPeer);
    rakPeer = NULL;
}

void RakNetStuff::initRakPeer()
{
    static const int MAX_PLAYERS=32;
    rakPeer=RakNet::RakPeerInterface::GetInstance();
    RakNet::SocketDescriptor sd(1234,0);
    sd.socketFamily=AF_INET; // Only IPV4 supports broadcast on 255.255.255.255
    while (IRNS2_Berkley::IsPortInUse(sd.port, sd.hostAddress, sd.socketFamily, SOCK_DGRAM)==true)
        sd.port++;
    RakNet::StartupResult sr = rakPeer->Startup(MAX_PLAYERS+1,&sd,1);
    RakAssert(sr==RakNet::RAKNET_STARTED);
    rakPeer->SetMaximumIncomingConnections(MAX_PLAYERS);
    // Fast disconnect for easier testing of host migration
    rakPeer->SetTimeoutTime(5000,UNASSIGNED_SYSTEM_ADDRESS);
    // ReplicaManager3 replies on NetworkIDManager. It assigns numbers to objects so they can be looked up over the network
    // It's a class in case you wanted to have multiple worlds, then you could have multiple instances of NetworkIDManager
}

void RakNetStuff::init() {

    static const unsigned short TCP_PORT=0;
    static const RakNet::TimeMS UDP_SLEEP_TIMER=30;

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
    ConnectionAttemptResult car = rakPeer->Connect(P2PConnectManager::getInstance()->natCompleteServerIp.c_str(),
            P2PConnectManager::getInstance()->NatCompleteServetPort,0,0);
//    ConnectionAttemptResult car = rakPeer->Connect("172.26.192.159", DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT,0,0);
    RakAssert(car==CONNECTION_ATTEMPT_STARTED);

    // Advertise ourselves on the lAN if the NAT punchthrough server is not available
    //for (int i=0; i < 8; i++)
    //	rakPeer->AdvertiseSystem("255.255.255.255", 1234+i, 0,0,0);

}

RakNetStuff::~RakNetStuff() {

}