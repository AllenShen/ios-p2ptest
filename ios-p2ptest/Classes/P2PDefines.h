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
//#define DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP "42.62.67.240"
#define DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP "natpunch.jenkinssoftware.com"

//p2p连接的几个阶段
enum P2PConnectStages
{
    P2PStage_Initial,
    P2PStage_NATTypeDetection,                  //nat类型检测
    P2PStage_UPNP,                              //绑定upnp类型
    P2PStage_ConnetToMasterServer,              //连接到maseter服务器
    P2PStage_NATPunchThrough,                   //对目标客户端进行穿墙连接      目标客户端需要和本机连接在一台punchthrough服务器上
    P2PStage_ConnectToPeer,                     //连接到指定GUID的目标
    P2PStage_ConnectEnd
};

#define STATIC_GET_SINGLEINSTANCE(Type) static Type* getInstance() { \
    static Type* instance = NULL;   \
    if(instance == NULL)            \
    {                               \
        instance = new Type();      \
    }                               \
    return instance;                \
}

#endif
