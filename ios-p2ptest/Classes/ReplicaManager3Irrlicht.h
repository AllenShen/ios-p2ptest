//
// Created by 沈冬冬 on 14-2-12.
//
//



#ifndef __ReplicaManager3Irrlicht_H_
#define __ReplicaManager3Irrlicht_H_

#include <iostream>
#include "Rand.h"
#include "ReplicaManager3.h"

using namespace RakNet;

class CDemo;

class Connection_RM3Irrlicht : public RakNet::Connection_RM3 {
public:
    Connection_RM3Irrlicht(const SystemAddress &_systemAddress,RakNetGUID _guid, CDemo *_demo):Connection_RM3(_systemAddress, _guid)
    {
        demo=_demo;
    }
    virtual ~Connection_RM3Irrlicht() {};

    virtual RakNet::Replica3 *AllocReplica(RakNet::BitStream *allocationId, RakNet::ReplicaManager3 *replicaManager3);
protected:
    CDemo *demo;
};

class ReplicaManager3Irrlicht : public RakNet::ReplicaManager3
{
public:

    CDemo *demo;

    ReplicaManager3Irrlicht();

    virtual ~ReplicaManager3Irrlicht();

    virtual Connection_RM3 *AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const;

    virtual void DeallocConnection(Connection_RM3 *connection) const;
};


#endif //__ReplicaManager3Irrlicht_H_
