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

#include "BuiltInMathPlugin.h"

#include <memory>

namespace INDI {
namespace AlignmentSubsystem {

/*!
 * \class MathPluginManagement
 * \brief The following INDI properties are used to manage math plugins
 *  - ALIGNMENT_SUBSYSTEM_MATH_PLUGINS\n
 *  A list of available plugins (switch). This also allows the client to select a plugin.
 *  - ALIGNMENT_SUBSYSTEM_MATH_PLUGIN_INITIALISE\n
 *  Initialise or re-initialise the current math plugin.
 *  - ALIGNMENT_SUBSYSTEM_CURRENT_MATH_PLUGIN\n
 *  This is not communicated to the client and only used to save the currently selected plugin name
 *  to persistent storage.
 *
 *  This class also provides function links to the currently selected math plugin
 */
class MathPluginManagement : public MathPlugin // Derive from MathPluign to force the function signatures to match
{
public:
    MathPluginManagement() : pInitialise(&MathPlugin::Initialise),
                            pTransformCelestialToTelescope(&MathPlugin::TransformCelestialToTelescope),
                            pTransformTelescopeToCelestial(&MathPlugin::TransformTelescopeToCelestial),
                            pLoadedMathPlugin(&BuiltInPlugin), LoadedMathPluginHandle(NULL) {}
    virtual ~MathPluginManagement() {}

    /** \brief Initilize alignment math plugin properties. It is recommended to call this function within initProperties() of your primary device
     * \param[in] pTelescope Pointer to the child INDI::Telecope class
    */
    void InitProperties(Telescope* pTelescope);

    /** \brief Call this function from within the ISNewSwitch processing path. The function will
     * handle any math plugin switch properties.
     * \param[in] pTelescope Pointer to the child INDI::Telecope class
     * \param[in] name vector property name
     * \param[in] states states as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n);

    /** \brief Call this function from within the ISNewText processing path. The function will
     * handle any math plugin text properties. This only text property at the moment is contained in the
     * config file so this will normally only have work to do when the config file is loaded.
     * \param[in] pTelescope Pointer to the child INDI::Telecope class
     * \param[in] name vector property name
     * \param[in] texts texts as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessTextProperties(Telescope* pTelescope, const char *name, char *texts[], char *names[], int n);

    /** \brief Call this function to save persistent math plugin properties.
     * This function should be called from within the saveConfigItems function of your driver.
     * \param[in] fp File pointer passed into saveConfigItems
    */
    void SaveConfigProperties(FILE *fp);

    // These must match the function signatures in MathPlugin
    bool Initialise();
    bool TransformCelestialToTelescope(const double RightAscension, const double Declination, double JulianOffset,
                                            TelescopeDirectionVector& ApparentTelescopeDirectionVector);
    bool TransformTelescopeToCelestial(const TelescopeDirectionVector& ApparentTelescopeDirectionVector, double& RightAscension, double& Declination);


private:
    void EnumeratePlugins();
    std::vector<std::string> MathPluginFiles;
    std::vector<std::string> MathPluginDisplayNames;

    std::auto_ptr<ISwitch> AlignmentSubsystemMathPlugins;
    ISwitchVectorProperty AlignmentSubsystemMathPluginsV;
    ISwitch AlignmentSubsystemMathPluginInitialise;
    ISwitchVectorProperty AlignmentSubsystemMathPluginInitialiseV;

    // The following property is used for configuration purposes only and is not propagated to the client
    IText AlignmentSubsystemCurrentMathPlugin;
    ITextVectorProperty AlignmentSubsystemCurrentMathPluginV;

    // The following hold links to the current loaded math plugin
    // These must match the function signatures in MathPlugin
    bool (MathPlugin::*pInitialise)();
    bool (MathPlugin::*pTransformCelestialToTelescope)(const double RightAscension, const double Declination, double JulianOffset,
                                                        TelescopeDirectionVector& TelescopeDirectionVector);
    bool (MathPlugin::*pTransformTelescopeToCelestial)(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination);
    MathPlugin* pLoadedMathPlugin;
    void* LoadedMathPluginHandle;

    BuiltInMathPlugin BuiltInPlugin;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_MATHPLUGINMANAGEMENT_H
