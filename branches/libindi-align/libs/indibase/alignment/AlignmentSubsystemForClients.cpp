/*!
 * \file AlignmentSubsystemForClients.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#include "AlignmentSubsystemForClients.h"

#include <cstring>

namespace INDI {
namespace AlignmentSubsystem {

void AlignmentSubsystemForClients::Initialise(const char * DeviceName, INDI::BaseClient * BaseClient)
{
    AlignmentSubsystemForClients::DeviceName = DeviceName;
    ClientAPIForAlignmentDatabase::Initialise(BaseClient);

}

void AlignmentSubsystemForClients::ProcessNewDevice(INDI::BaseDevice *DevicePointer)
{
    if (!strcmp(DevicePointer->getDeviceName(), DeviceName.c_str()))
    {
        IDLog("Receiving %s Device...\n", DevicePointer->getDeviceName());
        ClientAPIForAlignmentDatabase::ProcessNewDevice(DevicePointer);
    }
}

void AlignmentSubsystemForClients::ProcessNewProperty(INDI::Property *PropertyPointer)
{
    if (!strcmp(PropertyPointer->getDeviceName(), DeviceName.c_str()))
    {
        IDLog("newProperty %s\n", PropertyPointer->getName());
        ClientAPIForAlignmentDatabase::ProcessNewProperty(PropertyPointer);
    }
}

void AlignmentSubsystemForClients::ProcessNewSwitch(ISwitchVectorProperty *SwitchVectorPropertyPointer)
{
    if (!strcmp(SwitchVectorPropertyPointer->device, DeviceName.c_str()))
    {
        IDLog("newSwitch %s\n", SwitchVectorPropertyPointer->name);
        ClientAPIForAlignmentDatabase::ProcessNewSwitch(SwitchVectorPropertyPointer);
    }
}

void AlignmentSubsystemForClients::ProcessNewNumber(INumberVectorProperty *NumberVectorPropertyPointer)
{
    if (!strcmp(NumberVectorPropertyPointer->device, DeviceName.c_str()))
    {
        IDLog("newNumber %s\n", NumberVectorPropertyPointer->name);
        ClientAPIForAlignmentDatabase::ProcessNewNumber(NumberVectorPropertyPointer);
    }
}

} // namespace AlignmentSubsystem
} // namespace INDI