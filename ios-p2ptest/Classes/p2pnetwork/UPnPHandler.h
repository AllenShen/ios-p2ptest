//
// Created by 沈冬冬 on 14-2-14.
//
//



#ifndef __UPnPManager_H_
#define __UPnPManager_H_

#include <iostream>
#include "RakNetDefines.h"
#include "BaseStageHandler.h"

struct UPNPOpenWorkerArgs
{
    char buff[256];
    unsigned short portToOpen;
    unsigned int timeout;
    void *userData;
    void (*resultCallback)(bool success, unsigned short portToOpen, void *userData);
    void (*progressCallback)(const char *progressMsg, void *userData);
};

class UPnPHandler: public BaseStageHandler {

public:
    UPnPHandler();

    void startUPnp();
    void UPNPOpenAsynch(unsigned short portToOpen,
            unsigned int timeout,
            void (*progressCallback)(const char *progressMsg, void *userData),
            void (*resultCallback)(bool success, unsigned short portToOpen, void *userData),
            void *userData
    );

    virtual ~UPnPHandler();

    virtual void startCountDown() override;

    virtual void onTimeOutHandler() override;
};


#endif //__UPnPManager_H_
