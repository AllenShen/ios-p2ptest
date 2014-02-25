//
// Created by 沈冬冬 on 14-2-14.
//
//


#include "NatTypeDetectionHandler.h"
#include "P2PConnectManager.h"
#include "GetTime.h"

NatTypeDetectionHandler::NatTypeDetectionHandler() {
    currentStage = P2PStage_NATTypeDetection;
    natTypeDetectionClient = NatTypeDetectionClient::GetInstance();
    RakNetStuff::getInstance()->rakPeer->AttachPlugin(natTypeDetectionClient);
}


void NatTypeDetectionHandler::startCountDown() {
    BaseStageHandler::startCountDown();
    this->timeMileStone = GetTimeMS() + 3000;
}

void NatTypeDetectionHandler::onTimeOutHandler() {
    if(!this->isOnTimeCountingDown || this->isTimeUp)
        return;
    BaseStageHandler::onTimeOutHandler();
    printf("nat类型检测超时... \n");
    P2PConnectManager::getInstance()->enterStage(P2PStage_UPNP);
}

void NatTypeDetectionHandler::startDetect(SystemAddress address)
{
    if(this->isTimeUp)
        return;
    natTypeDetectionClient->DetectNATType(address);
}

void NatTypeDetectionHandler::handleSinglePacket(Packet *packet) {

    if(this->isTimeUp)
        return;
    BaseStageHandler::handleSinglePacket(packet);
    int dataType = packet->data[0];
    if(ID_NAT_TYPE_DETECTION_RESULT == dataType)
    {
        NATTypeDetectionResult result = (RakNet::NATTypeDetectionResult) packet->data[1];
        printf("NAT Type is %s (%s)\n", NATTypeDetectionResultToString(result), NATTypeDetectionResultToStringFriendly(result));

        if (result == RakNet::NAT_TYPE_PORT_RESTRICTED)
        {
            printf("端口受限型cone 可穿透 但依赖于upnp的实现 .\n");
        }
        else if(result == NAT_TYPE_SYMMETRIC)
        {
            printf("对称性的NAT,p2p 穿透直连基本宣告失败");
        }

        this->isOnTimeCountingDown = false;
        P2PConnectManager::getInstance()->clientNatType = result;
        if (result != RakNet::NAT_TYPE_NONE)                    //存在NAT的情形，需要打开upnp操作
        {
            P2PConnectManager::getInstance()->enterStage(P2PStage_UPNP);
        }
        else                                                    //不存在NAT 这种情况基本不可能
        {
            P2PConnectManager::getInstance()->enterStage(P2PStage_NATPunchThrough);        //直接进入连接服务器阶段
        }
    }

}

NatTypeDetectionHandler::~NatTypeDetectionHandler() {
    NatTypeDetectionClient::DestroyInstance(natTypeDetectionClient);
}
