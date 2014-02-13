//
// Created by 沈冬冬 on 14-2-12.
//
//


#include "RakNetHandler.h"
#include "GetTime.h"
#include "BaseIrrlichtReplica.h"
#import "miniupnpc.h"
#import "upnpcommands.h"

RakNetHandler::RakNetHandler():stuffInfo(NULL) {

}

void RakNetHandler::startTest() {

     if(stuffInfo == NULL)
     {
         stuffInfo = new RakNetStuff();
         stuffInfo->init();
     }
}

void RakNetHandler::PushMessage(RakNet::RakString rs)
{
    outputMessages.Push(rs,_FILE_AND_LINE_);
    if (whenOutputMessageStarted==0)
    {
        whenOutputMessageStarted=RakNet::GetTimeMS();
    }
}

void RakNetHandler::PushMessage(const char *rs) {
    RakNet::RakString stringInfo = RakNet::RakString("%s",rs);
    PushMessage(stringInfo);
}

const char *RakNetHandler::GetCurrentMessage(void)
{
    if (outputMessages.GetSize()==0)
        return "";
    RakNet::TimeMS curTime = RakNet::GetTimeMS();
    if (curTime-whenOutputMessageStarted>2500)
    {
        outputMessages.Pop(_FILE_AND_LINE_);
        whenOutputMessageStarted=curTime;
    }

    if (outputMessages.GetSize()==0)
    {
        whenOutputMessageStarted=0;
        return "";
    }
    return outputMessages.Peek().C_String();
}

void RakNetHandler::UpdateRakNet()
{
    RakNet::SystemAddress facilitatorSystemAddress(DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP, DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_PORT);
    RakNet::Packet *packet;
    RakNet::TimeMS curTime = RakNet::GetTimeMS();
    RakNet::RakString targetName;
    for (packet= RakNetStuff::rakPeer->Receive(); packet; RakNetStuff::rakPeer->DeallocatePacket(packet), packet=RakNetStuff::rakPeer->Receive())
    {
        if (strcmp(packet->systemAddress.ToString(false),DEFAULT_NAT_PUNCHTHROUGH_FACILITATOR_IP)==0)
        {
            targetName="NATPunchthroughServer";
        }
        else
        {
            targetName=packet->systemAddress.ToString(true);
        }

        switch (packet->data[0])
        {
            case ID_IP_RECENTLY_CONNECTED:
            {
                PushMessage(RakNet::RakString("%s","This IP address recently connected from ") + targetName + RakNet::RakString("%s","."));
                if (packet->systemAddress==facilitatorSystemAddress)
                    PushMessage("Multiplayer will not work without the NAT punchthrough server!");
            }
                break;
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            {
                PushMessage(RakNet::RakString("%s","Incompatible protocol version from ") + targetName + RakNet::RakString("%s","."));
                if (packet->systemAddress==facilitatorSystemAddress)
                    PushMessage("Multiplayer will not work without the NAT punchthrough server!");
            }
                break;
            case ID_DISCONNECTION_NOTIFICATION:
            {
                PushMessage(RakNet::RakString("%s","Disconnected from ") + targetName + RakNet::RakString("%s","."));
                if (packet->systemAddress==facilitatorSystemAddress)
                    isConnectedToNATPunchthroughServer=false;
            }
                break;
            case ID_CONNECTION_LOST:
            {
                PushMessage(RakNet::RakString("%s","Connection to ") + targetName + RakNet::RakString("%s"," lost."));
                if (packet->systemAddress==facilitatorSystemAddress)
                    isConnectedToNATPunchthroughServer=false;
            }
                break;
            case ID_NO_FREE_INCOMING_CONNECTIONS:
            {
                PushMessage(RakNet::RakString("%s","No free incoming connections to ") + targetName + RakNet::RakString("%s","."));
                if (packet->systemAddress==facilitatorSystemAddress)
                    PushMessage("Multiplayer will not work without the NAT punchthrough server!");
            }
                break;
            case ID_NEW_INCOMING_CONNECTION:
            {
                if (RakNetStuff::fullyConnectedMesh2->IsHostSystem())
                {
                    PushMessage(RakNet::RakString("%s","Sending player list to new connection"));
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
                    RakNetStuff::natPunchthroughClient->OpenNAT(guids[i], facilitatorSystemAddress);
            }
                break;
            case ID_FCM2_VERIFIED_JOIN_FAILED:
            {
                PushMessage(RakNet::RakString("%s","Failed to join game session"));
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
                    PushMessage("Game join request accepted\n");
                else
                    PushMessage(RakNet::RakString("System %s joined the mesh\n", systemsAccepted[0].ToString()));

                // DataStructures::List<RakNetGUID> participantList;
                // fullyConnectedMesh2->GetParticipantList(participantList);

                for (unsigned int i=0; i < systemsAccepted.Size(); i++)
                    RakNetStuff::replicaManager3->PushConnection(RakNetStuff::replicaManager3->AllocConnection(RakNetStuff::rakPeer->GetSystemAddressFromGuid(systemsAccepted[i]), systemsAccepted[i]));
            }
                break;
            case ID_CONNECTION_REQUEST_ACCEPTED:
            {
                PushMessage(RakNet::RakString("%s","Connection request to ") + targetName + RakNet::RakString("%s"," accepted."));
                std::string string1 = packet->systemAddress.ToString(true);
                std::string string2 = facilitatorSystemAddress.ToString(true);
                if (packet->systemAddress==facilitatorSystemAddress)
                {
                    isConnectedToNATPunchthroughServer=true;

                    struct UPNPDev * devlist = 0;
                    devlist = upnpDiscover(1000, 0, 0, 0, 0,0);
                    if (devlist)
                    {
                        char lanaddr[64];	/* my ip address on the LAN */
                        struct UPNPUrls urls;
                        struct IGDdatas data;
                        if (UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))==1)
                        {
                            // External port is the port people will be connecting to us on. This is our port as seen by the directory server
                            // Internal port is the port RakNet was internally started on
                            char eport[32], iport[32];
                            RakNetStuff::natPunchthroughClient->GetUPNPPortMappings(eport, iport, facilitatorSystemAddress);

                            int r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,eport, iport, lanaddr, 0, "UDP", 0,"0");

                            if(r==UPNPCOMMAND_SUCCESS)
                            {
                                // UPNP done
                            }

                        }
                    }

                    // Query cloud for other running game instances
                    RakNet::CloudQuery cloudQuery;
                    RakString tempStr = RakString("%s","IrrlichtDemo");
                    cloudQuery.keys.Push(RakNet::CloudKey(tempStr,0),_FILE_AND_LINE_);
                    RakNetStuff::cloudClient->Get(&cloudQuery, packet->guid);
                }
            }
                break;
            case ID_FCM2_NEW_HOST:
            {
                if (packet->guid==RakNetStuff::rakPeer->GetMyGUID())
                {
                    // Original host dropped. I am the new session host. Upload to the cloud so new players join this system.
                    RakString tempStr = RakString("%s","IrrlichtDemo");
                    RakNet::CloudKey cloudKey(tempStr,0);
                    RakNetStuff::cloudClient->Post(&cloudKey, 0, 0, RakNetStuff::rakPeer->GetGuidFromSystemAddress(facilitatorSystemAddress));
                }
            }
                break;
            case ID_CLOUD_GET_RESPONSE:
            {
                RakNet::CloudQueryResult cloudQueryResult;
                RakNetStuff::cloudClient->OnGetReponse(&cloudQueryResult, packet);
                if (cloudQueryResult.rowsReturned.Size()>0)
                {
                    PushMessage(RakNet::RakString("%s","NAT punch to existing game instance"));
                    RakNetStuff::natPunchthroughClient->OpenNAT(cloudQueryResult.rowsReturned[0]->clientGUID, facilitatorSystemAddress);
                }
                else
                {
                    PushMessage(RakNet::RakString("%s","Publishing new game instance"));

                    // Start as a new game instance because no other games are running
                    RakString tempStr = RakString("%s","IrrlichtDemo");
                    RakNet::CloudKey cloudKey(tempStr,0);
                    RakNetStuff::cloudClient->Post(&cloudKey, 0, 0, packet->guid);
                }

                RakNetStuff::cloudClient->DeallocateWithDefaultAllocator(&cloudQueryResult);
            }
                break;
            case ID_CONNECTION_ATTEMPT_FAILED:
            {
                PushMessage(RakNet::RakString("%s","Connection attempt to ") + targetName + RakNet::RakString("%s"," failed."));
                if (packet->systemAddress==facilitatorSystemAddress)
                    PushMessage("Multiplayer will not work without the NAT punchthrough server!");
            }
                break;
            case ID_NAT_TARGET_NOT_CONNECTED:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
                PushMessage(RakNet::RakString("%s","NAT target ") + targetName + RakNet::RakString("%s"," not connected."));
            }
                break;
            case ID_NAT_TARGET_UNRESPONSIVE:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
                PushMessage(RakNet::RakString("%s","NAT target ") + targetName + RakNet::RakString("%s"," unresponsive."));
            }
                break;
            case ID_NAT_CONNECTION_TO_TARGET_LOST:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
                PushMessage(RakNet::RakString("%s","NAT target connection to ") + targetName + RakNet::RakString("%s"," lost."));
            }
                break;
            case ID_NAT_ALREADY_IN_PROGRESS:
            {
                RakNet::RakNetGUID recipientGuid;
                RakNet::BitStream bs(packet->data,packet->length,false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));
                bs.Read(recipientGuid);
                targetName=recipientGuid.ToString();
                PushMessage(RakNet::RakString("%s","NAT punchthrough to ") + targetName + RakNet::RakString("%s"," in progress (skipping)."));
            }
                break;

            case ID_NAT_PUNCHTHROUGH_SUCCEEDED:
            {
                if (packet->data[1]==1)
                {
                    PushMessage(RakNet::RakString("%s","Connecting to existing game instance"));
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
//            ((BaseIrrlichtReplica::*)(RakNetStuff::replicaManager3->GetReplicaAtIndex(idx)))->Update(curTime);;
//    }
}

RakNetHandler::~RakNetHandler() {

}