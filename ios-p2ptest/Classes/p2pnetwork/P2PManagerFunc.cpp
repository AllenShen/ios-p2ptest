//
// Created by 沈冬冬 on 14-2-20.
//
//


#include "P2PManagerFunc.h"
#include "RakString.h"
#include "P2PDefines.h"
#include "MessageIdentifiers.h"
#include "P2PConnectManager.h"

P2PManagerFunc::P2PManagerFunc() {

}

void P2PManagerFunc::printPacketMessages(Packet *packet)
{
    if(packet == NULL)
        return;
    int packetCode = packet->data[0];
    const char* targetName;
    if (strcmp(packet->systemAddress.ToString(false),P2PConnectManager::getInstance()->generalConfigData->natCompleteServerIp.c_str())==0)
    {
        targetName = "NATPunchthroughServer";
    }
    else
    {
        targetName = packet->systemAddress.ToString(true);
    }

    switch (packetCode)
    {
        case ID_CONNECTION_ATTEMPT_FAILED:
            printf("连接 to %s 失败 \n",targetName);
            break;
        case ID_IP_RECENTLY_CONNECTED:
            printf("This IP address recently connected from %s\n",targetName);
            break;
        case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            printf("Incompatible protocol version from  %s\n",targetName);
            break;
        case ID_NO_FREE_INCOMING_CONNECTIONS:
            printf("No free incoming connections to  %s\n",targetName);
            break;
        case ID_DISCONNECTION_NOTIFICATION:
            printf("Disconnected from  %s \n",targetName);
            break;
        case ID_CONNECTION_LOST:
            printf("connection to %s 断开 \n",targetName);
            break;
    }
}

P2PManagerFunc::~P2PManagerFunc() {

}