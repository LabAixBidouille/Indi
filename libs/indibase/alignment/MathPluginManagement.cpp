/*!
 * \file MathPluginManagement.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#include "MathPluginManagement.h"

namespace INDI {
namespace AlignmentSubsystem {

void MathPluginManagement::InitProperties(Telescope* ChildTelescope)
{
    // TODO Find out the available math plugins and populate the array
    // Just use the default built in plugin for the time being
    AlignmentSubsystemMathPlugins.reset(new ISwitch[1]);
    IUFillSwitch(AlignmentSubsystemMathPlugins.get(), "INBUILT_MATH_PLUGIN", "Inbuilt Math Plugin", ISS_ON);
    IUFillSwitchVector(&AlignmentSubsystemMathPluginsV, AlignmentSubsystemMathPlugins.get(), 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_SUBSYSTEM_MATH_PLUGINS", "Math Plugins", ALIGNMENT_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentSubsystemMathPluginsV, INDI_SWITCH);

    // The following property is used for configuration purposes only and is not exposed to the client.
    IUFillText(&AlignmentSubsystemCurrentMathPlugin, "ALIGNMENT_SUBSYSTEM_CURRENT_MATH_PLUGIN", "Current Math Plugin",
        AlignmentSubsystemMathPlugins.get()[0].label);
    IUFillTextVector(&AlignmentSubsystemCurrentMathPluginV, &AlignmentSubsystemCurrentMathPlugin, 1, ChildTelescope->getDeviceName(),
                "ALIGNMENT_SUBSYSTEM_CURRENT_MATH_PLUGIN", "Current Math Plugin", ALIGNMENT_TAB, IP_RO, 60, IPS_IDLE);
}

void MathPluginManagement::ProcessSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessSwitchProperties - name(%s)", name);
    if (strcmp(name, AlignmentSubsystemMathPluginsV.name) == 0)
    {
        AlignmentSubsystemMathPluginsV.s=IPS_OK;
        IUUpdateSwitch(&AlignmentSubsystemMathPluginsV, states, names, n);
        //  Update client display
        IDSetSwitch(&AlignmentSubsystemMathPluginsV, NULL);

        // TODO Handle loading anf unloading plugins
    }
}

void MathPluginManagement::SaveConfigProperties(FILE *fp)
{
    IUSaveConfigText(fp, &AlignmentSubsystemCurrentMathPluginV);
}

} // namespace AlignmentSubsystem
} // namespace INDI
