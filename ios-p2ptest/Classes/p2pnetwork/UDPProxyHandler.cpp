//
// Created by 沈冬冬 on 14-2-21.
//
//


#include "UDPProxyHandler.h"
#include "RakNetStuff.h"
#include "P2PConnectManager.h"
#include "GetTime.h"
#include "UDPProxyCommon.h"

UDPProxyHandler::UDPProxyHandler():
proxyClient(NULL)
{
    proxyClient = new UDPProxyClient();
    RakNetStuff::getInstance()->rakPeer->AttachPlugin(proxyClient);
    proxyClient->SetResultHandler(this);
}

UDPProxyHandler::~UDPProxyHandler() {

}

void UDPProxyHandler::startCountDown() {
    BaseStageHandler::startCountDown();
    this->timeMileStone = GetTimeMS() + 4000;
}

void UDPProxyHandler::onTimeOutHandler()
{
    if(!this->isOnTimeCountingDown || this->isTimeUp)
        return;
    BaseStageHandler::onTimeOutHandler();
    printf("连接代理服务器超时。。。。。。。 \n");
    P2PConnectManager::getInstance()->onConnectFailed();
}

void UDPProxyHandler::startConnectToProxyServer() {

    this->startCountDown();
    RakNet::ConnectionAttemptResult car = RakNetStuff::rakPeer->Connect(P2PConnectManager::getInstance()->generalConfigData->proxyServerIp.c_str(),
            P2PConnectManager::getInstance()->generalConfigData->proxyServerPort,0,0, NULL,0,12,500,4000);
    if(car == INVALID_PARAMETER || car == CANNOT_RESOLVE_DOMAIN_NAME || car == SECURITY_INITIALIZATION_FAILED)
    {
        printf("--------------- connect to punch server error -----------------\n");
    }
}

void UDPProxyHandler::OnForwardingSuccess(const char *proxyIPAddress, unsigned short proxyPort,
        SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
{
    if(this->isTimeUp)
        return;
    printf("Datagrams forwarded by proxy %s:%i to target %s.\n", proxyIPAddress, proxyPort, targetAddress.ToString(false));
    printf("Connecting to proxy, which will be received by target.\n");
//    proxyIPAddress = DEFAULT_SERVER_ADDRESS;
//    timeout=RakNet::GetTimeMS() + 500000;
    char portInfo[30] = {0};
    sprintf(portInfo, "%d",proxyPort);

    P2PConnectManager::getInstance()->enterStage(P2PStage_ConnectProxyServer);

    //转发成功 连接至代理服务器
    ConnectionAttemptResult car = proxyClientPlugin->GetRakPeerInterface()->Connect(proxyIPAddress, proxyPort, 0, 0);
    RakAssert(car==CONNECTION_ATTEMPT_STARTED);
//		sampleResult=SUCCEEDED;
}

void UDPProxyHandler::OnForwardingNotification(const char *proxyIPAddress, unsigned short proxyPort,
        SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
{
    if(this->isTimeUp)
        return;

    printf("Source %s has setup forwarding to us through proxy %s:%i.\n", sourceAddress.ToString(false), proxyIPAddress, proxyPort);
    P2PConnectManager::getInstance()->enterStage(P2PStage_ConnectProxyServer);
}

void UDPProxyHandler::OnNoServersOnline(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
{
    printf("Failure: No servers logged into coordinator.\n");
    P2PConnectManager::getInstance()->onConnectFailed();
}

void UDPProxyHandler::OnRecipientNotConnected(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
{
    printf("Failure: Recipient not connected to coordinator.\n");
    P2PConnectManager::getInstance()->onConnectFailed();
}

void UDPProxyHandler::OnAllServersBusy(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
{
    printf("Failure: No servers have available forwarding ports.\n");
    P2PConnectManager::getInstance()->onConnectFailed();
}

void UDPProxyHandler::OnForwardingInProgress(const char *proxyIPAddress, unsigned short proxyPort, SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
{
    printf("Notification: Forwarding already in progress.\n");
    P2PConnectManager::getInstance()->onConnectFailed();
}