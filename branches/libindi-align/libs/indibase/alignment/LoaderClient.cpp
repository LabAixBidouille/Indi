#include "LoaderClient.h"



#include <cstring>
#include <sstream>


using namespace INDI::AlignmentSubsystem;

LoaderClient::LoaderClient() : DeviceName("skywatcherAPIMount")
{
    //ctor
}

LoaderClient::~LoaderClient()
{
    //dtor
}

void LoaderClient::Initialise(int argc, char* argv[])
{
    std::string HostName("localhost");
    int Port = 7624;

    if (argc > 1)
        DeviceName = argv[1];
    if (argc > 2)
        HostName = argv[2];
    if (argc > 3)
    {
        std::istringstream Parameter(argv[3]);
        Parameter >> Port;
    }

    AlignmentSubsystemForClients::Initialise(DeviceName.c_str(), this);

    setServer(HostName.c_str(), Port);

    watchDevice(DeviceName.c_str());

    connectServer();
}

void LoaderClient::newDevice(INDI::BaseDevice *dp)
{
    ProcessNewDevice(dp);
}

void LoaderClient::newProperty(INDI::Property *property)
{
    ProcessNewProperty(property);
}

void LoaderClient::newSwitch(ISwitchVectorProperty *svp)
{
    ProcessNewSwitch(svp);
}

void LoaderClient::newNumber(INumberVectorProperty *nvp)
{
    ProcessNewNumber(nvp);
}

void LoaderClient::newBLOB(IBLOB *bp)
{
    ProcessNewBLOB(bp);
}


void LoaderClient::Load()
{
    AlignmentDatabaseEntry CurrentValues;
    AppendSyncPoint(CurrentValues);
    AppendSyncPoint(CurrentValues);
    AppendSyncPoint(CurrentValues);
    CurrentValues.ObservationDate = 128;
    EditSyncPoint(2, CurrentValues);
    CurrentValues.ObservationDate = 256;
    InsertSyncPoint(2, CurrentValues);
    DeleteSyncPoint(0);
}


