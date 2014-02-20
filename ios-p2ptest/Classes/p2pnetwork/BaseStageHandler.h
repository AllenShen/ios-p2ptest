//
//  BaseStageHandler.h
//  ios-p2ptest
//
//  Created by 沈冬冬 on 14-2-14.
//
//

#ifndef __ios_p2ptest__BaseStageHandler__
#define __ios_p2ptest__BaseStageHandler__

#include <iostream>
#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "P2PDefines.h"

using namespace RakNet;

class BaseStageHandler {
    
public:

    P2PConnectStages currentStage;
    Packet *packet;

    BaseStageHandler();

    virtual void handleSinglePacket(Packet *packet);
    
    virtual ~BaseStageHandler();
};

#endif /* defined(__ios_p2ptest__BaseStageHandler__) */
