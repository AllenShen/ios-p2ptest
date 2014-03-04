//
// Created by 沈冬冬 on 14-2-14.
//
//


#include "NatTypeDetecteUPNPHandler.h"
#include "P2PConnectManager.h"
#include "GetTime.h"
#include "miniupnpc.h"
#include "upnpcommands.h"
#include "upnperrors.h"
#include "Itoa.h"

NatTypeDetecteUPNPHandler::NatTypeDetecteUPNPHandler():
isTypedectedFinished(0),
isUpnpFinished(0)
{
    currentStage = P2PStage_NATTypeUPNPDetection;
    natTypeDetectionClient = NatTypeDetectionClient::GetInstance();
    RakNetStuff::getInstance()->rakPeer->AttachPlugin(natTypeDetectionClient);
}

void NatTypeDetecteUPNPHandler::startCountDown() {
    BaseStageHandler::startCountDown();
    this->timeMileStone = GetTimeMS() + 3000;
}

void NatTypeDetecteUPNPHandler::onTimeOutHandler() {
    if(!this->isOnTimeCountingDown || this->isTimeUp)
        return;
    BaseStageHandler::onTimeOutHandler();

    if(!isTypedectedFinished)
        printf("nat类型检测超时... \n");
    if(!isUpnpFinished)
        printf("upnp绑定超时... \n");
}

void NatTypeDetecteUPNPHandler::startDetect(SystemAddress address)
{
    if(this->isTimeUp)
        return;
    isTypedectedFinished = 0;
    isUpnpFinished = 0;
    natTypeDetectionClient->DetectNATType(address);
    DataStructures::List<RakNetSocket2* > sockets;
    RakNetStuff::rakPeer->GetSockets(sockets);
    this->UPNPOpensynch(sockets[0]->GetBoundAddress().GetPort(), 3000);
}

void NatTypeDetecteUPNPHandler::UPNPProgressCallback(const char *progressMsg, void *userData)
{
    printf(progressMsg);
}

void NatTypeDetecteUPNPHandler::UPNPResultCallback(bool success, unsigned short portToOpen, void *userData)
{
    this->isUpnpFinished = 1;
    if(isTypedectedFinished)
        this->isOnTimeCountingDown = false;
    if (success)
    {
        P2PConnectManager::getInstance()->clientNatType = NAT_TYPE_SUPPORTS_UPNP;
        printf("upbp 端口映射成功  p2p连接可能增加。。。  \n");
    }
    else
    {
        printf("upnp 绑定失败。。。\n");
    }
//    P2PConnectManager::getInstance()->uPnPHandler->isOnTimeCountingDown = false;
    //绑定Upnp结束后 进行连接
//    P2PConnectManager::getInstance()->enterStage(P2PStage_NATPunchThrough);
}

void NatTypeDetecteUPNPHandler::handleSinglePacket(Packet *packet) {

    if(this->isTimeUp)
        return;
    BaseStageHandler::handleSinglePacket(packet);
    int dataType = packet->data[0];
    if(ID_NAT_TYPE_DETECTION_RESULT == dataType)
    {
        isTypedectedFinished = 1;
        NATTypeDetectionResult result = (RakNet::NATTypeDetectionResult) packet->data[1];
        printf("NAT Type is %s (%s)\n", NATTypeDetectionResultToString(result), NATTypeDetectionResultToStringFriendly(result));

        if (result == RakNet::NAT_TYPE_PORT_RESTRICTED)
        {
            printf("端口受限型cone 可穿透 但依赖于upnp的实现 .\n");
        }
        else if(result == NAT_TYPE_SYMMETRIC)
        {
            printf("对称性的NAT,p2p 穿透直连基本宣告失败");
        }
        if(isUpnpFinished)
            this->isOnTimeCountingDown = false;

        P2PConnectManager::getInstance()->clientNatType = result;
//        if (result != RakNet::NAT_TYPE_NONE)                    //存在NAT的情形，需要打开upnp操作
//        {
//            P2PConnectManager::getInstance()->enterStage(P2PStage_UPNP);
//        }
//        else                                                    //不存在NAT 这种情况基本不可能
//        {
//            P2PConnectManager::getInstance()->enterStage(P2PStage_NATPunchThrough);        //直接进入连接服务器阶段
//        }
    }

}

void NatTypeDetecteUPNPHandler::UPNPOpensynch(unsigned short portToOpen, unsigned int timeout)
{
    bool success=false;

    char buff[256];

    struct UPNPDev * devlist = 0;
    RakNet::Time t1 = GetTime();
    devlist = upnpDiscover(timeout, 0, 0, 0, 0, 0);
    RakNet::Time t2 = GetTime();
    if (devlist)
    {
        this->UPNPProgressCallback("List of UPNP devices found on the network :\n", 0);
        struct UPNPDev * device;
        for(device = devlist; device; device = device->pNext)
        {
            sprintf(buff, " desc: %s\n st: %s\n\n", device->descURL, device->st);
            this->UPNPProgressCallback(buff, 0);
        }

        char lanaddr[64];	/* my ip address on the LAN */
        struct UPNPUrls urls;
        struct IGDdatas data;
        if (UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))==1)
        {
            char iport[32];
            Itoa(portToOpen, iport,10);
            char eport[32];
            strcpy(eport, iport);

            // Version miniupnpc-1.6.20120410
            int r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                    eport, iport, lanaddr, 0, "UDP", 0, "0");

            if(r!=UPNPCOMMAND_SUCCESS)
                printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
                        eport, iport, lanaddr, r, strupnperror(r));

            char intPort[6];
            char intClient[16];

            // Version miniupnpc-1.6.20120410
            char desc[128];
            char enabled[128];
            char leaseDuration[128];
            r = UPNP_GetSpecificPortMappingEntry(urls.controlURL,
                    data.first.servicetype,
                    eport, "UDP",
                    intClient, intPort,
                    desc, enabled, leaseDuration);

            if(r!=UPNPCOMMAND_SUCCESS)
            {
                sprintf(buff, "GetSpecificPortMappingEntry() failed with code %d (%s)\n",
                        r, strupnperror(r));
                this->UPNPProgressCallback(buff, 0);
            }
            else
            {
                this->UPNPProgressCallback("UPNP success.\n", 0);
                // game->myNatType=NAT_TYPE_SUPPORTS_UPNP;

                success=true;
            }
        }
    }

    this->UPNPResultCallback(success, portToOpen,0);
}

NatTypeDetecteUPNPHandler::~NatTypeDetecteUPNPHandler() {
    NatTypeDetectionClient::DestroyInstance(natTypeDetectionClient);
}
