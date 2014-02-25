//
// Created by 沈冬冬 on 14-2-14.
//
//

#include "UPnPHandler.h"
#include "miniupnpc.h"
#include "RakNetStuff.h"
#include "upnpcommands.h"
#include "Itoa.h"
#include "P2PConnectManager.h"
#import "upnperrors.h"
#include "GetTime.h"

UPnPHandler::UPnPHandler() {
    currentStage = P2PStage_UPNP;
}

UPnPHandler::~UPnPHandler() {

}

void UPNPProgressCallback(const char *progressMsg, void *userData)
{
    printf(progressMsg);
}

void UPNPResultCallback(bool success, unsigned short portToOpen, void *userData)
{
    if (success)
    {
        P2PConnectManager::getInstance()->clientNatType = NAT_TYPE_SUPPORTS_UPNP;
        printf("upbp 端口映射成功  p2p连接可能增加");
    }
    else
    {
        printf("upnp 绑定失败。。。\n");
    }
    P2PConnectManager::getInstance()->uPnPHandler->isOnTimeCountingDown = false;
    //绑定Upnp结束后 进行连接
    P2PConnectManager::getInstance()->enterStage(P2PStage_NATPunchThrough);
}

void UPnPHandler::startCountDown() {
    BaseStageHandler::startCountDown();
    this->timeMileStone = GetTimeMS() + 2500;
}

void UPnPHandler::onTimeOutHandler() {
    if(!this->isOnTimeCountingDown || this->isTimeUp)
        return;if(!this->isOnTimeCountingDown || this->isTimeUp)
        return;
    BaseStageHandler::onTimeOutHandler();
    printf("UPNP 绑定超时...  \n");
}

void UPnPHandler::startUPnp()
{
    DataStructures::List<RakNetSocket2* > sockets;
    RakNetStuff::rakPeer->GetSockets(sockets);
    this->UPNPOpenAsynch(sockets[0]->GetBoundAddress().GetPort(), 800, UPNPProgressCallback, UPNPResultCallback, 0);
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

void UPnPHandler::UPNPOpenAsynch(unsigned short portToOpen,
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