//
// Created by 沈冬冬 on 14-2-20.
//
//



#ifndef __NatPunchThroughHandler_H_
#define __NatPunchThroughHandler_H_

#include <iostream>
#include "BaseStageHandler.h"
#include "NatPunchthroughClient.h"
#include "P2PConnectManager.h"

class NatPunchThroughHandler : public BaseStageHandler,public NatPunchthroughDebugInterface{
public:

    NatPunchthroughClient* punchthroughClient;

    NatPunchThroughHandler();
    void startPunch(RakNetGUID& guid);

    virtual ~NatPunchThroughHandler();

    virtual void startCountDown() override;

    virtual void onTimeOutHandler() override;

    virtual void OnClientMessage(const char *msg) override;
};


#endif //__NatPunchThroughHandler_H_
