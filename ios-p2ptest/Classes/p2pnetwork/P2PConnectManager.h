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
#include "UDPProxyHandler.h"

enum PeerConnectTypes
{
    PeerConnectType_Node,
    PeerConnectType_p2p = 1,
    PeerConnectType_proxy
};

class P2PConnectManager {

private:

    P2PConnectManager();

public:
    NatTypeDetectionHandler* natTypeDetectionHandler;           //nat类型检测client
    UPnPHandler* uPnPHandler;                                   //upnp服务类型检测
    NatPunchThroughHandler* natPunchThroughHandler;             //nat穿透服务类型检测
    UDPProxyHandler* proxyHandler;

public:

    NATTypeDetectionResult clientNatType;

    P2PConnectStages curConnectStage;
    bool isConnectedTpPeer;

    RakNetGUID peerGuid;
    bool isHost;

    PeerConnectTypes connectType;                               //端端之间的连接类型

    SystemAddress NATCompleteServerAddress;                         //Nat complete server地址 包含类型检测 nat穿透
    SystemAddress ProxyServerArrress;                               //代理服务器地址

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

    void onConnectSuccess(PeerConnectTypes connectType);            //连接成功的回调函数
    void onConnectFailed();                                         //连接失败的回调

    virtual ~P2PConnectManager();

};


#endif //__RakNetHandler_H_
