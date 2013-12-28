/*!
 * \file ClientAPIForMathPluginManagement.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#include "ClientAPIForMathPluginManagement.h"

#include <cstring>

namespace INDI {
namespace AlignmentSubsystem {

void ClientAPIForMathPluginManagement::Initialise(INDI::BaseClient *BaseClient)
{
    ClientAPIForMathPluginManagement::BaseClient = BaseClient;
}

void ClientAPIForMathPluginManagement::ProcessNewDevice(INDI::BaseDevice *DevicePointer)
{
    Device = DevicePointer;
}

void ClientAPIForMathPluginManagement::ProcessNewProperty(INDI::Property *PropertyPointer)
{
    int ReturnCode;
    bool GotOneOfMine = true;

    if (!strcmp(PropertyPointer->getName(), "ALIGNMENT_SUBSYSTEM_MATH_PLUGINS"))
        MathPlugins = PropertyPointer;
    else if (!strcmp(PropertyPointer->getName(), "ALIGNMENT_SUBSYSTEM_MATH_PLUGIN_INITIALISE"))
        PluginInitialise = PropertyPointer;
    else
        GotOneOfMine = false;

    // Tell the client when all the database proeprties have been set up
    if (GotOneOfMine
                && (NULL != MathPlugins)
                && (NULL != PluginInitialise))
    {
        // The DriverActionComplete state variable is initialised to false
        // So I need to call this to set it to true and signal anyone
        // waiting for the driver to initialise etc.
        SignalDriverCompletion();
    }
}

void ClientAPIForMathPluginManagement::ProcessNewSwitch(ISwitchVectorProperty *SwitchVectorProperty)
{
    if (!strcmp(SwitchVectorProperty->name, "ALIGNMENT_SUBSYSTEM_MATH_PLUGINS"))
    {
        if (IPS_BUSY != SwitchVectorProperty->s)
            SignalDriverCompletion();
    }
    else if (!strcmp(SwitchVectorProperty->name, "ALIGNMENT_SUBSYSTEM_MATH_PLUGIN_INITIALISE"))
    {
        if (IPS_BUSY != SwitchVectorProperty->s)
            SignalDriverCompletion();
    }
}

bool ClientAPIForMathPluginManagement::EnumerateMathPlugins(MathPluginsList& AvailableMathPlugins)
{
    // Wait for driver to initialise if neccessary
    WaitForDriverCompletion();

    AvailableMathPlugins.clear();

    ISwitchVectorProperty *pPlugins = MathPlugins->getSwitch();

    for (int i = 0; i < pPlugins->nsp; i++)
        AvailableMathPlugins.push_back(std::string(pPlugins->sp[i].label));

    return true;
}

bool ClientAPIForMathPluginManagement::SelectMathPlugin(const std::string& MathPluginName)
{
    // Wait for driver to initialise if neccessary
    WaitForDriverCompletion();

    ISwitchVectorProperty *pPlugins = MathPlugins->getSwitch();

    int i;
    for (i = 0; i < pPlugins->nsp; i++)
    {
        if (0 == strcmp(MathPluginName.c_str(), pPlugins->sp[i].label))
            break;
    }
    if (i >= pPlugins->nsp)
        return false;


    IUResetSwitch(pPlugins);
    pPlugins->sp[i].s = ISS_ON;
    SetDriverBusy();
    BaseClient->sendNewSwitch(pPlugins);
    WaitForDriverCompletion();
    if (IPS_OK != pPlugins->s)
    {
        IDLog("SelectMathPlugin - Bad MathPlugins switch state %s\n", pstateStr(pPlugins->s));
        return false;
    }
    return true;
}

bool ClientAPIForMathPluginManagement::ReInitialiseMathPlugin()
{
    // Wait for driver to initialise if neccessary
    WaitForDriverCompletion();

    ISwitchVectorProperty *pPluginInitialise = PluginInitialise->getSwitch();

    IUResetSwitch(pPluginInitialise);
    pPluginInitialise->sp[0].s = ISS_ON;
    SetDriverBusy();
    BaseClient->sendNewSwitch(pPluginInitialise);
    WaitForDriverCompletion();
    if (IPS_OK != pPluginInitialise->s)
    {
        IDLog("ReInitialiseMathPlugin - Bad PluginInitialise switch state %s\n", pstateStr(pPluginInitialise->s));
        return false;
    }
    return true;
}

bool ClientAPIForMathPluginManagement::WaitForDriverCompletion()
{
    int ReturnCode;
    ReturnCode = pthread_mutex_lock(&DriverActionCompleteMutex);
    while(!DriverActionComplete)
    {
        IDLog("WaitForDriverCompletion - Waiting\n");
        ReturnCode = pthread_cond_wait(&DriverActionCompleteCondition, &DriverActionCompleteMutex);
        IDLog("WaitForDriverCompletion - Back from wait ReturnCode = %d\n", ReturnCode);
        if (ReturnCode)
        {
            ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
            return false;
        }
    }
    IDLog("WaitForDriverCompletion - Finished waiting\n");
    ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    else
        return true;
}

bool ClientAPIForMathPluginManagement::SignalDriverCompletion()
{
    int ReturnCode;
    ReturnCode = pthread_mutex_lock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    DriverActionComplete = true;
    ReturnCode = pthread_cond_signal(&DriverActionCompleteCondition);
    if (ReturnCode)
    {
        ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
        return false;
    }
    IDLog("SignalDriverCompletion\n");
    ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    else
        return true;
}

bool ClientAPIForMathPluginManagement::SetDriverBusy()
{
    int ReturnCode;
    ReturnCode = pthread_mutex_lock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    DriverActionComplete = false;
    IDLog("SetDriverBusy\n");
    ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    else
        return true;
}

} // namespace AlignmentSubsystem
} // namespace INDI
