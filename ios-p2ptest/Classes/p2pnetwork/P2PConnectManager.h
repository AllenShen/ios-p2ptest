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
#include "NatTypeDetecteUPNPHandler.h"
#include "NatPunchThroughHandler.h"
#include "UDPProxyHandler.h"
#include "cocos2d.h"
USING_NS_CC;

using namespace std;

enum PeerConnectTypes
{
    PeerConnectType_Node,
    PeerConnectType_p2p = 1,
    PeerConnectType_proxy
};

class NatPunchThroughHandler;

//和服务器相关的配置，网络断开之前，只需要初始化一次
struct GeneralData {
    NATTypeDetectionResult clientNatType;
    string selfInnerIpAddress;
    SystemAddress selfInnerAddress;

    /**
    *   服务对应的guid以及serveraddress
    */
    SystemAddress NATCompleteServerAddress;                         //Nat complete server地址 包含类型检测 nat穿透
    RakNetGUID punchServerGuid;         //服务器的guid

    string natCompleteServerIp;
    unsigned short NatCompleteServetPort;

    string proxyServerIp;
    unsigned short proxyServerPort;

    SystemAddress ProxyServerArrress;                               //代理服务器地址

    int averageServerLatency;                                       //和服务器网络延时

    GeneralData ():
    clientNatType(NAT_TYPE_UNKNOWN),
    selfInnerIpAddress(""),
    selfInnerAddress(),
    NATCompleteServerAddress(""),
    punchServerGuid(),
    averageServerLatency(-1)
    {

    }
};

struct SelfPeerDataConfig
{
    PeerConnectTypes connectType;                               //端端之间的连接类型
    RakNetGUID peerGuid;                //对方玩家的guid
    bool isHost;
    int averagePeerLatency;                                         //平均的网络延迟时间

    SelfPeerDataConfig():
    peerGuid(0),
    isHost(false),
    connectType(PeerConnectType_Node),
    averagePeerLatency(0)
    {

    }
};

class P2PConnectManager {

private:

    P2PConnectManager();

public:
    NatTypeDetecteUPNPHandler * natTypeDetectionHandler;           //nat类型检测client
    NatPunchThroughHandler* natPunchThroughHandler;             //nat穿透服务类型检测
    UDPProxyHandler* proxyHandler;

    vector<BaseStageHandler*> allHandler;

private:
    void getIPAddress();

    STATIC_GET_SINGLEINSTANCE(P2PConnectManager);

public:

    GeneralData* generalConfigData;                                     //通用的配置数据
    SelfPeerDataConfig* selfPeerDataConfig;                             //peer的配置数据

    P2PConnectStages curConnectStage;
    int latencyCheckIndex;

    TimeMS nextActionTime;

public:

    void initInfo();
    void enterStage(P2PConnectStages stage,Packet * packet = NULL);

    void startNetWork();

    void UpdateRakNet();

    void onConnectSuccess(PeerConnectTypes connectType);            //连接成功的回调函数
    void onConnectFailed();                                         //连接失败的回调

    virtual ~P2PConnectManager();

};


#endif //__RakNetHandler_H_
