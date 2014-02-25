//
//  BaseStageHandler.cpp
//  ios-p2ptest
//
//  Created by 沈冬冬 on 14-2-14.
//
//

#include "BaseStageHandler.h"

BaseStageHandler::BaseStageHandler():
packet(NULL),
isTimeUp(false),
isOnTimeCountingDown(false),
timeMileStone(0)
{

}

void BaseStageHandler::startCountDown()
{
    this->isOnTimeCountingDown = true;
}

void BaseStageHandler::handleSinglePacket(Packet *packet) {
    this->packet = packet;
}

void BaseStageHandler::onTimeOutHandler() {
    this->isTimeUp = true;
}

BaseStageHandler::~BaseStageHandler() {

}
