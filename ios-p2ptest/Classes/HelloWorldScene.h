#include "CCPlatformMacros.h"

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;
#include "P2PConnectManager.h"

class HelloWorld : public cocos2d::CCLayer
{

public:
    static HelloWorld* instance;

public:

    CCLabelTTF* pLabel;

    // Method 'init' in cocos2d-x returns bool, instead of 'id' in cocos2d-iphone (an object pointer)
    virtual bool init();

    // there's no 'id' in cpp, so we recommend to return the class instance pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    void menuCloseCallback(CCObject* pSender);

    // preprocessor macro for "static create()" constructor ( node() deprecated )
    CREATE_FUNC(HelloWorld);

    virtual void update(float delta) override;
};

#endif // __HELLOWORLD_SCENE_H__
