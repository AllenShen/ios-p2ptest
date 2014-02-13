//
// Created by 沈冬冬 on 14-2-12.
//
//


#include "BaseIrrlichtReplica.h"

BaseIrrlichtReplica::BaseIrrlichtReplica()
{
}
BaseIrrlichtReplica::~BaseIrrlichtReplica()
{

}
void BaseIrrlichtReplica::SerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection)
{
//    constructionBitstream->Write(position);
}
bool BaseIrrlichtReplica::DeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection)
{
//    constructionBitstream->Read(position);
    return true;
}
RM3SerializationResult BaseIrrlichtReplica::Serialize(RakNet::SerializeParameters *serializeParameters)
{
    return RM3SR_BROADCAST_IDENTICALLY;
}
void BaseIrrlichtReplica::Deserialize(RakNet::DeserializeParameters *deserializeParameters)
{
}
void BaseIrrlichtReplica::Update(RakNet::TimeMS curTime)
{
}
