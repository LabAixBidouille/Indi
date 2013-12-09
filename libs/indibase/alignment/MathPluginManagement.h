/*!
 * \file MathPluginManagement.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#ifndef INDI_ALIGNMENTSUBSYSTEM_MATHPLUGINMANAGEMENT_H
#define INDI_ALIGNMENTSUBSYSTEM_MATHPLUGINMANAGEMENT_H

#include "indibase/inditelescope.h"

#include <memory>

namespace INDI {
namespace AlignmentSubsystem {

/*!
 * \class MathPluginManagement
 * \brief The following INDI properties are used to manage math plugins
 *  - ALIGNMENT_SUBSYSTEM_MATH_PLUGINS\n
 *  A list of available plugins (switch). This also allows the client to select a plugin.
 *  - ALIGNMENT_SUBSYSTEM_CURRENT_MATH_PLUGIN\n
 *  This is not communicated to the client and only used to save the currently selected plugin name
 *  to persistent storage.
 */
class MathPluginManagement
{
public:
    virtual ~MathPluginManagement() {}

    /** \brief Initilize alignment math plugin properties. It is recommended to call this function within initProperties() of your primary device
     * \param[in] pTelescope Pointer to the child INDI::Telecope class
    */
    void InitProperties(Telescope* pTelescope);

    /** \brief Call this function whenever a client updates a switch property. The function will
     * handle any math plugin related properties.
     * \param[in] pTelescope Pointer to the child INDI::Telecope class
     * \param[in] name vector property name
     * \param[in] states states as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n);

    /** \brief Call this function to save persistent math plugin properties.
     * This function should be called from within the saveConfigItems function of your driver.
     * \param[in] fp File pointer passed into saveConfigItems
    */
    void SaveConfigProperties(FILE *fp);

private:

    std::auto_ptr<ISwitch> AlignmentSubsystemMathPlugins;
    ISwitchVectorProperty AlignmentSubsystemMathPluginsV;

    // The following property is used for configuration purposes only and is not propagated to the client
    IText AlignmentSubsystemCurrentMathPlugin;
    ITextVectorProperty AlignmentSubsystemCurrentMathPluginV;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_MATHPLUGINMANAGEMENT_H
