//
// Created by 沈冬冬 on 14-2-20.
//
//


#include "NatPunchThroughHandler.h"
#include "NatPunchthroughClient.h"
#include "GetTime.h"

NatPunchThroughHandler::NatPunchThroughHandler():
punchthroughClient(NULL)
{
    punchthroughClient = new NatPunchthroughClient();
    punchthroughClient->SetDebugInterface(this);
    RakNetStuff::getInstance()->rakPeer->AttachPlugin(punchthroughClient);
}

void NatPunchThroughHandler::startPunch(RakNetGUID& guid) {
    punchthroughClient->OpenNAT(guid, P2PConnectManager::getInstance()->NATCompleteServerAddress);
}

void NatPunchThroughHandler::startCountDown() {
    BaseStageHandler::startCountDown();
    this->timeMileStone = GetTimeMS() + 5000000;
}

void NatPunchThroughHandler::onTimeOutHandler() {
    if(!this->isOnTimeCountingDown || this->isTimeUp)
        return;
    BaseStageHandler::onTimeOutHandler();
    printf("Nat 穿墙执行超时 \n");
}

NatPunchThroughHandler::~NatPunchThroughHandler() {

}

void NatPunchThroughHandler::OnClientMessage(const char *msg) {
    printf("---------------- debug info:  ");
    printf(msg);
    printf("\n");
}
