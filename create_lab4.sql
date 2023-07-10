-- CSE 180 Winter 2023 Lab4 create, which has two additional attributes in Owners, and a view

-- The following two lines are not needed, but they're useful.
DROP SCHEMA Lab4 CASCADE;
CREATE SCHEMA Lab4;


CREATE TABLE Highways (
    highwayNum INT,
    length NUMERIC(4,1),
    speedLimit INT,
    PRIMARY KEY (highwayNum)
);

CREATE TABLE Exits (
    highwayNum INT REFERENCES Highways,
    exitNum INT,
    description VARCHAR(60),
    mileMarker NUMERIC(4,1),
    exitCity VARCHAR(20),
    exitState CHAR(2),
    isExitOpen BOOL,
    PRIMARY KEY (highwayNum, exitNum)
);

CREATE TABLE Interchanges (
    highwayNum1 INT,
    exitNum1 INT,
    highwayNum2 INT,
    exitNum2 INT,
    FOREIGN KEY (highwayNum1, exitNum1) REFERENCES Exits(highwayNum, exitNum),
    FOREIGN KEY (highwayNum2, exitNum2) REFERENCES Exits(highwayNum, exitNum),
    PRIMARY KEY (highwayNum1, exitNum1, highwayNum2, exitNum2)
);

CREATE TABLE Cameras (
    cameraID INT,
    highwayNum INT REFERENCES Highways(highwayNum),
    mileMarker NUMERIC(4,1),
    isCameraWorking BOOL,
    PRIMARY KEY (cameraID)
);

CREATE TABLE Owners (
    ownerState CHAR(2),
    ownerLicenseID CHAR(8),
    name VARCHAR(60),
    address VARCHAR(60),
    startDate DATE,
    expirationDate DATE,
    speedingViolations INTEGER,
    fine INTEGER,
    PRIMARY KEY (ownerState, ownerLicenseID)
);

CREATE TABLE Vehicles (
    vehicleState CHAR(2),
    vehicleLicensePlate CHAR(7),
    ownerState CHAR(2),
    ownerLicenseID CHAR(8),
    year INT,
    color CHAR(2),
    FOREIGN KEY (ownerState, ownerLicenseID) REFERENCES Owners,
    PRIMARY KEY (vehicleState, vehicleLicensePlate)
);

CREATE TABLE Photos (
    cameraID INT REFERENCES Cameras,
    vehicleState CHAR(2),
    vehicleLicensePlate Char(7),
    photoTimestamp TIMESTAMP,
    FOREIGN KEY (vehicleState, vehicleLicensePlate) REFERENCES Vehicles,
    PRIMARY KEY (cameraID, photoTimestamp)
);


CREATE VIEW DistancesAndIntervalsForPhotos AS
   SELECT c1.highwayNum, p1.vehicleState, p1.vehicleLicensePlate,
          ABS(c2.mileMarker - c1.mileMarker) AS distBetweenCameraMileMarkers,
    ( EXTRACT(EPOCH FROM p2.photoTimestamp) 
      - EXTRACT(EPOCH FROM p1.photoTimestamp) ) /3600 AS photoIntervalInHours
   FROM Photos p1, Cameras c1, Photos p2, Cameras c2 
   WHERE c1.highwayNum = c2.highwayNum
     AND p1.cameraID = c1.cameraID
     AND p2.cameraID = c2.cameraID 
     AND p1.vehicleState = p2.vehicleState 
     AND p1.vehicleLicensePlate = p2.vehicleLicensePlate
     AND p2.photoTimestamp > p1.photoTimestamp;