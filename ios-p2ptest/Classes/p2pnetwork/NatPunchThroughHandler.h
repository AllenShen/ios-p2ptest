//
// Created by 沈冬冬 on 14-2-20.
//
//



#ifndef __NatPunchThroughHandler_H_
#define __NatPunchThroughHandler_H_

#include <iostream>
#include "BaseStageHandler.h"
#include "NatPunchthroughClient.h"

class NatPunchThroughHandler : public BaseStageHandler{
public:

    NatPunchthroughClient* punchthroughClient;

    NatPunchThroughHandler();

    virtual ~NatPunchThroughHandler();
};


#endif //__NatPunchThroughHandler_H_
