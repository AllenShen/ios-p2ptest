//
// Created by 沈冬冬 on 14-2-12.
//
//



#ifndef __PlayerReplica_H_
#define __PlayerReplica_H_

#include "Rand.h"
#include "ReplicaManager3.h"
#include "BaseIrrlichtReplica.h"

using namespace RakNet;

class CDemo;

class PlayerReplica : public BaseIrrlichtReplica{
public:
    PlayerReplica();
    virtual ~PlayerReplica();
    // Every function below, before Update overriding a function in Replica3
    virtual void WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const;
    virtual void SerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection);
    virtual bool DeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection);
    virtual RakNet::RM3SerializationResult Serialize(RakNet::SerializeParameters *serializeParameters);
    virtual void Deserialize(RakNet::DeserializeParameters *deserializeParameters);
    virtual void PostDeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection);
    virtual void PreDestruction(RakNet::Connection_RM3 *sourceConnection);

    virtual void Update(RakNet::TimeMS curTime);
//    void UpdateAnimation(irr::scene::EMD2_ANIMATION_TYPE anim);
    float GetRotationDifference(float r1, float r2);
//    virtual void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode* node);
    void PlayAttackAnimation(void);

    // playerName is only sent in SerializeConstruction, since it doesn't change
    RakNet::RakString playerName;

    // Networked rotation
    float rotationAroundYAxis;
    // Interpolation variables, not networked
//    irr::core::vector3df positionDeltaPerMS;
    float rotationDeltaPerMS;
    RakNet::TimeMS interpEndTime, lastUpdate;

    // Updated based on the keypresses, to control remote animation
    bool isMoving;

    // Only instantiated for remote systems, you never see yourself
//    irr::scene::IAnimatedMeshSceneNode* model;
//    irr::scene::EMD2_ANIMATION_TYPE curAnim;

    // deathTimeout is set from the local player
    RakNet::TimeMS deathTimeout;
    bool IsDead(void) const;
    // isDead is set from network packets for remote players
    bool isDead;

    // List of all players, including our own
    static DataStructures::List<PlayerReplica*> playerList;
};

#endif //__PlayerReplica_H_
