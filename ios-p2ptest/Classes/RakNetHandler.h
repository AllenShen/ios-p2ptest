//
// Created by 沈冬冬 on 14-2-12.
//
//



#ifndef __RakNetHandler_H_
#define __RakNetHandler_H_

#include <iostream>
#include "RakNetStuff.h"
#import "DS_Multilist.h"


class RakNetHandler {
public:

    RakNetStuff* stuffInfo;

    DataStructures::Multilist<ML_QUEUE, RakNet::RakString> outputMessages;
    RakNet::TimeMS whenOutputMessageStarted;

    bool isConnectedToNATPunchthroughServer;

    RakNetHandler();

    void UpdateRakNet();

    void PushMessage(RakNet::RakString rs);
    void PushMessage(const char * rs);

    const char *GetCurrentMessage(void);

    void startTest();

    virtual ~RakNetHandler();
};


#endif //__RakNetHandler_H_
