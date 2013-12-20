#include "LoaderClient.h"



#include <cstring>


#define MY_DEVICE "skywatcherAPIMount"

LoaderClient::LoaderClient()
{
    //ctor
}

LoaderClient::~LoaderClient()
{
    //dtor
}

void LoaderClient::Initialise()
{
    setServer("localhost", 7624);

    watchDevice(MY_DEVICE);

    connectServer();
}

void LoaderClient::newDevice(INDI::BaseDevice *dp)
{
    if (!strcmp(dp->getDeviceName(), MY_DEVICE))
        IDLog("Receiving %s Device...\n", dp->getDeviceName());

    Device = dp;
}

void LoaderClient::newProperty(INDI::Property *property)
{

    if (!strcmp(property->getDeviceName(), MY_DEVICE))
        IDLog("newProperty %s\n", property->getName());
}


