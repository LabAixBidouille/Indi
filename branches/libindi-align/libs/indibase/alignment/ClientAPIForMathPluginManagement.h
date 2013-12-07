/*!
 * \file ClientAPIForMathPluginManagement.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#ifndef INDI_ALIGNMENTSUBSYSTEM_CLIENTAPIFORMATHPLUGINMANAGEMENT_H
#define INDI_ALIGNMENTSUBSYSTEM_CLIENTAPIFORMATHPLUGINMANAGEMENT_H

#include <vector>
#include <string>

namespace INDI {
namespace AlignmentSubsystem {

class ClientAPIForMathPluginManagement
{
public:
    virtual ~ClientAPIForMathPluginManagement() {}

    typedef std::vector<std::string> MathPluginsList;

    /*!
     * \brief Return a list of the names of the available math plugins.
     * \param[out] MathPlugins Reference to a list of the names of the available math plugins.
     * \return False on failure
     */
    bool EnumerateMathPlugins(MathPluginsList& MathPlugins);

    /*!
     * \brief Selects, loads and initialises the named math plugin.
     * \param[in] MathPluginName The name of the required math plugin.
     * \return False on failure.
     */
    bool SelectMathPlugin(const std::string& MathPluginName);
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_CLIENTAPIFORMATHPLUGINMANAGEMENT_H
