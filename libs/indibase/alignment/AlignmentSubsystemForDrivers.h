/*!
 * \file AlignmentSubsystemForDrivers.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file provides a shorthand way for drivers to include all the
 * functionality they need to use the INDI Alignment Subsystem
 * Clients should inherit this class alongside INDI::Telescope or a similar class
 */

#ifndef INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORDRIVERS_H
#define INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORDRIVERS_H

#include "MapPropertiesToInMemoryDatabase.h"
#include "MathPluginManagement.h"
#include "TelescopeDirectionVectorSupportFunctions.h"

namespace INDI {
namespace AlignmentSubsystem {

class AlignmentSubsystemForDrivers : public MapPropertiesToInMemoryDatabase, public MathPluginManagement,
                                    public TelescopeDirectionVectorSupportFunctions
{
public:
    virtual ~AlignmentSubsystemForDrivers() {}

    /** \brief Initilize alignment subsystem properties. It is recommended to call this function within initProperties() of your primary device
        \param[in] deviceName Name of the primary device
    */
    void InitProperties(Telescope* pTelescope);

    /** \brief Call this function whenever a client updates a number property. The function will
     * handle any alignment database related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessNumberProperties(Telescope* pTelescope, const char *name, double values[], char *names[], int n);

    /** \brief Call this function whenever a client updates a switch property. The function will
     * handle any alignment database related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n);

    /** \brief Call this function whenever a client updates a blob property. The function will
     * handle any alignment database related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessBlobProperties(Telescope* pTelescope, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n);

    /** \brief Call this function to save persistent math plugin properties.
     * This function should be called from within the saveConfigItems function of your driver.
     * \param[in] fp File pointer passed into saveConfigItems
    */
    void SaveConfigProperties(FILE *fp);
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_ALIGNMENTSUBSYSTEMFORDRIVERS_H
