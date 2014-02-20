//
// Created by 沈冬冬 on 14-2-20.
//
//



#ifndef __P2PManagerFunc_H_
#define __P2PManagerFunc_H_

#include <iostream>
#include "RakPeerInterface.h"

using namespace RakNet;

class P2PManagerFunc {
public:
    P2PManagerFunc();

    //输出packet信息(error 信息 )
    static void printPacketMessages(Packet *packet);

    virtual ~P2PManagerFunc();
};


#endif //__P2PManagerFunc_H_
