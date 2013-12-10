#ifndef INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORMATHPLUGINS_H
#define INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORMATHPLUGINS_H

#include "MathPlugin.h"
#include "InMemoryDatabase.h"
#include "TelescopeDirectionVectorSupportFunctions.h"

namespace INDI {
namespace AlignmentSubsystem {

class AlignmentSubsystemForMathPlugins : public MathPlugin, public InMemoryDatabase, public TelescopeDirectionVectorSupportFunctions
{
    public:
        virtual ~AlignmentSubsystemForMathPlugins() {}
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORMATHPLUGINS_H
