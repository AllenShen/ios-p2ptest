//
// Created by 沈冬冬 on 14-2-12.
//
//


#include "PlayerReplica.h"
#include "GetTime.h"
#include "RakNetStuff.h"

DataStructures::List<PlayerReplica*> PlayerReplica::playerList;

PlayerReplica::PlayerReplica()
{
//    model=0;
    rotationDeltaPerMS=0.0f;
    isMoving=false;
    deathTimeout=0;
    lastUpdate=RakNet::GetTimeMS();
    playerList.Push(this,_FILE_AND_LINE_);
}
PlayerReplica::~PlayerReplica()
{
    unsigned int index = playerList.GetIndexOf(this);
    if (index != (unsigned int) -1)
        playerList.RemoveAtIndexFast(index);
}
void PlayerReplica::WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const
{
    allocationIdBitstream->Write(RakNet::RakString("PlayerReplica"));
}
void PlayerReplica::SerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection)
{
    BaseIrrlichtReplica::SerializeConstruction(constructionBitstream, destinationConnection);
    constructionBitstream->Write(rotationAroundYAxis);
    constructionBitstream->Write(playerName);
    constructionBitstream->Write(IsDead());
}
bool PlayerReplica::DeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection)
{
    if (!BaseIrrlichtReplica::DeserializeConstruction(constructionBitstream, sourceConnection))
        return false;
    constructionBitstream->Read(rotationAroundYAxis);
    constructionBitstream->Read(playerName);
    constructionBitstream->Read(isDead);
    return true;
}
void PlayerReplica::PostDeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection)
{
    // Object was remotely created and all data loaded. Now we can make the object visible
//    scene::IAnimatedMesh* mesh = 0;
//    scene::ISceneManager *sm = demo->GetSceneManager();
//    mesh = sm->getMesh(IRRLICHT_MEDIA_PATH "sydney.md2");
//    model = sm->addAnimatedMeshSceneNode(mesh, 0);
//
//    model->setPosition(position);
//    model->setRotation(core::vector3df(0, rotationAroundYAxis, 0));
//    model->setScale(core::vector3df(2,2,2));
//    model->setMD2Animation(scene::EMAT_STAND);
//    curAnim=scene::EMAT_STAND;
//    model->setMaterialTexture(0, demo->GetDevice()->getVideoDriver()->getTexture(IRRLICHT_MEDIA_PATH "sydney.bmp"));
//    model->setMaterialFlag(video::EMF_LIGHTING, true);
//    model->addShadowVolumeSceneNode();
//    model->setAutomaticCulling ( scene::EAC_BOX );
//    model->setVisible(true);
//    model->setAnimationEndCallback(this);
//    wchar_t playerNameWChar[1024];
//    mbstowcs(playerNameWChar, playerName.C_String(), 1024);
//    scene::IBillboardSceneNode *bb = sm->addBillboardTextSceneNode(0, playerNameWChar, model);
//    bb->setSize(core::dimension2df(40,20));
//    bb->setPosition(core::vector3df(0,model->getBoundingBox().MaxEdge.Y+bb->getBoundingBox().MaxEdge.Y-bb->getBoundingBox().MinEdge.Y+5.0,0));
//    bb->setColor(video::SColor(255,255,128,128), video::SColor(255,255,128,128));
}
void PlayerReplica::PreDestruction(RakNet::Connection_RM3 *sourceConnection)
{
//    if (model)
//        model->remove();
}
RM3SerializationResult PlayerReplica::Serialize(RakNet::SerializeParameters *serializeParameters)
{
    BaseIrrlichtReplica::Serialize(serializeParameters);
//    serializeParameters->outputBitstream[0].Write(position);
    serializeParameters->outputBitstream[0].Write(rotationAroundYAxis);
    serializeParameters->outputBitstream[0].Write(isMoving);
    serializeParameters->outputBitstream[0].Write(IsDead());
    return RM3SR_BROADCAST_IDENTICALLY;
}
void PlayerReplica::Deserialize(RakNet::DeserializeParameters *deserializeParameters)
{
    BaseIrrlichtReplica::Deserialize(deserializeParameters);
//    deserializeParameters->serializationBitstream[0].Read(position);
    deserializeParameters->serializationBitstream[0].Read(rotationAroundYAxis);
    deserializeParameters->serializationBitstream[0].Read(isMoving);
    bool wasDead=isDead;
    deserializeParameters->serializationBitstream[0].Read(isDead);
    if (isDead==true && wasDead==false)
    {
//        demo->PlayDeathSound(position);
    }

//    core::vector3df positionOffset;
//    positionOffset=position-model->getPosition();
//    positionDeltaPerMS = positionOffset / INTERP_TIME_MS;
//    float rotationOffset;
//    rotationOffset=GetRotationDifference(rotationAroundYAxis,model->getRotation().Y);
//    rotationDeltaPerMS = rotationOffset / INTERP_TIME_MS;
//    interpEndTime = RakNet::GetTimeMS() + (RakNet::TimeMS) INTERP_TIME_MS;
}

void PlayerReplica::Update(RakNet::TimeMS curTime)
{
    // Is a locally created object?
    if (creatingSystemGUID==RakNetStuff::rakPeer->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS))
    {
        // Local player has no mesh to interpolate
        // Input our camera position as our player position
//        RakNetStuff::playerReplica->position=demo->GetSceneManager()->getActiveCamera()->getPosition()-irr::core::vector3df(0,CAMERA_HEIGHT,0);
//        RakNetStuff::playerReplica->rotationAroundYAxis=demo->GetSceneManager()->getActiveCamera()->getRotation().Y-90.0f;
//        isMoving=demo->IsMovementKeyDown();
        return;
    }

    // Update interpolation
    RakNet::TimeMS elapsed = curTime-lastUpdate;
    if (elapsed<=1)
        return;
    if (elapsed>100)
        elapsed=100;

    lastUpdate=curTime;
//    irr::core::vector3df curPositionDelta = position-model->getPosition();
//    irr::core::vector3df interpThisTick = positionDeltaPerMS*(float) elapsed;
//    if (curTime < interpEndTime && interpThisTick.getLengthSQ() < curPositionDelta.getLengthSQ())
//    {
//        model->setPosition(model->getPosition()+positionDeltaPerMS*(float) elapsed);
//    }
//    else
//    {
//        model->setPosition(position);
//    }
//
//    float curRotationDelta = GetRotationDifference(rotationAroundYAxis,model->getRotation().Y);
//    float interpThisTickRotation = rotationDeltaPerMS*(float)elapsed;
//    if (curTime < interpEndTime && fabs(interpThisTickRotation) < fabs(curRotationDelta))
//    {
//        model->setRotation(model->getRotation()+core::vector3df(0,interpThisTickRotation,0));
//    }
//    else
//    {
//        model->setRotation(core::vector3df(0,rotationAroundYAxis,0));
//    }
//
//    if (isDead)
//    {
//        UpdateAnimation(scene::EMAT_DEATH_FALLBACK);
//        model->setLoopMode(false);
//    }
//    else if (curAnim!=scene::EMAT_ATTACK)
//    {
//        if (isMoving)
//        {
//            UpdateAnimation(scene::EMAT_RUN);
//            model->setLoopMode(true);
//        }
//        else
//        {
//            UpdateAnimation(scene::EMAT_STAND);
//            model->setLoopMode(true);
//        }
//    }
}

//void PlayerReplica::UpdateAnimation(irr::scene::EMD2_ANIMATION_TYPE anim)
//{
//    if (anim!=curAnim)
//        model->setMD2Animation(anim);
//    curAnim=anim;
//}

float PlayerReplica::GetRotationDifference(float r1, float r2)
{
    float diff = r1-r2;
    while (diff>180.0f)
        diff-=360.0f;
    while (diff<-180.0f)
        diff+=360.0f;
    return diff;
}

//void PlayerReplica::OnAnimationEnd(scene::IAnimatedMeshSceneNode* node)
//{
//    if (curAnim==scene::EMAT_ATTACK)
//    {
//        if (isMoving)
//        {
//            UpdateAnimation(scene::EMAT_RUN);
//            model->setLoopMode(true);
//        }
//        else
//        {
//            UpdateAnimation(scene::EMAT_STAND);
//            model->setLoopMode(true);
//        }
//    }
//}

void PlayerReplica::PlayAttackAnimation(void)
{
    if (isDead==false)
    {
//        UpdateAnimation(scene::EMAT_ATTACK);
//        model->setLoopMode(false);
    }
}
bool PlayerReplica::IsDead(void) const
{
    return deathTimeout > RakNet::GetTimeMS();
}
