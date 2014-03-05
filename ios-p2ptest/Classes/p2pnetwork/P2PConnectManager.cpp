//
// Created by 沈冬冬 on 14-2-12.
//
//


#include <ifaddrs.h>
#include "P2PConnectManager.h"
#include "GetTime.h"
#include "NatTypeDetecteUPNPHandler.h"
#include "P2PManagerFunc.h"
#include "UDPProxyCommon.h"
#include "HelloWorldScene.h"

P2PConnectManager::P2PConnectManager():
natTypeDetectionHandler(NULL),
natPunchThroughHandler(NULL),
proxyHandler(NULL),
isConnectedTpPeer(false),
peerGuid(0),
isHost(false),
connectType(PeerConnectType_Node),
latencyCheckIndex(0),
averageLatency(0),
selfInnerIpAddress("")
{
}

void P2PConnectManager::initInfo() {

    this->isConnectedTpPeer = false;

    this->connectType = PeerConnectType_Node;

    clientNatType = NAT_TYPE_UNKNOWN;

    this->natCompleteServerIp = DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP;
    this->NatCompleteServetPort = DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT;

    this->proxyServerIp = DEFAULT_UDPPROXY_IP;
    this->proxyServerPort = DEFAULT_UDPPROXY_PORT;

    NATCompleteServerAddress = SystemAddress(natCompleteServerIp.c_str(),NatCompleteServetPort);
    ProxyServerArrress = SystemAddress(proxyServerIp.c_str(),proxyServerPort);

    natTypeDetectionHandler = new NatTypeDetecteUPNPHandler();
    natPunchThroughHandler = new NatPunchThroughHandler();
    proxyHandler = new UDPProxyHandler();

    allHandler.push_back(natTypeDetectionHandler);
    allHandler.push_back(natPunchThroughHandler);
    allHandler.push_back(proxyHandler);

    this->peerGuid.FromString("18446744073461406419");
    this->isHost = false;

    enterStage(P2PStage_Initial, NULL);

}

void P2PConnectManager::getIPAddress() {
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;

    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0)
    {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL)
        {
            int familyType = temp_addr->ifa_addr->sa_family;
            if(familyType == AF_INET)
            {
                if(strcmp(temp_addr->ifa_name, "en0") == 0)
                {
                    struct sockaddr_in * tempSockaddr = ((struct sockaddr_in *)temp_addr->ifa_addr);
                    this->selfInnerIpAddress = inet_ntoa(tempSockaddr->sin_addr);
                    printf("the inner0 ipaddress is %s the port is %d \n",selfInnerIpAddress.c_str(),((struct sockaddr_in *)temp_addr->ifa_addr)->sin_port);
                }
                else if(strcmp(temp_addr->ifa_name, "en1") == 0)
                {
                    this->selfInnerIpAddress = inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr);
                    printf("the inner1 ipaddress is %s the port is %d \n",selfInnerIpAddress.c_str(),((struct sockaddr_in *)temp_addr->ifa_addr)->sin_port);
                }
            }

            temp_addr = temp_addr->ifa_next;
        }
    }
    freeifaddrs(interfaces);
}

void P2PConnectManager::enterStage(P2PConnectStages stage,Packet * packet)
{
    curConnectStage = stage;
    switch (stage)
    {
        case P2PStage_Initial:
            printf("获取本机的ip地址  \n");
            getIPAddress();
            break;
        case P2PStage_NATTypeUPNPDetection:
            printf("检测Nat类型... \n");
            natTypeDetectionHandler->startCountDown();
            this->natTypeDetectionHandler->startDetect(NATCompleteServerAddress);
            break;
        case P2PStage_NATPunchThrough:
            natPunchThroughHandler->startCountDown();
            if(this->isHost)                                    //我方发起发，主动发出穿墙请求
            {
                printf("开始进行NAT穿透...  \n");
                natPunchThroughHandler->startPunch(peerGuid);
            }
            else
            {
                latencyCheckIndex = 0;
                printf("等待小伙伴进行NAT穿透。。。。 \n");
            }
            break;
        case P2PStage_ConnectToPeer:                                        //nat穿透成功  连接peer
            break;
        case P2PStage_ConnectForwardServer:
            natPunchThroughHandler->isOnTimeCountingDown = false;
            printf("连接到代理服务器。。。。。。 \n");
            this->proxyHandler->startConnectToProxyServer();
            break;
        case P2PStage_ConnectProxyServer:
            printf("连接到代理服务器...... \n");
            break;
        case P2PStage_CountLatency:
            printf("开始估算双方通信延迟  \n");
            latencyCheckIndex = 0;
        case P2PStage_ConnectEnd:
            break;
    }
}

void P2PConnectManager::startNetWork() {
    RakNetStuff::getInstance();
    this->initInfo();
    RakNet::ConnectionAttemptResult car = RakNetStuff::rakPeer->Connect(natCompleteServerIp.c_str(),
            NatCompleteServetPort,0,0, NULL,0,12,500,4000);
    if(car == INVALID_PARAMETER || car == CANNOT_RESOLVE_DOMAIN_NAME || car == SECURITY_INITIALIZATION_FAILED)
    {
        printf("--------------- connect to punch server error -----------------\n");
    }
}

void P2PConnectManager::onConnectSuccess(PeerConnectTypes connectType) {
    this->isConnectedTpPeer = true;
    this->connectType = connectType;

    this->enterStage(P2PStage_CountLatency);

    //进行延迟探测
    if(this->isHost)
    {
        BitStream bsOut;
        bsOut.Write((MessageID) ID_USER_CheckLatency);
        TimeMS packsetSendTime = GetTimeMS();
        bsOut.Write(packsetSendTime);
        RakNetStuff::getInstance()->rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,this->peerGuid,false);
    }
}

void P2PConnectManager::onConnectFailed() {
    proxyHandler->isOnTimeCountingDown = false;
    printf("连接建立失败，请检查网络设置。。。。。。 \n");
    return;
}

BaseStageHandler *P2PConnectManager::getTargetHandlerByStage()
{
    BaseStageHandler* retHandler = NULL;
    switch (curConnectStage)
    {
        case P2PStage_Initial:
        case P2PStage_NATTypeUPNPDetection:
            retHandler = natTypeDetectionHandler;
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
                    printf("My GUID is %s\n", RakNetStuff::rakPeer->GetMyGUID().ToString());
                    if(!this->isHost)
                    {
                        HelloWorld::instance->pLabel->setString(RakNetStuff::rakPeer->GetMyGUID().ToString());
                    }
                    enterStage(P2PStage_NATTypeUPNPDetection,packet);
                    enterStage(P2PStage_NATPunchThrough,packet);
                }
                else if(this->curConnectStage == P2PStage_ConnectToPeer)
                {
                    //peer to peer 连接成功
                    printf("连接到小伙伴 %s with GUID: %s 成功\n",packet->systemAddress.ToString(true), packet->guid.ToString());
                    natPunchThroughHandler->isOnTimeCountingDown = false;
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
                    proxyHandler->isOnTimeCountingDown = false;
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
                    natPunchThroughHandler->isOnTimeCountingDown = false;
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
                else if(this->curConnectStage == P2PStage_ConnectForwardServer || this->curConnectStage == P2PStage_ConnectProxyServer)
                {
                    proxyHandler->isOnTimeCountingDown = false;
                    this->onConnectFailed();
                }
            }
                break;

#pragma NAT穿透阶段消息
                //NAT 穿透系列
            case ID_NAT_TARGET_NOT_CONNECTED:
            case ID_NAT_TARGET_UNRESPONSIVE:
            case ID_NAT_CONNECTION_TO_TARGET_LOST:
            {
                printf("穿墙阶段连接信息丢失.....  \n");
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                this->enterStage(P2PStage_ConnectForwardServer);
            }
                break;
            case ID_NAT_ALREADY_IN_PROGRESS:                                //NAT Punch Through 进行中
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                printf("正在向 %s 穿墙中 ....... ",recipientGuid.ToString());
            }
                break;
            case ID_NAT_PUNCHTHROUGH_FAILED:
                printf("NATp2p 穿透失败 \n");
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

#pragma 连接出错处理_Start
            case ID_DISCONNECTION_NOTIFICATION:
            case ID_CONNECTION_LOST:
            {
                P2PManagerFunc::printPacketMessages(packet);

                //从穿墙服务器断开
                if(strcmp(packet->systemAddress.ToString(false),natCompleteServerIp.c_str())==0)
                {
                    printf("punchthrough 服务器断开连接  \n");
                    if(this->curConnectStage == P2PStage_NATPunchThrough)
                    {
                        printf("跳过punch through 服务器     \n");
                        this->enterStage(P2PStage_ConnectForwardServer);
                    }
                }
                else if(strcmp(packet->systemAddress.ToString(false),proxyServerIp.c_str())==0)
                {
                    printf("proxy 服务器断开连接  \n");
                    if(this->curConnectStage == P2PStage_ConnectProxyServer)
                    {
                        printf("proxy 服务器断开连接 \n");
                        this->onConnectFailed();
                    }
                }
                else
                {
                    printf("与 %s 的连接断开，pk失败 .... \n",packet->systemAddress.ToString(false));
                }

            }
                break;
#pragma 连接出错_End

#pragma 延迟探测信息_Start

            case ID_USER_CheckLatency:           //收到peer发来的信息
            {
                printf(">>>>>>>>>>>>>>>>  收到延迟探测信息  \n");

                TimeMS sendTime;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(sendTime);
                
                if(!this->isHost)
                {
                    this->peerGuid = packet->guid;
                }
                
                BitStream bsOut;
                bsOut.Write((MessageID) ID_USER_CheckLatency_FeedBack);
                bsOut.Write(sendTime);
                RakNetStuff::getInstance()->rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,this->peerGuid,false);
                break;
            }
            case ID_USER_CheckLatency_FeedBack:           //收到peer发来的信息
            {
                printf(">>>>>>>>>>>>>>>  收到延迟探测反馈信息  \n");

                TimeMS curtime = GetTimeMS();

                TimeMS sendTime;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(sendTime);

                uint32_t latencyTime = curtime - sendTime;              //延迟时间
                latencyCheckIndex++;

                averageLatency += latencyTime;
                printf("单次延迟事件为: %d   \n",latencyTime);

                if(SINGLE_MAXLATENCY_CHECKTIME == latencyCheckIndex)
                {
                    BitStream bsOut;
                    bsOut.Write((MessageID) ID_USER_NotifySelfLatency);
                    bsOut.Write(averageLatency);
                    RakNetStuff::getInstance()->rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,this->peerGuid,false);
                    if(!this->isHost)
                    {
                        this->averageLatency = this->averageLatency / 2 / SINGLE_MAXLATENCY_CHECKTIME;
                        printf("-----------平均延时为 %d 等待正式游戏逻辑 \n",this->averageLatency);
                    }
                }
                else                            //继续发送延迟测试信息
                {
                    BitStream bsOut;
                    bsOut.Write((MessageID) ID_USER_CheckLatency);
                    TimeMS packsetSendTime = GetTimeMS();
                    bsOut.Write(packsetSendTime);
                    RakNetStuff::getInstance()->rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,this->peerGuid,false);
                }

                break;
            }
            case ID_USER_NotifySelfLatency:
            {
                uint32_t peerlatency;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(peerlatency);

                printf("收到对方peer的游戏延迟反馈信息 \n");
                if(this->isHost)                //收到被动方的延迟信息，可以进入开始游戏阶段
                {
                    this->averageLatency = peerlatency / 2 / SINGLE_MAXLATENCY_CHECKTIME;
                    printf("++++++++++平均延时为 %d  进入正式游戏逻辑 \n",this->averageLatency);
                }
                else                            //收到主动发的延迟信息，从我方开始测试
                {
                    this->averageLatency = peerlatency;

                    //我方发送延迟探测信息
                    BitStream bsOut;
                    bsOut.Write((MessageID) ID_USER_CheckLatency);
                    TimeMS packsetSendTime = GetTimeMS();
                    bsOut.Write(packsetSendTime);
                    RakNetStuff::getInstance()->rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,this->peerGuid,false);
                }
                break;
            }

#pragma 延迟探测信息_End

        }
    }

    if(curTime > natTypeDetectionHandler->timeMileStone)         //此handler执行时间已经过期
    {
        natTypeDetectionHandler->onTimeOutHandler();
    }
    if(curTime > natPunchThroughHandler->timeMileStone)         //此handler执行时间已经过期
    {
        natPunchThroughHandler->onTimeOutHandler();
    }
    if(curTime > proxyHandler->timeMileStone)         //此handler执行时间已经过期
    {
        proxyHandler->onTimeOutHandler();
    }

}

P2PConnectManager::~P2PConnectManager() {

}