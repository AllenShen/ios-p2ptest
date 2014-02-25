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

    TimeMS timeMileStone;                                       //过时的时间点
    bool isOnTimeCountingDown;                                          //是否在倒计时
    bool isTimeUp;                                          //是否已经超时

    P2PConnectStages currentStage;
    Packet *packet;

    BaseStageHandler();

    virtual void startCountDown();
    virtual void handleSinglePacket(Packet *packet);

    virtual void onTimeOutHandler();
    
    virtual ~BaseStageHandler();
};

#endif /* defined(__ios_p2ptest__BaseStageHandler__) */
