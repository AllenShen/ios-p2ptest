//
// Created by 沈冬冬 on 14-2-12.
//
//


#include "ReplicaManager3Irrlicht.h"
#include "BaseIrrlichtReplica.h"
#include "PlayerReplica.h"

ReplicaManager3Irrlicht::ReplicaManager3Irrlicht() {

}

ReplicaManager3Irrlicht::~ReplicaManager3Irrlicht() {

}

RakNet::Replica3 *Connection_RM3Irrlicht::AllocReplica(RakNet::BitStream *allocationId, ReplicaManager3 *replicaManager3)
{
	RakNet::RakString typeName;
    allocationId->Read(typeName);
	if (typeName=="PlayerReplica")
    {
        BaseIrrlichtReplica *r = new PlayerReplica;
        r->demo=demo; return r;
    }
//	if (typeName=="BallReplica")
//    {
//        BaseIrrlichtReplica *r = new BallReplica;
//        r->demo=demo;
//        return r;
//    }
	return 0;
}

Connection_RM3 *ReplicaManager3Irrlicht::AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const {
    return new Connection_RM3Irrlicht(systemAddress,rakNetGUID,this->demo);
}


void ReplicaManager3Irrlicht::DeallocConnection(Connection_RM3 *connection) const {
    delete connection;
}
