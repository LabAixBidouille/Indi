/*!
 * \file AlignmentSubsystemForClients.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file provides a shorthand way for clients to include all the
 * functionality they need to use the INDI Alignment Subsystem
 * Clients should inherit this class alongside INDI::BaseClient
 */

#ifndef INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORCLIENTS_H
#define INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORCLIENTS_H

#include "ClientAPIForAlignmentDatabase.h"
#include "ClientAPIForMathPluginManagement.h"
#include "TelescopeDirectionVectorSupportFunctions.h"

#include "indibase/basedevice.h"


namespace INDI {
namespace AlignmentSubsystem {

/*!
 * \class AlignmentSubsystemForClients
 * \brief This class encapsulates all the alignment subsystem classes that are useful to client implementations.
 * Clients should inherit from this class.
 */
class AlignmentSubsystemForClients : public ClientAPIForMathPluginManagement, public ClientAPIForAlignmentDatabase,
                                    public TelescopeDirectionVectorSupportFunctions
{
public:
    virtual ~AlignmentSubsystemForClients() {}

    /** \brief This routine should be called before any connections to devices are made.
        \param[in] DeviceName The device name of INDI driver instance to be used.
    */
    void Initialise(const char * DeviceName);

    /** \brief Process new device message from driver. This routine should be called from within
     the newDevice handler in the client.
        \param[in] DevicePointer A pointer to the INDI::BaseDevice object.
    */
    void ProcessNewDevice(INDI::BaseDevice *DevicePointer);

    /** \brief Process new property message from driver. This routine should be called from within
     the newProperty handler in the client.
        \param[in] PropertyPointer A pointer to the INDI::Property object.
    */
    void ProcessNewProperty(INDI::Property *PropertyPointer);

private:
    std::string DeviceName;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORCLIENTS_H
