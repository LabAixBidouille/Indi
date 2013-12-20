#ifndef LOADERCLIENT_H
#define LOADERCLIENT_H

#include "indibase/baseclient.h"
#include "indibase/basedevice.h"

class LoaderClient : public INDI::BaseClient
{
public:
    LoaderClient();
    virtual ~LoaderClient();

    void Initialise();

protected:
    virtual void newDevice(INDI::BaseDevice *dp);
    virtual void newProperty(INDI::Property *property);
    virtual void removeProperty(INDI::Property *property) {}
    virtual void newBLOB(IBLOB *bp) {}
    virtual void newSwitch(ISwitchVectorProperty *svp) {}
    virtual void newNumber(INumberVectorProperty *nvp) {}
    virtual void newMessage(INDI::BaseDevice *dp, int messageID) {}
    virtual void newText(ITextVectorProperty *tvp) {}
    virtual void newLight(ILightVectorProperty *lvp) {}
    virtual void serverConnected() {}
    virtual void serverDisconnected(int exit_code) {}

private:
    INDI::BaseDevice *Device;
};

#endif // LOADERCLIENT_H
