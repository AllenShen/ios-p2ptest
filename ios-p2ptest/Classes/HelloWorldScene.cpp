#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "RakNetStuff.h"

using namespace cocos2d;
using namespace CocosDenshion;

HelloWorld* HelloWorld::instance = NULL;

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    instance = this;
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        this,
                                        menu_selector(HelloWorld::menuCloseCallback) );
    pCloseItem->setScale(3);
    pCloseItem->setPosition( ccp(CCDirector::sharedDirector()->getWinSize().width - 200, 20) );

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition( CCPointZero );
    this->addChild(pMenu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    pLabel = CCLabelTTF::create("Hello World", "Thonburi", 34);

    // ask director the window size
    CCSize size = CCDirector::sharedDirector()->getWinSize();

    // position the label on the center of the screen
    pLabel->setPosition( ccp(size.width / 2, size.height - 20) );

    // add the label as a child to this layer
    this->addChild(pLabel, 1);

    // add "HelloWorld" splash screen"
//    CCSprite* pSprite = CCSprite::create("HelloWorld.png");
//
//    // position the sprite on the center of the screen
//    pSprite->setPosition( ccp(size.width/2, size.height/2) );
//
//    // add the sprite as a child to this layer
//    this->addChild(pSprite, 0);

    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("logoanim.plist");

    CCArray* framesArr = CCArray::create();
    for(int i =0;i < 40;i++)
    {
        char framename[50] = {};
        if(i < 10)
            sprintf(framename,"logoanim000%d",i);
        else
            sprintf(framename,"logoanim00%d",i);
        CCSpriteFrame* spriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(framename);
        if(spriteFrame)
            framesArr->addObject(spriteFrame);
    }
    CCAnimation* singleAnim = CCAnimation::createWithSpriteFrames(framesArr, 0.08);

    CCSprite* animSprite = CCSprite::create();
    animSprite->runAction(CCRepeatForever::create(CCAnimate::create(singleAnim)));
    this->addChild(animSprite);
    animSprite->setPosition(ccp(CCDirector::sharedDirector()->getWinSize().width/2, CCDirector::sharedDirector()->getWinSize().height/2));
    animSprite->setAnchorPoint(ccp(0.5, 0.5));

    return true;
}

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    this->unscheduleUpdate();
    P2PConnectManager::getInstance()->startNetWork();
    this->scheduleUpdate();
    
//    CCDirector::sharedDirector()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//    exit(0);
#endif
}

void HelloWorld::update(float delta) {
    CCNode::update(delta);
    P2PConnectManager::getInstance()->UpdateRakNet();
}
