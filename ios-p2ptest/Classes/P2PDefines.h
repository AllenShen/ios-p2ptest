//
//  RakNetDefines.h
//  ios-p2ptest
//
//  Created by 沈冬冬 on 14-2-14.
//
//

#ifndef ios_p2ptest_RakNetDefines_h
#define ios_p2ptest_RakNetDefines_h

#define DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT 61111
#define DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP "42.62.67.240"
//#define DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP "172.26.192.231"
//#define DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP "natpunch.jenkinssoftware.com"
//#define DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP "66.118.161.138"

#define DEFAULT_UDPPROXY_PORT 61111
#define DEFAULT_UDPPROXY_IP "42.62.67.240"

const int SINGLE_MAXLATENCY_CHECKTIME = 2;

//p2p连接的几个阶段
enum P2PConnectStages
{
    P2PStage_Initial,
    P2PStage_NATTypeUPNPDetection,                  //nat类型检测 + UPNP检测
    P2PStage_NATPunchThrough,                   //对目标客户端进行穿墙连接      目标客户端需要和本机连接在一台punchthrough服务器上
    P2PStage_ConnectToPeer,                     //连接到指定GUID的目标
    P2PStage_ConnectForwardServer,                //连接到转发服务器
    P2PStage_ConnectProxyServer,                //连接到代理服务器
    P2PStage_CountLatency,                      //计算延迟
    P2PStage_AdjustTime,                        //客户端校验时间
    P2PStage_ConnectEnd
};

#define STATIC_GET_SINGLEINSTANCE(Type) public: static Type* getInstance() { \
    static Type* instance = NULL;   \
    if(instance == NULL)            \
    {                               \
        instance = new Type();      \
    }                               \
    return instance;                \
}

#endif
