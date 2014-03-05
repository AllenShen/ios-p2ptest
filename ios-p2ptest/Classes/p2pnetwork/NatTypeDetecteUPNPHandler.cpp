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
    this->timeMileStone = GetTimeMS() + executeMaxTime;
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

void UPNPProgressCallback(const char *progressMsg, void *userData)
{
    printf(progressMsg);
}

void UPNPResultCallback(bool success, unsigned short portToOpen, void *userData)
{
    P2PConnectManager::getInstance()->natTypeDetectionHandler->isUpnpFinished = 1;
    if(P2PConnectManager::getInstance()->natTypeDetectionHandler->isTypedectedFinished)
        P2PConnectManager::getInstance()->natTypeDetectionHandler->isOnTimeCountingDown = false;
    if (success)
    {
        P2PConnectManager::getInstance()->generalConfigData->clientNatType = NAT_TYPE_SUPPORTS_UPNP;
        printf("upbp 端口映射成功  p2p连接可能增加。。。  \n");
    }
    else
    {
        printf("upnp 绑定失败。。。\n");
    }
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
    this->UPNPOpenAsynch(sockets[0]->GetBoundAddress().GetPort(), executeMaxTime,UPNPProgressCallback,UPNPResultCallback,0);
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

        P2PConnectManager::getInstance()->generalConfigData->clientNatType = result;
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

RAK_THREAD_DECLARATION(UPNPOpenWorker)
{
    UPNPOpenWorkerArgs *args = ( UPNPOpenWorkerArgs * ) arguments;
    bool success=false;

    struct UPNPDev * devlist = 0;
    RakNet::Time t1 = GetTime();
    devlist = upnpDiscover(args->timeout, 0, 0, 0, 0, 0);
    RakNet::Time t2 = GetTime();
    if (devlist)
    {
        if (args->progressCallback)
            args->progressCallback("List of UPNP devices found on the network :\n", args->userData);
        struct UPNPDev * device;
        for(device = devlist; device; device = device->pNext)
        {
            sprintf(args->buff, " desc: %s\n st: %s\n\n", device->descURL, device->st);
            if (args->progressCallback)
                args->progressCallback(args->buff, args->userData);
        }

        char lanaddr[64];	/* my ip address on the LAN */
        struct UPNPUrls urls;
        struct IGDdatas data;
        if (UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))==1)
        {
            char iport[32];
            Itoa(args->portToOpen, iport,10);
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
                sprintf(args->buff, "GetSpecificPortMappingEntry() failed with code %d (%s)\n",
                        r, strupnperror(r));
                if (args->progressCallback)
                    args->progressCallback(args->buff, args->userData);
            }
            else
            {
                if (args->progressCallback)
                    args->progressCallback("UPNP success.\n", args->userData);
                // game->myNatType=NAT_TYPE_SUPPORTS_UPNP;

                success=true;
            }
        }
    }

    if (args->resultCallback)
        args->resultCallback(success, args->portToOpen, args->userData);
    RakNet::OP_DELETE(args, _FILE_AND_LINE_);
    return 0;
}

void NatTypeDetecteUPNPHandler::UPNPOpenAsynch(unsigned short portToOpen,
        unsigned int timeout,
        void (*progressCallback)(const char *progressMsg, void *userData),
        void (*resultCallback)(bool success, unsigned short portToOpen, void *userData),
        void *userData
)
{
    UPNPOpenWorkerArgs *args = RakNet::OP_NEW<UPNPOpenWorkerArgs>(_FILE_AND_LINE_);
    args->portToOpen = portToOpen;
    args->timeout = timeout;
    args->userData = userData;
    args->progressCallback = progressCallback;
    args->resultCallback = resultCallback;
    RakThread::Create(UPNPOpenWorker, args);
}

NatTypeDetecteUPNPHandler::~NatTypeDetecteUPNPHandler() {
    NatTypeDetectionClient::DestroyInstance(natTypeDetectionClient);
}
