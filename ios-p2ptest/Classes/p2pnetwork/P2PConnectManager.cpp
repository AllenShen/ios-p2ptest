//
// Created by 沈冬冬 on 14-2-12.
//
//


#include "P2PConnectManager.h"
#include "GetTime.h"
#include "UPnPHandler.h"
#include "NatTypeDetectionHandler.h"
#include "P2PManagerFunc.h"

P2PConnectManager::P2PConnectManager():
natTypeDetectionHandler(NULL),
uPnPHandler(NULL),
natPunchThroughHandler(NULL)
{
}

void P2PConnectManager::enterStage(P2PConnectStages stage,Packet * packet)
{
    curConnectStage = stage;
    switch (stage)
    {
        case P2PStage_NATTypeDetection:
            printf("检测Nat类型... \n");
            P2PConnectManager::getInstance()->natTypeDetectionHandler->startDetect(NATCompleteServerAddress);
            break;
        case P2PStage_UPNP:
            printf("开始绑定UPnP...  \n");
            isConnectedToNATPunchthroughServer=true;
            P2PConnectManager::getInstance()->uPnPHandler->startUPnp();
            P2PConnectManager::getInstance()->uPnPHandler->handleSinglePacket(packet);
            break;
        case P2PStage_NATPunchThrough:
            printf("开始进行NAT穿透...  \n");
            break;
        case P2PStage_ConnectToPeer:
            break;
        case P2PStage_ConnectEnd:
            break;
    }
}

void P2PConnectManager::initInfo() {

    natTypeDetectionHandler = new NatTypeDetectionHandler();
    uPnPHandler = new UPnPHandler();
    natPunchThroughHandler = new NatPunchThroughHandler();

    enterStage(P2PStage_Initial, NULL);
    clientNatType = NAT_TYPE_UNKNOWN;
    NATCompleteServerAddress = SystemAddress(DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP,DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT);
}

void P2PConnectManager::startNetWork() {
    RakNetStuff::getInstance();
    this->initInfo();
    RakNet::ConnectionAttemptResult car = RakNetStuff::rakPeer->Connect(DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP,
            DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT,0,0);
    printf("connecting to natcomplete server %s, port %d \n",DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP,
            DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT);
    if(car == INVALID_PARAMETER || car == CANNOT_RESOLVE_DOMAIN_NAME || car == SECURITY_INITIALIZATION_FAILED)
    {
        printf("--------------- connect to punch server error -----------------\n");
    }
}

BaseStageHandler *P2PConnectManager::getTargetHandlerByStage()
{
    BaseStageHandler* retHandler = NULL;
    switch (curConnectStage)
    {
        case P2PStage_Initial:
        case P2PStage_NATTypeDetection:
            retHandler = natTypeDetectionHandler;
            break;
        case P2PStage_UPNP:
            retHandler = uPnPHandler;
            break;
        case P2PStage_NATPunchThrough:
            retHandler = natPunchThroughHandler;
            break;
        case P2PStage_ConnectToPeer:
//            retHandler = natTypeDetectionHandler;
            break;
    }
    return retHandler;
}

void P2PConnectManager::startNatPunch(RakNetGUID &destination) {

}

void P2PConnectManager::UpdateRakNet()
{
    RakNet::Packet *packet;
    RakNet::TimeMS curTime = RakNet::GetTimeMS();
    RakNet::RakString targetName;
    for (packet= RakNetStuff::rakPeer->Receive(); packet; RakNetStuff::rakPeer->DeallocatePacket(packet), packet=RakNetStuff::rakPeer->Receive())
    {
        int retCode = packet->data[0];

        if (strcmp(packet->systemAddress.ToString(false),DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP)==0)
        {
            targetName="NATPunchthroughServer";
        }
        else
        {
            targetName=packet->systemAddress.ToString(true);
        }

        switch (retCode)
        {
            case ID_CONNECTION_REQUEST_ACCEPTED:            //成功连接上了服务器/peer
            {
                printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", packet->systemAddress.ToString(true), packet->guid.ToString());

                if(this->curConnectStage == P2PStage_Initial)
                {
                    //连接服务器
                    printf("My external address is %s\n", RakNetStuff::rakPeer->GetExternalID(packet->systemAddress).ToString(true));
                    enterStage(P2PStage_NATTypeDetection,packet);
                }
                else if(this->curConnectStage == P2PStage_ConnectToPeer)
                {
                    //peer to peer 连接成功
                    printf("connect to peer succeed to %s with GUID %s\n",
                            packet->systemAddress.ToString(true), packet->guid.ToString());
                    natTypeDetectionHandler->startDetect(NATCompleteServerAddress);         //开始检测nat类型
                }
                break;
             }
            case ID_NAT_TYPE_DETECTION_RESULT:              //nat类型检测得出结果
                natTypeDetectionHandler->handleSinglePacket(packet);
                break;
            case ID_CONNECTION_ATTEMPT_FAILED:
            case ID_IP_RECENTLY_CONNECTED:
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            case ID_NO_FREE_INCOMING_CONNECTIONS:
            {
                P2PManagerFunc::printPacketMessages(packet);
                if (packet->systemAddress== NATCompleteServerAddress)
                {
                    printf("NatCompleteServer 连接失败, 不能建立p2p连接 \n");
                }
            }
                break;
            case ID_DISCONNECTION_NOTIFICATION:
            case ID_CONNECTION_LOST:
            {
                P2PManagerFunc::printPacketMessages(packet);
                if (packet->systemAddress== NATCompleteServerAddress)
                {
                    isConnectedToNATPunchthroughServer=false;
                }
            }
                break;
            case ID_NEW_INCOMING_CONNECTION:
            {
                if (RakNetStuff::fullyConnectedMesh2->IsHostSystem())
                {
                    printf("%s","Sending player list to new connection");
                    RakNetStuff::fullyConnectedMesh2->StartVerifiedJoin(packet->guid);
                }
            }
                break;
            case ID_FCM2_VERIFIED_JOIN_START:
            {
                DataStructures::List<RakNet::SystemAddress> addresses;
                DataStructures::List<RakNet::RakNetGUID> guids;
                RakNetStuff::fullyConnectedMesh2->GetVerifiedJoinRequiredProcessingList(packet->guid, addresses, guids);
                for (unsigned int i=0; i < guids.Size(); i++)
                    RakNetStuff::natPunchthroughClient->OpenNAT(guids[i], NATCompleteServerAddress);
            }
                break;
            case ID_FCM2_VERIFIED_JOIN_FAILED:
            {
                printf("%s","Failed to join game session");
            }
                break;
            case ID_FCM2_VERIFIED_JOIN_CAPABLE:
            {
                RakNetStuff::fullyConnectedMesh2->RespondOnVerifiedJoinCapable(packet, true, 0);
            }
                break;
            case ID_FCM2_VERIFIED_JOIN_ACCEPTED:
            {
                DataStructures::List<RakNet::RakNetGUID> systemsAccepted;
                bool thisSystemAccepted;
                RakNetStuff::fullyConnectedMesh2->GetVerifiedJoinAcceptedAdditionalData(packet, &thisSystemAccepted, systemsAccepted, 0);
                if (thisSystemAccepted)
                    printf("Game join request accepted\n");
                else
                    printf("System %s joined the mesh\n", systemsAccepted[0].ToString());

                // DataStructures::List<RakNetGUID> participantList;
                // fullyConnectedMesh2->GetParticipantList(participantList);

                for (unsigned int i=0; i < systemsAccepted.Size(); i++)
                    RakNetStuff::replicaManager3->PushConnection(RakNetStuff::replicaManager3->AllocConnection(RakNetStuff::rakPeer->GetSystemAddressFromGuid(systemsAccepted[i]), systemsAccepted[i]));
            }
                break;
            case ID_FCM2_NEW_HOST:
            {
                if (packet->guid==RakNetStuff::rakPeer->GetMyGUID())
                {
                    // Original host dropped. I am the new session host. Upload to the cloud so new players join this system.
                    RakString tempStr = RakString("%s","IrrlichtDemo");
                    RakNet::CloudKey cloudKey(tempStr,0);
                    RakNetStuff::cloudClient->Post(&cloudKey, 0, 0, RakNetStuff::rakPeer->GetGuidFromSystemAddress(NATCompleteServerAddress));
                }
            }
                break;
            case ID_CLOUD_GET_RESPONSE:
            {
                RakNet::CloudQueryResult cloudQueryResult;
                RakNetStuff::cloudClient->OnGetReponse(&cloudQueryResult, packet);
                if (cloudQueryResult.rowsReturned.Size()>0)
                {
                    printf("%s","NAT punch to existing game instance");
                    RakNetStuff::natPunchthroughClient->OpenNAT(cloudQueryResult.rowsReturned[0]->clientGUID, NATCompleteServerAddress);
                }
                else
                {
                    printf("%s","Publishing new game instance");

                    // Start as a new game instance because no other games are running
                    RakString tempStr = RakString("%s","IrrlichtDemo");
                    RakNet::CloudKey cloudKey(tempStr,0);
                    RakNetStuff::cloudClient->Post(&cloudKey, 0, 0, packet->guid);
                }

                RakNetStuff::cloudClient->DeallocateWithDefaultAllocator(&cloudQueryResult);
            }
                break;
            case ID_NAT_TARGET_NOT_CONNECTED:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
//                PushMessage(RakNet::RakString("%s","NAT target ") + targetName + RakNet::RakString("%s"," not connected."));
            }
                break;
            case ID_NAT_TARGET_UNRESPONSIVE:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
//                PushMessage(RakNet::RakString("%s","NAT target ") + targetName + RakNet::RakString("%s"," unresponsive."));
            }
                break;
            case ID_NAT_CONNECTION_TO_TARGET_LOST:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
//                PushMessage(RakNet::RakString("%s","NAT target connection to ") + targetName + RakNet::RakString("%s"," lost."));
            }
                break;
            case ID_NAT_ALREADY_IN_PROGRESS:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
//                PushMessage(RakNet::RakString("%s","NAT punchthrough to ") + targetName + RakNet::RakString("%s"," in progress (skipping)."));
            }
                break;
            case ID_NAT_PUNCHTHROUGH_SUCCEEDED:
            {
                if (packet->data[1]==1)
                {
                    printf("%s","Connecting to existing game instance");
                    RakNet::ConnectionAttemptResult car = RakNetStuff::rakPeer->Connect(packet->systemAddress.ToString(false), packet->systemAddress.GetPort(), 0, 0);
                    RakAssert(car==RakNet::CONNECTION_ATTEMPT_STARTED);
                }
            }
                break;

            case ID_ADVERTISE_SYSTEM:
                if (packet->guid!=RakNetStuff::rakPeer->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS))
                {
                    char hostIP[32];
                    packet->systemAddress.ToString(false,hostIP);
                    RakNet::ConnectionAttemptResult car = RakNetStuff::rakPeer->Connect(hostIP,packet->systemAddress.GetPort(),0,0);
                    RakAssert(car==RakNet::CONNECTION_ATTEMPT_STARTED);
                }
                break;
        }
    }

    // Call the Update function for networked game objects added to BaseIrrlichtReplica once the game is ready
//    if (currentScene>=1)
//    {
//        unsigned int idx;
//        for (idx=0; idx < RakNetStuff::replicaManager3->GetReplicaCount(); idx++)
//            ((BaseIrrlichtReplica::*)(RakNetStuff::replicaManager3->GetReplicaAtIndex(idx)))->Update(curTime);
//    }
}

P2PConnectManager::~P2PConnectManager() {

}