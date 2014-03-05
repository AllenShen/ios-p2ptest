//
// Created by 沈冬冬 on 14-2-12.
//
//



#ifndef __RakNetStuff_H_
#define __RakNetStuff_H_

#include <iostream>

#include "RakPeerInterface.h"
#include "ReplicaManager3.h"
#include "NatPunchthroughClient.h"
#include "CloudClient.h"
#include "FullyConnectedMesh2.h"
#include "UDPProxyClient.h"
#include "TCPInterface.h"
#include "HTTPConnection.h"
#include "MessageIdentifiers.h"
#include "ReplicaManager3Irrlicht.h"
#include "NatTypeDetectionClient.h"
#include "P2PDefines.h"

using namespace RakNet;

class PlayerReplica;

class RakNetStuff {

private:
    void initRakPeer();
    void init();

public:

    static RakPeerInterface *rakPeer;
    static PlayerReplica *playerReplica; // Network object that represents the player
    static NetworkIDManager *networkIDManager; // Unique IDs per network object
    static ReplicaManager3Irrlicht *replicaManager3; // Autoreplicate network objects
    static CloudClient *cloudClient; // Used to upload game instance to the cloud

    RakNetStuff();

    void clearInfo();

    STATIC_GET_SINGLEINSTANCE(RakNetStuff);

    virtual ~RakNetStuff();
};


#endif //__RakNetStuff_H_
