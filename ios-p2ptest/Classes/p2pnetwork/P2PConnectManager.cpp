//
// Created by 沈冬冬 on 14-2-12.
//
//


#include "P2PConnectManager.h"
#include "GetTime.h"
#include "UPnPHandler.h"
#include "NatTypeDetectionHandler.h"
#include "P2PManagerFunc.h"
#include "UDPProxyCommon.h"

P2PConnectManager::P2PConnectManager():
natTypeDetectionHandler(NULL),
uPnPHandler(NULL),
natPunchThroughHandler(NULL),
proxyHandler(NULL),
isConnectedTpPeer(false),
peerGuid(0),
isHost(false),
connectType(PeerConnectType_Node)
{
}

void P2PConnectManager::initInfo() {

    this->isConnectedTpPeer = false;

    this->connectType = PeerConnectType_Node;

    this->isHost = false;
    this->peerGuid.FromString("");

    clientNatType = NAT_TYPE_UNKNOWN;
    NATCompleteServerAddress = SystemAddress(DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP,DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT);
    ProxyServerArrress = SystemAddress(DEFAULT_UDPPROXY_IP,DEFAULT_UDPPROXY_PORT);

    natTypeDetectionHandler = new NatTypeDetectionHandler();
    uPnPHandler = new UPnPHandler();
    natPunchThroughHandler = new NatPunchThroughHandler();
    proxyHandler = new UDPProxyHandler();

    enterStage(P2PStage_Initial, NULL);

}

void P2PConnectManager::enterStage(P2PConnectStages stage,Packet * packet)
{
    curConnectStage = stage;
    switch (stage)
    {
        case P2PStage_NATTypeDetection:
            printf("检测Nat类型... \n");
            natTypeDetectionHandler->startCountDown();
            this->natTypeDetectionHandler->startDetect(NATCompleteServerAddress);
            break;
        case P2PStage_UPNP:
            printf("开始绑定UPnP...  \n");
            isConnectedToNATPunchthroughServer = true;
            this->uPnPHandler->startUPnp();
            this->uPnPHandler->handleSinglePacket(packet);
            break;
        case P2PStage_NATPunchThrough:
            printf("开始进行NAT穿透...  \n");
            natPunchThroughHandler->startCountDown();
            if(this->isHost)                                    //我方发起发，主动发出穿墙请求
            {
                natPunchThroughHandler->startPunch(peerGuid);
            }
            break;
        case P2PStage_ConnectToPeer:                                        //nat穿透成功  连接peer
            break;
        case P2PStage_ConnectForwardServer:
            printf("连接到代理服务器。。。。。。 \n");
            this->proxyHandler->startConnectToProxyServer();
            break;
        case P2PStage_ConnectProxyServer:
            printf("连接到代理服务器...... \n");
        case P2PStage_ConnectEnd:
            break;
    }
}

void P2PConnectManager::startNetWork() {
    RakNetStuff::getInstance();
    this->initInfo();
    RakNet::ConnectionAttemptResult car = RakNetStuff::rakPeer->Connect(DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP,
            DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT,0,0, NULL,0,12,500,4000);
    printf("connecting to natcomplete server %s, port %d \n",DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP,
            DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT);
    if(car == INVALID_PARAMETER || car == CANNOT_RESOLVE_DOMAIN_NAME || car == SECURITY_INITIALIZATION_FAILED)
    {
        printf("--------------- connect to punch server error -----------------\n");
    }
}

void P2PConnectManager::onConnectSuccess(PeerConnectTypes connectType) {
    this->isConnectedTpPeer = true;
    this->connectType = connectType;

    //发送测试消息给peer
    BitStream bsOut;
    bsOut.Write((MessageID)ID_USER_PACKET_ENUM);
    RakNetStuff::getInstance()->rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,peerGuid,false);
}

void P2PConnectManager::onConnectFailed() {
    printf("连接建立失败，请检查网络设置。。。。。。");
    return;
}

BaseStageHandler *P2PConnectManager::getTargetHandlerByStage()
{
    BaseStageHandler* retHandler = NULL;
    switch (curConnectStage)
    {
        case P2PStage_Initial:
        case P2PStage_NATTypeDetection:
            retHandler = natTypeDetectionHandler;
            break;
        case P2PStage_UPNP:
            retHandler = uPnPHandler;
            break;
        case P2PStage_NATPunchThrough:
            retHandler = natPunchThroughHandler;
            break;
        case P2PStage_ConnectToPeer:
//            retHandler = natTypeDetectionHandler;
            break;
    }
    return retHandler;
}

void P2PConnectManager::startNatPunch(RakNetGUID &destination) {

}

void P2PConnectManager::UpdateRakNet()
{
    RakNet::Packet *packet;
    RakNet::TimeMS curTime = GetTimeMS();
    RakNet::RakString targetName;
    for (packet= RakNetStuff::rakPeer->Receive(); packet; RakNetStuff::rakPeer->DeallocatePacket(packet), packet=RakNetStuff::rakPeer->Receive())
    {
        int retCode = packet->data[0];

        switch (retCode)
        {
            case ID_CONNECTION_REQUEST_ACCEPTED:            //成功连接上了服务器/peer
            {
                if(this->curConnectStage == P2PStage_Initial)
                {
                    //连接服务器
                    printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", packet->systemAddress.ToString(true), packet->guid.ToString());
                    printf("My external address is %s\n", RakNetStuff::rakPeer->GetExternalID(packet->systemAddress).ToString(true));
                    enterStage(P2PStage_NATTypeDetection,packet);
                }
                else if(this->curConnectStage == P2PStage_ConnectToPeer)
                {
                    //peer to peer 连接成功
                    printf("连接到小伙伴 %s with GUID: %s 成功\n",packet->systemAddress.ToString(true), packet->guid.ToString());

                    this->onConnectSuccess(PeerConnectType_p2p);
                }
                else if(this->curConnectStage == P2PStage_ConnectForwardServer)              //连接上proxy服务器
                {
                    printf("连接上代理服务器 \n");
                    //开始尝试转发
                    proxyHandler->proxyClient->RequestForwarding(P2PConnectManager::getInstance()->ProxyServerArrress, UNASSIGNED_SYSTEM_ADDRESS,
                            this->peerGuid, UDP_FORWARDER_MAXIMUM_TIMEOUT, 0);
                }
                else if(this->curConnectStage == P2PStage_ConnectProxyServer)               //连接到代理服务器成功
                {
                    this->onConnectSuccess(PeerConnectType_proxy);                      //通过代理连接成功
                }
                break;
             }
            case ID_NEW_INCOMING_CONNECTION:
            {
                if(this->curConnectStage == P2PStage_ConnectToPeer)             //连接到peer
                {
                    printf("收到来自小伙伴 %s with GUID: %s 的连接请求\n",
                            packet->systemAddress.ToString(true), packet->guid.ToString());
                    this->onConnectSuccess(PeerConnectType_p2p);
                }
                else
                {
                    if (RakNetStuff::fullyConnectedMesh2->IsHostSystem())
                    {
                        printf("%s","Sending player list to new connection");
                        RakNetStuff::fullyConnectedMesh2->StartVerifiedJoin(packet->guid);
                    }
                }

            }
                break;
            case ID_NAT_TYPE_DETECTION_RESULT:              //nat类型检测得出结果
                natTypeDetectionHandler->handleSinglePacket(packet);
                break;
            case ID_CONNECTION_ATTEMPT_FAILED:
            case ID_IP_RECENTLY_CONNECTED:
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            case ID_NO_FREE_INCOMING_CONNECTIONS:
            {
                P2PManagerFunc::printPacketMessages(packet);
                if(this->curConnectStage == P2PStage_Initial)
                {
                    printf("连接punchThrough服务器失败,\n");
                    this->enterStage(P2PStage_ConnectForwardServer);
                }
                else if(this->curConnectStage == P2PStage_NATPunchThrough || this->curConnectStage == P2PStage_ConnectToPeer)
                {
                    printf("NatCompleteServer 连接失败, 不能建立p2p连接 \n");
                    this->enterStage(P2PStage_ConnectForwardServer);
                }
            }
                break;

#pragma NAT穿透阶段消息
                //NAT 穿透系列
            case ID_NAT_TARGET_NOT_CONNECTED:
            case ID_NAT_TARGET_UNRESPONSIVE:
            case ID_NAT_CONNECTION_TO_TARGET_LOST:
            {
                printf("穿墙阶段连接信息丢失");
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                natPunchThroughHandler->isOnTimeCountingDown = false;
                this->enterStage(P2PStage_ConnectForwardServer);
            }
                break;
            case ID_NAT_PUNCHTHROUGH_FAILED:
                printf("NATp2p 穿透失败 \n");
                natPunchThroughHandler->isOnTimeCountingDown = false;
                this->enterStage(P2PStage_ConnectForwardServer);
                break;
            case ID_NAT_PUNCHTHROUGH_SUCCEEDED:             //穿墙成功
            {
                natPunchThroughHandler->isOnTimeCountingDown = false;
                unsigned char weAreTheSender = packet->data[1];
                this->enterStage(P2PStage_ConnectToPeer);                   //进入直连阶段
                if (weAreTheSender)                                         //我方是发起方
                {
                    printf("%s","穿墙成功，正在尝试建立直连 \n");
                    RakNet::ConnectionAttemptResult car = RakNetStuff::rakPeer->Connect(packet->systemAddress.ToString(false), packet->systemAddress.GetPort(), 0, 0);
                    RakAssert(car==RakNet::CONNECTION_ATTEMPT_STARTED);
                }
                else
                {
                    printf("成功收到穿墙请求。。。等待对方直连建立.....\n");
                }
            }
                break;
#pragma NAT 穿透阶段结束

#pragma 连接出错_Start
            case ID_DISCONNECTION_NOTIFICATION:
            case ID_CONNECTION_LOST:
            {
                P2PManagerFunc::printPacketMessages(packet);
                printf("无法连接master服务器");
            }
                break;
#pragma 连接出错_End

            case ID_FCM2_VERIFIED_JOIN_START:
            {
                DataStructures::List<RakNet::SystemAddress> addresses;
                DataStructures::List<RakNet::RakNetGUID> guids;
                RakNetStuff::fullyConnectedMesh2->GetVerifiedJoinRequiredProcessingList(packet->guid, addresses, guids);
                for (unsigned int i=0; i < guids.Size(); i++)
                    RakNetStuff::natPunchthroughClient->OpenNAT(guids[i], NATCompleteServerAddress);
            }
                break;
            case ID_FCM2_VERIFIED_JOIN_FAILED:
            {
                printf("%s","Failed to join game session");
            }
                break;
            case ID_FCM2_VERIFIED_JOIN_CAPABLE:
            {
                RakNetStuff::fullyConnectedMesh2->RespondOnVerifiedJoinCapable(packet, true, 0);
            }
                break;
            case ID_FCM2_VERIFIED_JOIN_ACCEPTED:
            {
                DataStructures::List<RakNet::RakNetGUID> systemsAccepted;
                bool thisSystemAccepted;
                RakNetStuff::fullyConnectedMesh2->GetVerifiedJoinAcceptedAdditionalData(packet, &thisSystemAccepted, systemsAccepted, 0);
                if (thisSystemAccepted)
                    printf("Game join request accepted\n");
                else
                    printf("System %s joined the mesh\n", systemsAccepted[0].ToString());

                // DataStructures::List<RakNetGUID> participantList;
                // fullyConnectedMesh2->GetParticipantList(participantList);

                for (unsigned int i=0; i < systemsAccepted.Size(); i++)
                    RakNetStuff::replicaManager3->PushConnection(RakNetStuff::replicaManager3->AllocConnection(RakNetStuff::rakPeer->GetSystemAddressFromGuid(systemsAccepted[i]), systemsAccepted[i]));
            }
                break;
            case ID_FCM2_NEW_HOST:
            {
                if (packet->guid==RakNetStuff::rakPeer->GetMyGUID())
                {
                    // Original host dropped. I am the new session host. Upload to the cloud so new players join this system.
                    RakString tempStr = RakString("%s","IrrlichtDemo");
                    RakNet::CloudKey cloudKey(tempStr,0);
                    RakNetStuff::cloudClient->Post(&cloudKey, 0, 0, RakNetStuff::rakPeer->GetGuidFromSystemAddress(NATCompleteServerAddress));
                }
            }
                break;
            case ID_CLOUD_GET_RESPONSE:
            {
                RakNet::CloudQueryResult cloudQueryResult;
                RakNetStuff::cloudClient->OnGetReponse(&cloudQueryResult, packet);
                if (cloudQueryResult.rowsReturned.Size()>0)
                {
                    printf("%s","NAT punch to existing game instance");
                    RakNetStuff::natPunchthroughClient->OpenNAT(cloudQueryResult.rowsReturned[0]->clientGUID, NATCompleteServerAddress);
                }
                else
                {
                    printf("%s","Publishing new game instance");

                    // Start as a new game instance because no other games are running
                    RakString tempStr = RakString("%s","IrrlichtDemo");
                    RakNet::CloudKey cloudKey(tempStr,0);
                    RakNetStuff::cloudClient->Post(&cloudKey, 0, 0, packet->guid);
                }

                RakNetStuff::cloudClient->DeallocateWithDefaultAllocator(&cloudQueryResult);
            }
                break;
            case ID_NAT_ALREADY_IN_PROGRESS:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
//                PushMessage(RakNet::RakString("%s","NAT punchthrough to ") + targetName + RakNet::RakString("%s"," in progress (skipping)."));
            }
                break;
            case ID_ADVERTISE_SYSTEM:
                if (packet->guid!=RakNetStuff::rakPeer->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS))
                {
                    char hostIP[32];
                    packet->systemAddress.ToString(false,hostIP);
                    RakNet::ConnectionAttemptResult car = RakNetStuff::rakPeer->Connect(hostIP,packet->systemAddress.GetPort(),0,0);
                    RakAssert(car==RakNet::CONNECTION_ATTEMPT_STARTED);
                }
                break;

            case ID_USER_PACKET_ENUM:           //收到peer发来的信息
                printf("+++++++++++++++  收到小伙伴发来的请求信息  +++++++++++++++++\n");
                break;
        }
    }

    BaseStageHandler* curHandler = this->getTargetHandlerByStage();
    if(curHandler)                          //当前正在处理的handler
    {
        if(curTime > curHandler->timeMileStone)         //此handler执行时间已经过期
        {
            curHandler->onTimeOutHandler();
        }
    }

}

P2PConnectManager::~P2PConnectManager() {

}