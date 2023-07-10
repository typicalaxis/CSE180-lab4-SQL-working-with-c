CREATE OR REPLACE FUNCTION
determineSpeedingViolationsAndFines(maxFineTotal INTEGER)
RETURNS INTEGER AS $$


    DECLARE
    	fineTotal		INTEGER;	  /* Number actually fined, the value returned */
    	theOwnerLicenseID	CHAR(8);  /* owner license */
        theOwnerState	CHAR(2);  /* owner state */
        theSpeedingViolations INTEGER; /*number of speeding violations per owner */
        theFine INTEGER; /*value of fine per owner */

    DECLARE speedingCursor CURSOR FOR
    	    SELECT o.ownerLicenseID, o.ownerState, COUNT(*)
            FROM DistancesAndIntervalsForPhotos d, Owners o, Vehicles v, Highways h
            WHERE h.highwayNum = d.highwayNum
                AND v.vehicleState = d.vehicleState
                AND v.vehicleLicensePlate = d.vehicleLicensePlate
                AND h.speedLimit < (distBetweenCameraMileMarkers/photoIntervalInHours)
                AND o.ownerState = v.ownerState
                AND o.ownerLicenseID = v.ownerLicenseID
            GROUP BY o.ownerLicenseID, o.ownerState;

    DECLARE fineCursor CURSOR FOR
    	    SELECT o.fine
            FROM Owners o
            ORDER BY o.fine DESC;

    BEGIN

	-- Input Validation
	IF maxFineTotal <= 0 THEN
	    RETURN -1;		/* Illegal value of maxFineTotal */
	    END IF;

        fineTotal := 0;

        OPEN speedingCursor;

        LOOP
 
            FETCH speedingCursor INTO theOwnerLicenseID, theOwnerState, theSpeedingViolations;

            -- Exit if there are no more records for firingCursor
            EXIT WHEN NOT FOUND;

            IF theSpeedingViolations >= 3 THEN
                UPDATE Owners
                SET fine = theSpeedingViolations * 50, speedingViolations = theSpeedingViolations
                WHERE ownerLicenseID = theOwnerLicenseID 
                    AND ownerState = theOwnerState;
                END IF;
            IF theSpeedingViolations = 2 THEN
                UPDATE Owners
                SET fine = theSpeedingViolations * 20, speedingViolations = theSpeedingViolations
                WHERE ownerLicenseID = theOwnerLicenseID 
                    AND ownerState = theOwnerState;
                END IF;
            IF theSpeedingViolations = 1 THEN
                UPDATE Owners
                SET fine = theSpeedingViolations * 10, speedingViolations = theSpeedingViolations
                WHERE ownerLicenseID = theOwnerLicenseID 
                    AND ownerState = theOwnerState;
                END IF;
            IF theSpeedingViolations = 0 THEN
                UPDATE Owners
                SET fine = 0, speedingViolations = theSpeedingViolations
                WHERE ownerLicenseID = theOwnerLicenseID 
                    AND ownerState = theOwnerState;
                END IF;

        END LOOP;
        CLOSE speedingCursor;

        OPEN fineCursor;

        LOOP
 
            FETCH fineCursor INTO theFine;

            -- Exit if there are no more records for firingCursor,
            -- or when fineTotal = maxFineTotal, as no more can be added.
            EXIT WHEN NOT FOUND OR fineTotal = maxFineTotal;

            IF (fineTotal + theFine) <= maxFineTotal THEN
                fineTotal := fineTotal + theFine;
                END IF;

        END LOOP;
        CLOSE fineCursor;

	RETURN fineTotal;

    END

$$ LANGUAGE plpgsql;
