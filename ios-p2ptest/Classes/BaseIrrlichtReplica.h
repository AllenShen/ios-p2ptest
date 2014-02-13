//
// Created by 沈冬冬 on 14-2-12.
//
//



#ifndef __BaseIrrlichtReplica_H_
#define __BaseIrrlichtReplica_H_

#include <iostream>
#include "RakNetTime.h"
#include "ReplicaManager3.h"

using namespace RakNet;

class CDemo;

class BaseIrrlichtReplica : public RakNet::Replica3
{
public:
    BaseIrrlichtReplica();
    virtual ~BaseIrrlichtReplica();
    virtual RakNet::RM3ConstructionState QueryConstruction(RakNet::Connection_RM3 *destinationConnection, RakNet::ReplicaManager3 *replicaManager3) {return QueryConstruction_PeerToPeer(destinationConnection);}
    virtual bool QueryRemoteConstruction(RakNet::Connection_RM3 *sourceConnection) {return QueryRemoteConstruction_PeerToPeer(sourceConnection);}
    virtual void DeallocReplica(RakNet::Connection_RM3 *sourceConnection) {delete this;}
    virtual RakNet::RM3QuerySerializationResult QuerySerialization(RakNet::Connection_RM3 *destinationConnection) {return QuerySerialization_PeerToPeer(destinationConnection);}
    virtual void SerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection);
    virtual bool DeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection);
    virtual RakNet::RM3SerializationResult Serialize(RakNet::SerializeParameters *serializeParameters);
    virtual void Deserialize(RakNet::DeserializeParameters *deserializeParameters);
    virtual void SerializeDestruction(RakNet::BitStream *destructionBitstream, RakNet::Connection_RM3 *destinationConnection) {}
    virtual bool DeserializeDestruction(RakNet::BitStream *destructionBitstream, RakNet::Connection_RM3 *sourceConnection) {return true;}
    virtual RakNet::RM3ActionOnPopConnection QueryActionOnPopConnection(RakNet::Connection_RM3 *droppedConnection) const {return QueryActionOnPopConnection_PeerToPeer(droppedConnection);}

    /// This function is not derived from Replica3, it's specific to this app
    /// Called from CDemo::UpdateRakNet
    virtual void Update(RakNet::TimeMS curTime);

    // Set when the object is constructed
    CDemo *demo;

    // real is written on the owner peer, read on the remote peer
//    irr::core::vector3df position;
    RakNet::TimeMS creationTime;
};


#endif //__BaseIrrlichtReplica_H_
