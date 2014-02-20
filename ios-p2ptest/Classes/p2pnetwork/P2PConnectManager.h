//
// Created by 沈冬冬 on 14-2-12.
//
//



#ifndef __RakNetHandler_H_
#define __RakNetHandler_H_

#include <iostream>
#include "RakNetStuff.h"
#include "DS_Multilist.h"
#include "P2PDefines.h"
#include "NatTypeDetectionHandler.h"
#include "UPnPHandler.h"
#include "NatPunchThroughHandler.h"

class P2PConnectManager {

private:

    P2PConnectManager();

public:
    NatTypeDetectionHandler* natTypeDetectionHandler;
    UPnPHandler* uPnPHandler;
    NatPunchThroughHandler* natPunchThroughHandler;

public:

    NATTypeDetectionResult clientNatType;
    P2PConnectStages curConnectStage;

    SystemAddress NATCompleteServerAddress;

    STATIC_GET_SINGLEINSTANCE(P2PConnectManager);
    RakNet::TimeMS whenOutputMessageStarted;

    bool isConnectedToNATPunchthroughServer;

    void initInfo();
    void enterStage(P2PConnectStages stage,Packet * packet = NULL);

    BaseStageHandler* getTargetHandlerByStage();                    //获得当前阶段对应的handler

    //向某个guid发起nat穿越请求
    void startNatPunch(RakNetGUID& destination);

    void UpdateRakNet();

    void startNetWork();

    virtual ~P2PConnectManager();

};


#endif //__RakNetHandler_H_
