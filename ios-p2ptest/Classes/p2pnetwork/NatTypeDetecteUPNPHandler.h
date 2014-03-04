//
// Created by 沈冬冬 on 14-2-14.
//
//



#ifndef __NatTypeDetectionHandler_H_
#define __NatTypeDetectionHandler_H_

#include <iostream>
#include "BaseStageHandler.h"
#include "NatTypeDetectionClient.h"
#include "RakNetDefines.h"

//检测client的Nat类型
class NatTypeDetecteUPNPHandler : public BaseStageHandler{

private:
    NatTypeDetectionClient *natTypeDetectionClient;

    int isTypedectedFinished;
    int isUpnpFinished;

public:

    NatTypeDetecteUPNPHandler();

    void UPNPProgressCallback(const char *progressMsg, void *userData);
    void UPNPResultCallback(bool success, unsigned short portToOpen, void *userData);

    void startDetect(SystemAddress address);
    void UPNPOpensynch(unsigned short portToOpen,unsigned int timeout);

    virtual ~NatTypeDetecteUPNPHandler();

    virtual void handleSinglePacket(Packet *packet) override;

    virtual void startCountDown() override;

    virtual void onTimeOutHandler() override;
};


#endif //__NatTypeDetectionHandler_H_
