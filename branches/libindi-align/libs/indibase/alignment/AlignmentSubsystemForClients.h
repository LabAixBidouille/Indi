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

namespace INDI {
namespace AlignmentSubsystem {

class AlignmentSubsystemForClients : public ClientAPIForMathPluginManagement, public ClientAPIForAlignmentDatabase,
                                    public TelescopeDirectionVectorSupportFunctions
{
public:
    virtual ~AlignmentSubsystemForClients();
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORCLIENTS_H
