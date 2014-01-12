#include "BuiltInMathPlugin.h"

#include <gsl/gsl_matrix.h>

namespace INDI {
namespace AlignmentSubsystem {

BuiltInMathPlugin::BuiltInMathPlugin()
{
    //ctor
}

BuiltInMathPlugin::~BuiltInMathPlugin()
{
    //dtor
}

bool BuiltInMathPlugin::Initialise()
{
    // See how many entries there are in the in memory database.
    // If just one compute local horizontal coordinate offsets.
    // If two compute a dummy third entry and compute and store
    // a TAKI transform matrix. If three compute a TAKI transform matrix.
    // If four or more compute a convex hull, then TAKI matrices for each
    // triangular facet of the hull.
    switch (GetAlignmentDatabase().size())
    {
        case 0:
            return false;

        case 1:
        {
            // Compute local horizontal coordinate offsets
            AlignmentDatabaseEntry& Entry = GetAlignmentDatabase()[0];
            ln_equ_posn RaDec;
            ln_lnlat_posn Position;
            if (!GetDatabaseReferencePosition(Position))
                return false;
            RaDec.dec = Entry.Declination;
            RaDec.ra = Entry.RightAscension;
            switch (ApproximateMountAlignment)
            {
                case ZENITH:
                    ln_hrz_posn AltAz;
                    ln_hrz_posn MountAltAz;
                   // Probably an altaz goto or a dobsonian
                    // Translate sync point RA/DEC into alt/az (clockwise from South) for the
                    // date and time of the sync point observation.
                    ln_get_hrz_from_equ(&RaDec, &Position, Entry.ObservationJulianDate, &AltAz);
                    // Translate the mount coordinates from the sync point into similar clockwise coordinate space
                    AltitudeAzimuthFromNormalisedDirectionVector(Entry.TelescopeDirection, MountAltAz);
                    SinglePointOffsetsAltAz.alt = AltAz.alt - MountAltAz.alt;
                    SinglePointOffsetsAltAz.az = AltAz.az - MountAltAz.az;
                    break;

                case NORTH_CELESTIAL_POLE:
                    // Equatorial in the northern hemisphere
                    ln_equ_posn AdjustedEquatorialCoordinate;
                    ln_equ_posn MountEquatorialCoordinate;
                    double GreenwichMeanSiderealTime = ln_get_mean_sidereal_time(Entry.ObservationJulianDate);
                    // Convert time to degrees
                    GreenwichMeanSiderealTime *= 360.0 / 24.0;
                    // Convert right ascension to local hour angle
                    // LHA = GMST + Longitude (positive east) - RA
                    AdjustedEquatorialCoordinate.ra = GreenwichMeanSiderealTime + Position.lng - RaDec.ra;
                    AdjustedEquatorialCoordinate.dec = RaDec.ra;
                    // N.B. this has converted the ra value from ANTI_CLOCKWISE to CLOCKWISE
                    LocalHourAngleDeclinationFromTelescopeDirectionVector(Entry.TelescopeDirection, MountEquatorialCoordinate);
                    SinglePointsOffsetsRaDec.ra =  AdjustedEquatorialCoordinate.ra - MountEquatorialCoordinate.ra;
                    SinglePointsOffsetsRaDec.dec =  AdjustedEquatorialCoordinate.dec - MountEquatorialCoordinate.dec;
                    break;
            }
            return true;
        }

        case 2:
            // Compute dummy third entry
            // Compute transform
            return true;

        case 3:
            // Compute transform
            return true;

        default:
            // Compute Hull etc.
            return true;
    }
}

bool BuiltInMathPlugin::TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector)
{
    return false;
}

bool BuiltInMathPlugin::TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination)
{
    return false;
}

} // namespace AlignmentSubsystem
} // namespace INDI
