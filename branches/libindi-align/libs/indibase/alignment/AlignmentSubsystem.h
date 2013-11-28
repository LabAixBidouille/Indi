/*!
 * \file AlignmentSubsystem.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file contains a declarations of the functionality for an alignment subsystem to be used
 * alongside the INDI::Telescope class in drivers and directly in clients.
 */
#ifndef ALIGNMENTSUBSYSTEM_H
#define ALIGNMENTSUBSYSTEM_H

#include <vector>
#include <string>
#include <memory>

#include <libnova.h>

#include "inditelescope.h"

namespace INDI
{
    class TelescopeDirectionVector;
    class AlignmentSubsystemClient;
    class AlignmentSubsystemDriver;
    class AlignmentSubsystemMathPlugin;
    class AlignmentSubsystemBase;
}

/*!
 * \class INDI::TelescopeDirectionVector
 * \brief Holds a nomalised direction vector (direction cosines)
 *
 * The x y,z fields of this class should always represent a normalised (unit length)
 * vector in a right handed rectangular coordinate space.
 */
class INDI::TelescopeDirectionVector
{
public:
    double x;
    double y;
    double z;
};

/*!
 * \class INDI::AlignmentSubsystemBase
 * \brief Provides common alignment subsystem functions that are used in clients, drivers, and math plugins
 *
 */
class INDI::AlignmentSubsystemBase
{
public:
    /*! @name Telescope direction vector helper functions
     *  These functions are used to convert different coordinate systems to and from the
     *  telescope direction vectors (normalised vector/direction cosines) used for telescope coordinates in the
     *  alignment susbsystem.
     *  They are intended to be used by clients, drivers, and math plugins are are therefore declared static
     */
    ///@{

    /*!
     * \enum AzimuthAngleDirection
     * The direction of measurement of an azimuth angle
     */
    enum AzimuthAngleDirection{ CLOCKWISE, /*!< Angle is measured clockwise */
                                ANTI_CLOCKWISE /*!< Angle is measured anti clockwise */};

    enum PolarAngleDirection{ FROM_POLAR_AXIS, /*!< Angle is measured down from the polar axis */
                            FROM_AZIMUTHAL_PLANE /*!< Angle is measured upwards from the azimuthal plane */};

    /*! \brief Calculates a telescope direction vector from the supplied spherical coordinate information
     * \param[in] AzimuthAngle The azimuth angle in radians
     * \param[in] AzimuthAngleDirection The direction the azimuth angle has been measured either CLOCKWISE or ANTI_CLOCKWISE
     * \param[in] PolarAngle The polar angle in radians
     * \param[in] PolarAngleDirection The direction the polar angle has been measured either FROM_POLAR_AXIS or FROM_AZIMUTHAL_PLANE
     * \return A TelescopeDirectionVector
     * \note TelescopeDirectionVectors are always normalised and right handed.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromSphericalCoordinate(const double AzimuthAngle, AzimuthAngleDirection AzimuthAngleDirection,
                                                                                            const double PolarAngle, PolarAngleDirection PolarAngleDirection);


    /*! \brief Calculates a spherical coordinate from the supplied telescope direction vector
     * \param[in] TelescopeDirectionVector
     * \param[out] AzimuthAngle The azimuth angle in radians
     * \param[in] AzimuthAngleDirection The direction the azimuth angle has been measured either CLOCKWISE or ANTI_CLOCKWISE
     * \param[out] PolarAngle The polar angle in radians
     * \param[in] PolarAngleDirection The direction the polar angle has been measured either FROM_POLAR_AXIS or FROM_AZIMUTHAL_PLANE
     * \note TelescopeDirectionVectors are always normalised and right handed.
     */
    static void SphericalCoordinateFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                const double& AzimuthAngle, AzimuthAngleDirection AzimuthAngleDirection,
                                                                const double& PolarAngle, PolarAngleDirection PolarAngleDirection);


    /*! \brief Calculates a telescope direction vector from the supplied equatorial coordinates.
     * \param[in] EquatorialCoordinates The equatorial cordinates in decimal degrees
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the direction vector with the right ascension being in the XY plane.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromEquatorialCoordinates(struct ln_equ_posn EquatorialCoordinates)
    {
        return TelescopeDirectionVectorFromSphericalCoordinate(ln_deg_to_rad(EquatorialCoordinates.ra), ANTI_CLOCKWISE, ln_deg_to_rad(EquatorialCoordinates.dec), FROM_AZIMUTHAL_PLANE);
    };

    /*! \brief Calculates a telescope direction vector from the supplied equatorial coordinates.
     * \param[in] EquatorialCoordinates The equatorial cordinates in hours minutes seconds and degrees minutes seconds
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the direction vector with the right ascension being in the XY plane.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromEquatorialCoordinates(struct lnh_equ_posn EquatorialCoordinates)
    {
        return TelescopeDirectionVectorFromSphericalCoordinate(ln_hms_to_rad(&EquatorialCoordinates.ra), ANTI_CLOCKWISE, ln_dms_to_rad(&EquatorialCoordinates.dec), FROM_AZIMUTHAL_PLANE);
    };

    /*! \brief Calculates equatorial coordinates from the supplied telescope direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] EquatorialCoordinates The equatorial coordinates in decimal degrees
     * \note This assumes a right handed coordinate system for the direction vector with the right ascension being in the XY plane.
     */
    static void EquatorialCoordinatesFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                struct ln_equ_posn& EquatorialCoordinates)
    {
        double AzimuthAngle;
        double PolarAngle;
        SphericalCoordinateFromTelescopeDirectionVector(TelescopeDirectionVector, AzimuthAngle, ANTI_CLOCKWISE, PolarAngle, FROM_AZIMUTHAL_PLANE);
        EquatorialCoordinates.ra = ln_rad_to_deg(AzimuthAngle);
        EquatorialCoordinates.dec = ln_rad_to_deg(PolarAngle);
    };

    /*! \brief Calculates equatorial coordinates from the supplied telescope direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] EquatorialCoordinates The equatorial coordinates in hours minutes seconds and degrees minutes seconds
     * \note This assumes a right handed coordinate system for the direction vector with the right ascension being in the XY plane.
     */
    static void EquatorialCoordinatesFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                struct lnh_equ_posn& EquatorialCoordinates)
    {
        double AzimuthAngle;
        double PolarAngle;
        SphericalCoordinateFromTelescopeDirectionVector(TelescopeDirectionVector, AzimuthAngle, ANTI_CLOCKWISE, PolarAngle, FROM_AZIMUTHAL_PLANE);
        ln_rad_to_hms(AzimuthAngle, &EquatorialCoordinates.ra);
        ln_rad_to_dms(PolarAngle, &EquatorialCoordinates.dec);
    };

    /*! \brief Calculates a telescope direction vector from the supplied local hour angle and declination.
     * \param[in] EquatorialCoordinates The local hour angle and declination in decimal degrees
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the direction vector with the hour angle being in the XY plane.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromLocalHourAngleDeclination(struct ln_equ_posn EquatorialCoordinates)
    {
        return TelescopeDirectionVectorFromSphericalCoordinate(ln_deg_to_rad(EquatorialCoordinates.ra), CLOCKWISE, ln_deg_to_rad(EquatorialCoordinates.dec), FROM_AZIMUTHAL_PLANE);
    };

    /*! \brief Calculates a telescope direction vector from the supplied local hour angle and declination.
     * \param[in] EquatorialCoordinates The local hour angle and declination in hours minutes seconds and degrees minutes seconds
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the direction vector with the hour angle being in the XY plane.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromLocalHourAngleDeclination(struct lnh_equ_posn EquatorialCoordinates)
    {
        return TelescopeDirectionVectorFromSphericalCoordinate(ln_hms_to_rad(&EquatorialCoordinates.ra), CLOCKWISE, ln_dms_to_rad(&EquatorialCoordinates.dec), FROM_AZIMUTHAL_PLANE);
    };

    /*! \brief Calculates a local hour angle and declination from the supplied telescope direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] EquatorialCoordinates The local hour angle and declination in decimal degrees
     * \note This assumes a right handed coordinate system for the direction vector with the hour angle being in the XY plane.
     */
    static void LocalHourAngleDeclinationFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                struct ln_equ_posn& EquatorialCoordinates)
    {
        double AzimuthAngle;
        double PolarAngle;
        SphericalCoordinateFromTelescopeDirectionVector(TelescopeDirectionVector, AzimuthAngle, CLOCKWISE, PolarAngle, FROM_AZIMUTHAL_PLANE);
        EquatorialCoordinates.ra = ln_rad_to_deg(AzimuthAngle);
        EquatorialCoordinates.dec = ln_rad_to_deg(PolarAngle);
    };

    /*! \brief Calculates a local hour angle and declination from the supplied telescope direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] EquatorialCoordinates The local hour angle and declination in hours minutes seconds and degrees minutes seconds
     * \note This assumes a right handed coordinate system for the direction vector with the hour angle being in the XY plane.
     */
    static void LocalHourAngleDeclinationFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                struct lnh_equ_posn& EquatorialCoordinates)
    {
        double AzimuthAngle;
        double PolarAngle;
        SphericalCoordinateFromTelescopeDirectionVector(TelescopeDirectionVector, AzimuthAngle, CLOCKWISE, PolarAngle, FROM_AZIMUTHAL_PLANE);
        ln_rad_to_hms(AzimuthAngle, &EquatorialCoordinates.ra);
        ln_rad_to_dms(PolarAngle, &EquatorialCoordinates.dec);
    };

    /*! \brief Calculates a normalised direction vector from the supplied altitude and azimuth.
     * \param[in] HorizontalCoordinates Altitude and Azimuth in decimal degrees
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the telescope direction vector with XY being the azimuthal plane,
     * and azimuth being measured in a clockwise direction.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromAltitudeAzimuth(ln_hrz_posn HorizontalCoordinates)
    {
        return TelescopeDirectionVectorFromSphericalCoordinate(ln_deg_to_rad(HorizontalCoordinates.az), CLOCKWISE, ln_deg_to_rad(HorizontalCoordinates.alt), FROM_AZIMUTHAL_PLANE);
    };

    /*! \brief Calculates a normalised direction vector from the supplied altitude and azimuth.
     * \param[in] HorizontalCoordinates Altitude and Azimuth in degrees minutes seconds
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the telescope direction vector with XY being the azimuthal plane,
     * and azimuth being measured in a clockwise direction.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromAltitudeAzimuth(lnh_hrz_posn HorizontalCoordinates)
    {
        return TelescopeDirectionVectorFromSphericalCoordinate(ln_dms_to_rad(&HorizontalCoordinates.az), CLOCKWISE, ln_dms_to_rad(&HorizontalCoordinates.alt), FROM_AZIMUTHAL_PLANE);
    };

    /*! \brief Calculates an altitude and azimuth from the supplied normalised direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] HorizontalCoordinates Altitude and Azimuth in decimal degrees
     * \note This assumes a right handed coordinate system for the telescope direction vector with XY being the azimuthal plane,
     * and azimuth being measured in a clockwise direction.
     */
    static void AltitudeAzimuthFromNormalisedDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector, ln_hrz_posn& HorizontalCoordinates)
    {
        double AzimuthAngle;
        double AltitudeAngle;
        SphericalCoordinateFromTelescopeDirectionVector(TelescopeDirectionVector, AzimuthAngle, CLOCKWISE, AltitudeAngle, FROM_AZIMUTHAL_PLANE);
        HorizontalCoordinates.az = ln_rad_to_deg(AzimuthAngle);
        HorizontalCoordinates.alt = ln_rad_to_deg(AltitudeAngle);
    };

    /*! \brief Calculates an altitude and azimuth from the supplied normalised direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] HorizontalCoordinates Altitude and Azimuth in degrees minutes seconds
     * \note This assumes a right handed coordinate system for the telescope direction vector with XY being the azimuthal plane,
     * and azimuth being measured in a clockwise direction.
     */
    static void AltitudeAzimuthFromNormalisedDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector, lnh_hrz_posn& HorizontalCoordinates)
    {
        double AzimuthAngle;
        double AltitudeAngle;
        SphericalCoordinateFromTelescopeDirectionVector(TelescopeDirectionVector, AzimuthAngle, CLOCKWISE, AltitudeAngle, FROM_AZIMUTHAL_PLANE);
        ln_rad_to_dms(AzimuthAngle, &HorizontalCoordinates.az);
        ln_rad_to_dms(AltitudeAngle, &HorizontalCoordinates.alt);
    };

    ///@}

    /*! @name Database helper functions
     * An entry in the sync point database is defined by the following INDI properties
     * - ALIGNMENT_POINT_ENTRY_OBSERVATION_JULIAN_DATE\n
     *   The Julian date of the sync point observation (number)
     * - ALIGNMENT_POINT_ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME\n
     *   The local sidereal time of the sync point observation (number)
     * - ALIGNMENT_POINT_ENTRY_RA\n
     *   The right ascension of the sync point (number)
     * - ALIGNMENT_POINT_ENTRY_DEC\n
     *   The declination of the sync point (number)
     * - ALIGNMENT_POINT_ENTRY_VECTOR_X\n
     *   The x component of the telescope direction vector of the sync point (number)
     * - ALIGNMENT_POINT_ENTRY_VECTOR_Y\n
     *   The y component of the telescope direction vector of the sync point (number)
     * - ALIGNMENT_POINT_ENTRY_VECTOR_Z\n
     *   The z component of the telescope direction vector of the sync point (number)
     * - ALIGNMENT_POINT_ENTRY_PRIVATE\n
     *   An optional binary blob for communication between the client and the math plugin
     * .
     * The database is accessed using the following properties
     * - ALIGNMENT_POINTSET_SIZE\n
     *   The count of the number of sync points in the set (number)
     * - ALIGNMENT_POINTSET_CURRENT_ENTRY\n
     *   A zero based number that sets/shows the current entry (number)
     *   Only valid if ALIGNMENT_POINTSET_SIZE is greater than zero
     * - ALIGNMENT_POINTSET_ACTION\n
     *   Whenever this switch is written to one of the following actions is taken
     *   - APPEND\n
     *     Append a new entry to the set.
     *   - INSERT\n
     *     Insert a new entry at the pointer.
     *   - EDIT\n
     *     Overwrites the entry at the pointer.
     *   - DELETE\n
     *     Delete the entry at the pointer.
     *   - CLEAR\n
     *     Delete all entries.
     *   - READ\n
     *     Read the entry at the pointer.
     *   - READ INCREMENT\n
     *     Increment the pointer after reading the entry.
     *   - LOAD DATABASE\n
     *     Load the databse from local storage.
     *   - SAVE DATABASE\n
     *     Save the database to local storage.
     *
     *  The following pure virtual functions must be implemented in derived classes
     */

    ///@{

    enum AlignmentDatabaseActions { APPEND, INSERT, EDIT, DELETE, CLEAR, READ, READ_INCREMENT, LOAD_DATABASE, SAVE_DATABASE };


    /** \brief Apend a sync point to the database.
        \param[in] ObservationDate The Julian date the observation was made.
        \param[in] ObservationTime The local sidereal time the observation was made.
        \param[in] RightAscension Right Ascension (Decimal Hours).
        \param[in] Declination Declination of the observed object (Decimal Degrees).
        \param[in] TelescopeDirectionVector - The direction vector returned from one of the alignment subsystem helper functions
        \return True if successful
    */
    virtual bool AppendSyncPoint(const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector) = 0;

    /** \brief Insert a sync point in the database.
        \param[in] Offset Pointer to where to make then insertion.
        \param[in] ObservationDate The Julian date the observation was made.
        \param[in] ObservationTime The local sidereal time the observation was made.
        \param[in] RightAscension Right Ascension (Decimal Hours).
        \param[in] Declination Declination of the observed object (Decimal Degrees).
        \param[in] TelescopeDirectionVector - The direction vector returned from one of the alignment subsystem helper functions
        \return True if successful
    */
    virtual bool InsertSyncPoint(unsigned int Offset, const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector) = 0;

    /** \brief Edit a sync point in the database.
        \param[in] Offset Pointer to where to make then edit.
        \param[in] ObservationDate The Julian date the observation was made.
        \param[in] ObservationTime The local sidereal time the observation was made.
        \param[in] RightAscension Right Ascension (Decimal Hours).
        \param[in] Declination Declination of the observed object (Decimal Degrees).
        \param[in] TelescopeDirectionVector - The direction vector returned from one of the alignment subsystem helper functions
        \return True if successful
    */
    virtual bool EditSyncPoint(unsigned int Offset, const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector) = 0;

    /** \brief Delete a sync point from the database.
        \param[in] Offset Pointer to where to make then insertion
        \return True if successful
    */
    virtual bool DeleteSyncPoint(unsigned int Offset) = 0;

    /** \brief Delete all sync points from the database.
        \return True if successful
    */
    virtual bool ClearSyncPoints() = 0;

    /** \brief Read a sync point from the database.
        \param[in] Offset Pointer to where to read from.
        \param[out] ObservationDate The Julian date the observation was made.
        \param[out] ObservationTime The local sidereal time the observation was made.
        \param[out] RightAscension Right Ascension (Decimal Hours).
        \param[out] Declination Declination of the observed object (Decimal Degrees).
        \param[out] TelescopeDirectionVector - The direction vector returned from one of the alignment subsystem helper functions
        \return True if successful
    */
    virtual bool ReadSyncPoint(unsigned int Offset, double& ObservationDate, double& ObservationTime, double& RightAscension, double& Declination,
                        TelescopeDirectionVector& TelescopeDirectionVector) = 0;

    /** \brief Read the next sync point from the database.
        \param[out] ObservationDate The Julian date the observation was made.
        \param[out] ObservationTime The local sidereal time the observation was made.
        \param[out] RightAscension Right Ascension (Decimal Hours).
        \param[out] Declination Declination of the observed object (Decimal Degrees).
        \param[out] TelescopeDirectionVector - The direction vector returned from one of the alignment subsystem helper functions
        \return True if successful
    */
    virtual bool ReadNextSyncPoint(double& ObservationDate, double& ObservationTime, double& RightAscension, double& Declination,
                        TelescopeDirectionVector& TelescopeDirectionVector) = 0;
///@}

protected:
};

/*!
 * \class INDI::AlignmentSubsystemClient
 * \brief Provides alignment subsystem functions to INDI clients
 *
 */
class INDI::AlignmentSubsystemClient : public INDI::AlignmentSubsystemBase
{
public:
    /*! @name Database helper function implementations
     */

    ///@{

    bool AppendSyncPoint(const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool InsertSyncPoint(unsigned int Offset, const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool EditSyncPoint(unsigned int Offset, const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool DeleteSyncPoint(unsigned int Offset);

    bool ClearSyncPoints();

    bool ReadSyncPoint(unsigned int Offset, double& ObservationDate, double& ObservationTime, double& RightAscension, double& Declination,
                        TelescopeDirectionVector& TelescopeDirectionVector);

    bool ReadNextSyncPoint(double& ObservationDate, double& ObservationTime, double& RightAscension, double& Declination,
                        TelescopeDirectionVector& TelescopeDirectionVector);
    ///@}
};

/*!
 * \class INDI::AlignmentSubsystemDriver
 * \brief Provides alignment subsystem functions to INDI telescope drivers
 *
 */
class INDI::AlignmentSubsystemDriver : public INDI::AlignmentSubsystemBase
{
public:
    AlignmentSubsystemDriver() : CurrentMathPlugin(NULL) {}
    /*! @name Plugin management
     *  These functions are used to enumerate, load, and utilise math plugins.
     *  They are intended to be used solely in driver modules.
     *  The following INDI properties are used to communicate the plugin details to the client if required.
     *  - ALIGNMENT_SUBSYSTEM_MATH_PLUGINS\n
     *  A list of available plugins (switch). This also indicates the currently
     *  selected plugin.
     *  - ALIGNMENT_SUBSYSTEM_CURRENT_MATH_PLUGIN\n
     *  A read only persistent property giving the name of the currently selected plugin.
     */
    ///@{
    /*!
     * \brief Return a list of the names of the available math plugins.
     * \param[out] MathPlugins Reference to a list of the names of the available math plugins.
     * \return False on failure
     */
    bool EnumerateMathPlugins(std::vector<std::string>& MathPlugins);

    /*!
     * \brief Selects, loads and initialises the named math plugin.
     * \param[in] MathPluginName The name of the required math plugin.
     * \return False on failure.
     */
    bool SelectMathPlugin(const std::string& MathPluginName);

    /*!
     * \brief Get a pointer to the current math plugin.
     * \return NULL on failure.
     */
    const AlignmentSubsystemMathPlugin* GetMathPluginPointer(void);
    ///@}

    /** \brief Initilize alignment properties. It is recommended to call this function within initProperties() of your primary device
        \param[in] deviceName Name of the primary device
    */
    void InitAlignmentProperties(Telescope* pTelescope);

    /** \brief Call this function whenever a client updates a text property. The function will
     * handle any alignment related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessAlignmentTextProperties(Telescope* pTelescope, const char *name, char *texts[], char *names[], int n);

    /** \brief Call this function whenever a client updates a number property. The function will
     * handle any alignment related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessAlignmentNumberProperties(Telescope* pTelescope, const char *name, double values[], char *names[], int n);

    /** \brief Call this function whenever a client updates a switch property. The function will
     * handle any alignment related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessAlignmentSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n);

    /** \brief Call this function whenever a client updates a blob property. The function will
     * handle any alignment related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessAlignmentBlobProperties(Telescope* pTelescope, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n);

    /** \brief Call this function to save persistent alignment subsytem properties.
     * This function should be called from within the saveConfigItems function of your driver.
     * \param[in] fp File pointer passed into saveConfigItems
    */
    void SaveAlignmentConfigProperties(FILE *fp);

    /*! @name Database helper function implementations
     */

    ///@{

    bool AppendSyncPoint(const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool InsertSyncPoint(unsigned int Offset, const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool EditSyncPoint(unsigned int Offset, const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool DeleteSyncPoint(unsigned int Offset);

    bool ClearSyncPoints();

    bool ReadSyncPoint(unsigned int Offset, double& ObservationDate, double& ObservationTime, double& RightAscension, double& Declination,
                        TelescopeDirectionVector& TelescopeDirectionVector);

    bool ReadNextSyncPoint(double& ObservationDate, double& ObservationTime, double& RightAscension, double& Declination,
                        TelescopeDirectionVector& TelescopeDirectionVector);
    ///@}

protected:

    // Property values
    std::auto_ptr<ISwitch> AlignmentSubsystemMathPlugins;
    ISwitchVectorProperty AlignmentSubsystemMathPluginsV;
    enum AlignmentPointSetEnum {ENTRY_OBSERVATION_JULIAN_DATE, ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME,
                                ENTRY_RA, ENTRY_DEC, ENTRY_VECTOR_X, ENTRY_VECTOR_Y, ENTRY_VECTOR_Z};
    INumber AlignmentPointSetEntry[7];
    INumberVectorProperty AlignmentPointSetEntryV;
    IBLOB AlignmentPointSetPrivateBinaryData;
    IBLOBVectorProperty AlignmentPointSetPrivateBinaryDataV;
    INumber AlignmentPointSetSize;
    INumberVectorProperty AlignmentPointSetSizeV;
    INumber AlignmentPointSetPointer;
    INumberVectorProperty AlignmentPointSetPointerV;
    ISwitch AlignmentPointSetAction[9];
    ISwitchVectorProperty AlignmentPointSetActionV;
    ISwitch AlignmentPointSetCommit;
    ISwitchVectorProperty AlignmentPointSetCommitV;
    AlignmentSubsystemMathPlugin* CurrentMathPlugin;

    // The following property is used for configuration purposes only and is not propagated to the client
    IText AlignmentSubsystemCurrentMathPlugin;
    ITextVectorProperty AlignmentSubsystemCurrentMathPluginV;

private:
    // Functions
    bool LoadDatabase(const char* DeviceName);
    bool SaveDatabase(const char* DeviceName);

    // Data
    struct DatabaseEntry
    {
        double ObservationDate;
        double ObservationTime;
        double RightAscension;
        double Declination;
        TelescopeDirectionVector TelescopeDirection;
        std::auto_ptr<void> PrivateData;
    };
    typedef std::vector<DatabaseEntry> SyncPointsType;
    SyncPointsType SyncPoints;
};

/*!
 * \class INDI::AlignmentSubsystemMathPlugin
 * \brief Provides alignment subsystem functions to INDI alignment math plugins
 *
 * \note This class is intended to be implemented within a dynamic shared object. If the
 * implementation of this class uses a standard 3 by 3 transformation matrix to convert between coordinate systems
 * then it will not normally need to know the handedness of either the celestial or telescope coordinate systems, as the
 * necessary rotations and scaling will be handled in the derivation of the matrix coefficients. This will normally
 * be done using the three reference (sync) points method. Knowledge of the handedness of the coordinate systems is needed
 * when only two reference points are available and a third reference point has to artificially generated in order to
 * derive the matrix coefficients.
 */
class INDI::AlignmentSubsystemMathPlugin : public INDI::AlignmentSubsystemBase
{
public:
    /** \brief Get the alignment corrected telescope pointing direction for the supplied celestial coordinates
        \param[in] RightAscension Right Ascension (Decimal Hours).
        \param[in] Declination Declination (Decimal Degrees).
        \param[out] TelescopeDirectionVector Parameter to receive the corrected telescope direction
        \return True if successful
    */
    virtual bool TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector) = 0;

    /** \brief Get the true celestial coordinates for the supplied telescope pointing direction
        \param[in] TelescopeDirectionVector the telescope direction
        \param[out] RightAscension Parameter to receive the Right Ascension (Decimal Hours).
        \param[out] Declination Parameter to receive the Declination (Decimal Degrees).
        \return True if successful
    */
    virtual bool TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination) = 0;

    /*! @name Database helper function implementations
     */

    ///@{

    bool AppendSyncPoint(const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool InsertSyncPoint(unsigned int Offset, const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool EditSyncPoint(unsigned int Offset, const double ObservationDate, const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    bool DeleteSyncPoint(unsigned int Offset);

    bool ClearSyncPoints();

    bool ReadSyncPoint(unsigned int Offset, double& ObservationDate, double& ObservationTime, double& RightAscension, double& Declination,
                        TelescopeDirectionVector& TelescopeDirectionVector);

    bool ReadNextSyncPoint(double& ObservationDate, double& ObservationTime, double& RightAscension, double& Declination,
                        TelescopeDirectionVector& TelescopeDirectionVector);
    ///@}
};

#endif // ALIGNMENTSUBSYSTEM_H
