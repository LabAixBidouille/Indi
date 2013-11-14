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

/*!
 * \class CTelescopeDirectionVector
 * \brief Holds a nomalised direction vector (direction cosines)
 *
 *
 */
class CTelescopeDirectionVector;

/*!
 * \class CDatabasePlugin
 * \brief Provides functions for managing a table of sync points.
 *
 * \note This class is intended to be implemented as a dynamic shared object.
 *
 */
class CDatabasePlugin
{
public:
    /** \brief Add a sync point to the database.
        \param ReferenceTime
        \param LocalHourAngle Local hour angle of the observed object (Decimal Hours).
        \param Declination Declination of the observed object (Decimal Degrees).
        \param TelescopeDirectionVector - The direction vector returned from one of the alignment subsystem helper functions
        \return True if successful
    */
    bool AddSyncPoint(const double ReferenceTime, const double LocalHourAngle, const double Declination,
                        const CTelescopeDirectionVector& TelescopeDirectionVector);
};


/*!
 * \class CMathPlugin
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
class CMathPlugin
{
public:
    /** \brief Get the alignment corrected telescope pointing direction for the supplied celestial coordinates
        \param RightAscension Right Ascension (Decimal Hours).
        \param Declination Declination (Decimal Degrees).
        \param TelescopeDirectionVector Parameter to receive the corrected telescope direction
        \return True if successful
    */
    bool TransformCelestialToTelescope(const double RightAscension, const double Declination, CTelescopeDirectionVector& TelescopeDirectionVector);

    /** \brief Get the true celestial coordinates for the supplied telescope pointing direction
        \param TelescopeDirectionVector the telescope direction
        \param RightAscension Parameter to receive the Right Ascension (Decimal Hours).
        \param Declination Parameter to receive the Declination (Decimal Degrees).
        \return True if successful
    */
    bool TransformTelescopeToCelestial(const CTelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination);

    /** \brief Set the current database plugin
        \param CurrentDatabasePlugin A pointer to the current database plugin
    */
    void SetCurrentDatabasePlugin(CDatabasePlugin* CurrentDatabasePlugin);
};

/*!
 * \class CAlignmentSubsystem
 * \brief Provides functions to manage the loading and initialisation of Alignment Subsystem plugin database and math modules. Also provides
 * helper functions for telescope drivers using the the Alignment Subsystem.
 *
 * This is the long descriptipon.
 *
 * \note It is intended that this class should be included in the INDI::Telescope object
 * in some way, either as a direct class member of by multiple inheritance.
 */
class CAlignmentSubsystem
{
public:
    /*!
     * \brief Return lists of the names of the available database and math plugins.
     * \param[out] DatabasePlugins Reference to a list of the names of the available database plugins.
     * \param[out] MathPlugins Reference to a list of the names of the available math plugins.
     * \return False on failure
     */
    bool EnumeratePlugins(const std::vector<std::string>& DatabasePlugins , const std::vector<std::string>& MathPlugins);

    /*! \brief Selects, loads and initialises the named database plugin.
     * \param[in] DatabasePluginName The name of the required database plugin.
     * \return False on failure.
     */
    bool SelectDatabasePlugin(const std::string& DatabasePluginName);

    /*!
     * \brief Selects, loads and initialises the named math plugin.
     * \param[in] MathPluginName The name of the required math plugin.
     * \return False on failure.
     */
    bool SelectMathPlugin(const std::string& MathPluginName);

    /*!
     * \brief Get a pointer to the current database plugin.
     * \return NULL on failure.
     */
    const CDatabasePlugin* GetDatabasePluginPointer(void);

    /*!
     * \brief Get a pointer to the current math plugin.
     * \return NULL on failure.
     */
    const CMathPlugin* GetMathPluginPointer(void);

    /*! @name Helper Functions
     *  These functions are used to convert different coordinate systems to and from the
     *  normalised direction vectors (direction cosines) used for telescope coordinates in the
     *  alignment susbsystem.
     */
    ///@{

    /*! \brief Calculates a normalised direction vector from the supplied local hour angle
     * and declination.
     * \param[in] LocalHourAngle
     * \param[in] Declination
     * \return A CTelescopeDirectionVector
     * \note This assumes a right handed coordinate system for the direction vector with the hour angle being in the XY plane.
     */
    const CTelescopeDirectionVector& TelescopeDirectionVectorFromLocalHourAngleDeclination(const double LocalHourAngle, const double Declination);

    /*! \brief Calculates a local hour angle and declination from the supplied normalised direction vector
     * and declination.
     * \param[in] DirectionVectorX
     * \param[in] DirectionVectorY
     * \param[in] DirectionVectorZ
     * \param[out] LocalHourAngle
     * \param[out] Declination
     * \note This assumes a right handed coordinate system for the direction vector with the hour angle being in the XY plane.
     */
    void LocalHourAngleDeclinationFromNormalisedDirectionVector(const double DirectionVectorX,
                                                                const double DirectionVectorY,
                                                                const double DirectionVectorZ,
                                                                double LocalHourAngle,
                                                                double Declination);

    /*! \brief Calculates a normalised direction vector from the supplied altitude
     * and azimuth.
     * \param[in] Altitude
     * \param[in] Azimuth
     * \return A CTelescopeDirectionVector
     * \note This assumes a right handed coordinate syste for the direction vector with XY being the azimuthal plane,
     * and azimuth being measured in a clockwise direction.
     */
    const CTelescopeDirectionVector& NormalisedDirectionVectorFromAltitudeAzimuth(const double Altitude, const double Azimuth);

    /*! \brief Calculates an altitude and azimuth from the supplied normalised direction vector
     * and declination.
     * \param[in] DirectionVectorX
     * \param[in] DirectionVectorY
     * \param[in] DirectionVectorZ
     * \param[out] Altitude
     * \param[out] Azimuth
     * \note This assumes a right handed coordinate syste for the direction vector with XY being the azimuthal plane,
     * and azimuth being measured in a clockwise direction.
     */
    void AltitudeAzimuthFromNormalisedDirectionVector(const double DirectionVectorX,
                                                                const double DirectionVectorY,
                                                                const double DirectionVectorZ,
                                                                double Altitude,
                                                                double Azimuth);

    ///@}
};

#endif // ALIGNMENTSUBSYSTEM_H
