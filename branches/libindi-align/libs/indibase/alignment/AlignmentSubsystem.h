/*!
 * \file AlignmentSubsystem.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file contains a straw man for adding scope alignment functions to the INDI telescope class.
 */
#ifndef ALIGNMENTSUBSYSTEM_H
#define ALIGNMENTSUBSYSTEM_H

#include <vector>
#include <string>

// Include libnova types N.B. This prefix should be stripped when this is included in a cmake build
#include <libnova/utility.h>

/*!
 * \class TelescopeDirectionVector
 * \brief Holds a nomalised direction vector (direction cosines)
 *
 * The x y,z fields of this class should always represent a normalised (unit length)
 * vector in a right handed rectangular coordinate space.
 */
class TelescopeDirectionVector
{
public:
    double x;
    double y;
    double z;
};

/*!
 * \class MathPlugin
 * \brief Provides functions for transforming celestial coordinates into telescope coordinates.
 *
 *
 * \note This class is intended to be implemented as a dynamic shared object. If the
 * implementation of this class uses a standard 3 by 3 transformation matrix to convert between coordinate systems
 * then it will not normally need to know the handedness of either the celestial or telescope coordinate systems, as the
 * necessary rotations and scaling will be handled in the derivation of the matrix coefficients. This will normally
 * be done using the three reference (sync) points method. Knowledge of the handedness of the coordinate systems is needed
 * when only two reference points are available and a third reference point has to artificially generated in order to
 * derive the matrix coefficients.
 */
class MathPlugin
{
public:
    /** \brief Get the alignment corrected telescope pointing direction for the supplied celestial coordinates
        \param RightAscension Right Ascension (Decimal Hours).
        \param Declination Declination (Decimal Degrees).
        \param TelescopeDirectionVector Parameter to receive the corrected telescope direction
        \return True if successful
    */
    virtual bool TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector) = 0;

    /** \brief Get the true celestial coordinates for the supplied telescope pointing direction
        \param TelescopeDirectionVector the telescope direction
        \param RightAscension Parameter to receive the Right Ascension (Decimal Hours).
        \param Declination Parameter to receive the Declination (Decimal Degrees).
        \return True if successful
    */
    virtual bool TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination) = 0;
};

/*!
 * \class AlignmentSubsystem
 * \brief Provides functions to manage the loading and initialisation of Alignment Subsystem plugin database and math modules. Also provides
 * helper functions for telescope drivers using the the Alignment Subsystem.
 *
 * This is the long descriptipon.
 *
 * \note It is intended that this class should be included in the INDI::Telescope object
 * in some way, either as a direct class member of by multiple inheritance.
 */
class AlignmentSubsystem
{
public:
    /*! @name Plugin Management
     *  These functions are used to enumerate, load, and utilise math plugins.
     *  They are intended to be used solely in driver modules and therefore are
     *  non static.
     */
    ///@{

    /*!
     * \brief Return a list of the names of the available math plugins.
     * \param[out] MathPlugins Reference to a list of the names of the available math plugins.
     * \return False on failure
     */
    bool EnumerateMathPlugins(const std::vector<std::string>& MathPlugins);

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
    const MathPlugin* GetMathPluginPointer(void);

    ///@}

    /*! @name TelescopeDirectionVector Helper Functions
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
        return TelescopeDirectionVectorFromSphericalCoordinate(ln_deg_to_rad(EquatorialCoordinates.ra), CLOCKWISE, ln_deg_to_rad(EquatorialCoordinates.dec), FROM_AZIMUTHAL_PLANE);
    };

    /*! \brief Calculates a telescope direction vector from the supplied equatorial coordinates.
     * \param[in] EquatorialCoordinates The equatorial cordinates in hour minutes seconds and degrees minutes seconds
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the direction vector with the right ascension being in the XY plane.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromEquatorialCoordinates(struct lnh_equ_posn EquatorialCoordinates)
    {
        return TelescopeDirectionVectorFromSphericalCoordinate(ln_hms_to_rad(&EquatorialCoordinates.ra), CLOCKWISE, ln_dms_to_rad(&EquatorialCoordinates.dec), FROM_AZIMUTHAL_PLANE);
    };

    /*! \brief Calculates equatorial coordinates from the supplied telescope direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] EquatorialCoordinates The equatorial cordinates in decimal degrees
     * \note This assumes a right handed coordinate system for the direction vector with the right ascension being in the XY plane.
     */
    static void EquatorialCoordinatesFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                struct ln_equ_posn& EquatorialCoordinates)
    {
        double AzimuthAngle;
        double PolarAngle;
        SphericalCoordinateFromTelescopeDirectionVector(TelescopeDirectionVector, AzimuthAngle, CLOCKWISE, PolarAngle, FROM_AZIMUTHAL_PLANE);
        EquatorialCoordinates.ra = ln_rad_to_deg(AzimuthAngle);
        EquatorialCoordinates.dec = ln_rad_to_deg(PolarAngle);
    };

    /*! \brief Calculates equatorial coordinates from the supplied telescope direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] EquatorialCoordinates The equatorial cordinates in hour minutes seconds and degrees minutes seconds
     * \note This assumes a right handed coordinate system for the direction vector with the right ascension being in the XY plane.
     */
    static void EquatorialCoordinatesFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                struct lnh_equ_posn& EquatorialCoordinates)
    {
        double AzimuthAngle;
        double PolarAngle;
        SphericalCoordinateFromTelescopeDirectionVector(TelescopeDirectionVector, AzimuthAngle, CLOCKWISE, PolarAngle, FROM_AZIMUTHAL_PLANE);
        ln_rad_to_hms(AzimuthAngle, &EquatorialCoordinates.ra);
        ln_rad_to_dms(PolarAngle, &EquatorialCoordinates.dec);
    };

    /*! \brief Calculates a telescope direction vector from the supplied local hour angle
     * and declination.
     * \param[in] LocalHourAngle
     * \param[in] Declination
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the direction vector with the hour angle being in the XY plane.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromLocalHourAngleDeclination(const double LocalHourAngle, const double Declination);

    /*! \brief Calculates a local hour angle and declination from the supplied telescope direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] LocalHourAngle
     * \param[out] Declination
     * \note This assumes a right handed coordinate system for the direction vector with the hour angle being in the XY plane.
     */
    static void LocalHourAngleDeclinationFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                double LocalHourAngle,
                                                                double Declination);

    /*! \brief Calculates a normalised direction vector from the supplied altitude
     * and azimuth.
     * \param[in] Altitude
     * \param[in] Azimuth
     * \return A TelescopeDirectionVector
     * \note This assumes a right handed coordinate syste for the telescope direction vector with XY being the azimuthal plane,
     * and azimuth being measured in a clockwise direction.
     */
    static const TelescopeDirectionVector& TelescopeDirectionVectorFromAltitudeAzimuth(const double Altitude, const double Azimuth);

    /*! \brief Calculates an altitude and azimuth from the supplied normalised direction vector
     * and declination.
     * \param[in] TelescopeDirectionVector
     * \param[out] Altitude
     * \param[out] Azimuth
     * \note This assumes a right handed coordinate system for the telescope direction vector with XY being the azimuthal plane,
     * and azimuth being measured in a clockwise direction.
     */
    static void AltitudeAzimuthFromNormalisedDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                double Altitude,
                                                                double Azimuth);

    ///@}

    /*! @name Database Helper Functions
     */
    ///@{

    /** \brief Add a sync point to the database.
        \param ObservationTime The time the observation was made.
        \param RightAscension Right Ascension (Decimal Hours).
        \param Declination Declination of the observed object (Decimal Degrees).
        \param TelescopeDirectionVector - The direction vector returned from one of the alignment subsystem helper functions
        \return True if successful
        \note This is just here as a placeholder at the moment
    */
    bool AddSyncPoint(const double ObservationTime, const double RightAscension, const double Declination,
                        const TelescopeDirectionVector& TelescopeDirectionVector);

    ///@}
};

#endif // ALIGNMENTSUBSYSTEM_H
