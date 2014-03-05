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

#define executeMaxTime 4500

struct UPNPOpenWorkerArgs
{
    char buff[256];
    unsigned short portToOpen;
    unsigned int timeout;
    void *userData;
    void (*resultCallback)(bool success, unsigned short portToOpen, void *userData);
    void (*progressCallback)(const char *progressMsg, void *userData);
};

//检测client的Nat类型
class NatTypeDetecteUPNPHandler : public BaseStageHandler{

private:
    NatTypeDetectionClient *natTypeDetectionClient;

public:
    int isTypedectedFinished;
    int isUpnpFinished;

public:

    NatTypeDetecteUPNPHandler();

    void UPNPOpenAsynch(unsigned short portToOpen,
            unsigned int timeout,
            void (*progressCallback)(const char *progressMsg, void *userData),
            void (*resultCallback)(bool success, unsigned short portToOpen, void *userData),
            void *userData
    );

    void startDetect(SystemAddress address);

    virtual ~NatTypeDetecteUPNPHandler();

    virtual void handleSinglePacket(Packet *packet) override;

    virtual void startCountDown() override;

    virtual void onTimeOutHandler() override;
};


#endif //__NatTypeDetectionHandler_H_
