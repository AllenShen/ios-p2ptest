//
//  BaseStageHandler.cpp
//  ios-p2ptest
//
//  Created by 沈冬冬 on 14-2-14.
//
//

#include "BaseStageHandler.h"

BaseStageHandler::BaseStageHandler():packet(NULL) {

}

void BaseStageHandler::handleSinglePacket(Packet *packet) {
    this->packet = packet;
}

BaseStageHandler::~BaseStageHandler() {

}
