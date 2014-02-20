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
class NatTypeDetectionHandler : public BaseStageHandler{

private:
    NatTypeDetectionClient *natTypeDetectionClient;

public:

    NatTypeDetectionHandler();

    void startDetect(SystemAddress address);

    virtual ~NatTypeDetectionHandler();

    virtual void handleSinglePacket(Packet *packet) override;
};


#endif //__NatTypeDetectionHandler_H_
