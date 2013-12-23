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

void AlignmentSubsystemForClients::Initialise(const char * DeviceName)
{
    AlignmentSubsystemForClients::DeviceName = DeviceName;
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

} // namespace AlignmentSubsystem
} // namespace INDI
