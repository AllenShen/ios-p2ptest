//
// Created by 沈冬冬 on 14-2-20.
//
//


#include "NatPunchThroughHandler.h"
#include "NatPunchthroughClient.h"

NatPunchThroughHandler::NatPunchThroughHandler():
punchthroughClient(NULL)
{
    punchthroughClient = new NatPunchthroughClient();
}

NatPunchThroughHandler::~NatPunchThroughHandler() {

}