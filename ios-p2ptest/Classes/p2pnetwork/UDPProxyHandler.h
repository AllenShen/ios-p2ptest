//
// Created by 沈冬冬 on 14-2-21.
//
//



#ifndef __UDPProxyHandler_H_
#define __UDPProxyHandler_H_

#include <iostream>
#include "BaseStageHandler.h"
#include "UDPProxyClient.h"

class UDPProxyHandler :public BaseStageHandler,public UDPProxyClientResultHandler{
public:

    UDPProxyClient* proxyClient;

    UDPProxyHandler();

    void startConnectToProxyServer();

    virtual void OnForwardingSuccess(const char *proxyIPAddress, unsigned short proxyPort,
            SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin);

    virtual void OnForwardingNotification(const char *proxyIPAddress, unsigned short proxyPort,
            SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin);

    virtual void OnNoServersOnline(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin);

    virtual void OnRecipientNotConnected(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin);

    virtual void OnAllServersBusy(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin);

    virtual void OnForwardingInProgress(const char *proxyIPAddress, unsigned short proxyPort, SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin);

    virtual ~UDPProxyHandler();

    virtual void startCountDown() override;

    virtual void onTimeOutHandler() override;
};


#endif //__UDPProxyHandler_H_
