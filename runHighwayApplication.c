/**
 * runHighwayApplication skeleton, to be modified by students
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "libpq-fe.h"

/* These constants would normally be in a header file */
/* Maximum length of string used to submit a connection */
#define MAXCONNECTIONSTRINGSIZE 501
/* Maximum length of string used to submit a SQL statement */
#define MAXSQLSTATEMENTSTRINGSIZE 2001
/* Maximum length of string version of integer; you don't have to use a value this big */
#define  MAXNUMBERSTRINGSIZE        20


/* Exit with success after closing connection to the server
 *  and freeing memory that was used by the PGconn object.
 */
static void good_exit(PGconn *conn)
{
    PQfinish(conn);
    exit(EXIT_SUCCESS);
}

/* Exit with failure after closing connection to the server
 *  and freeing memory that was used by the PGconn object.
 */
static void bad_exit(PGconn *conn)
{
    PQfinish(conn);
    exit(EXIT_FAILURE);
}

/* The three C functions that for Lab4 should appear below.
 * Write those functions, as described in Lab4 Section 4 (and Section 5,
 * which describes the Stored Function used by the third C function).
 *
 * Write the tests of those function in main, as described in Section 6
 * of Lab4.
 *
 * You may use "helper" functions to avoid having to duplicate calls and
 * printing, if you'd like, but if Lab4 says do things in a function, do them
 * in that function, and if Lab4 says do things in main, do them in main,
 * possibly using a helper function, if you'd like.
 */

/* Function: printCameraPhotoCount:
 * -------------------------------------
 * Parameters:  connection, and theCameraID, which should be the ID of a camera.
 * Prints the cameraID, the highwayNum and mileMarker of that camera, and the
 * number of number of photos for that camera, if camera exists for thecameraID.
 * Return 0 if normal execution, -1 if no such camera.
 * bad_exit if SQL statement execution fails.
 */


int printCameraPhotoCount(PGconn *conn, int theCameraID)
{
    PGresult *res = PQexec(conn, "BEGIN TRANSACTION;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "BEGIN failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    char command[128];
    sprintf(command, "SELECT * FROM Photos p WHERE p.cameraID = %d", theCameraID);
    res = PQexec(conn, command);

    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    int numphotos = PQntuples(res);
    
    sprintf(command, "SELECT * FROM Cameras c WHERE c.cameraID = %d", theCameraID);
    res = PQexec(conn, command);

    int numCameras = PQntuples(res);
    if(numCameras == 0){
        res = PQexec(conn, "COMMIT;");
        if (PQresultStatus(res) != PGRES_COMMAND_OK){
            fprintf(stderr, "COMMIT failed: %s", PQerrorMessage(conn));
            PQclear(res);
            bad_exit(conn);
        }
        PQclear(res);
        return(-1);
    }
    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    printf("Camera %d, on %s at %s has taken %d photos.\n",atoi( PQgetvalue(res, 0, 0) ),PQgetvalue(res, 0, 1), PQgetvalue(res, 0, 2), numphotos);

    res = PQexec(conn, "COMMIT;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "COMMIT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    
    PQclear(res);
    return(0);
}

/* Function: openAllExits:
 * ----------------------------
 * Parameters:  connection, and theHighwayNum, the number of a highway.
 
 * Opens all the exit on that highway that weren't already open.
 * Returns the number of exits on the highway that weren't open,
 * or -1 if there is no highway whose highwayNum is theHighwayNum.
 */

int openAllExits(PGconn *conn, int theHighwayNum)
{
    PGresult *res = PQexec(conn, "BEGIN TRANSACTION;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "BEGIN failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    char command[128];
    sprintf(command, "SELECT * FROM Highways h WHERE h.highwayNum = %d", theHighwayNum);
    res = PQexec(conn, command);

    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }

    int numhighways = PQntuples(res);
    if(numhighways == 0){
        res = PQexec(conn, "COMMIT;");
        if (PQresultStatus(res) != PGRES_COMMAND_OK){
            fprintf(stderr, "COMMIT failed: %s", PQerrorMessage(conn));
            PQclear(res);
            bad_exit(conn);
        }
        PQclear(res);
        return(-1);
    }
    sprintf(command, "SELECT DISTINCT * FROM Exits e WHERE e.highwayNum = %d AND NOT e.isExitOpen = TRUE", theHighwayNum);
    res = PQexec(conn, command);

    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    int numExitsClosed = PQntuples(res);

    sprintf(command, "UPDATE Exits SET isExitOpen = TRUE WHERE highwayNum = %d AND NOT isExitOpen = TRUE", theHighwayNum);
    res = PQexec(conn, command);

    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "UPDATE failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }

    res = PQexec(conn, "COMMIT;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "COMMIT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }

    PQclear(res);
    return(numExitsClosed);
}

/* Function: determineSpeedingViolationsAndFines:
 * -------------------------------
 * Parameters:  connection, and an integer maxFineTotal, the maximum total
 * of the fines that should be assessed to owners whose vehicles were speeding.
 *
 * It should count the number of speeding violations by vehicles that each owner
 * owns, and set the speedingViolations field of Owners accordingly.
 *
 * It should also assess fines to some owners based on the number of speeding
 * violations they have.
 *
 * Executes by invoking a Stored Function,
 * determineSpeedingViolationsAndFinesFunction, which does all of the work,
 * as described in Section 5 of Lab4.
 *
 * Returns a negative value if there is an error, and otherwise returns the total
 * fines that were assessed by the Stored Function.
 */

int determineSpeedingViolationsAndFines(PGconn *conn, int maxFineTotal)
{
    PGresult *res = PQexec(conn, "BEGIN TRANSACTION;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "BEGIN failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }

    char command[64];
    sprintf(command, "SELECT determineSpeedingViolationsAndFines(%d)", maxFineTotal);
    res = PQexec(conn, command);
    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        fprintf(stderr, "SELECT Stored Function failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    int totalFines = atoi(PQgetvalue(res, 0, 0));
    res = PQexec(conn, "COMMIT;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "COMMIT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    PQclear(res);
    return(totalFines);
}

int main(int argc, char **argv)
{
    PGconn *conn;
    int theResult;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: ./runHighwayApplication <username> <password>\n");
        exit(EXIT_FAILURE);
    }

    char *userID = argv[1];
    char *pwd = argv[2];

    char conninfo[MAXCONNECTIONSTRINGSIZE] = "host=cse180-db.lt.ucsc.edu user=";
    strcat(conninfo, userID);
    strcat(conninfo, " password=");
    strcat(conninfo, pwd);

    /* Make a connection to the database */
    conn = PQconnectdb(conninfo);

    /* Check to see if the database connection was successfully made. */
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s\n",
                PQerrorMessage(conn));
        bad_exit(conn);
    }

    int result;
    
    /* Perform the calls to printCameraPhotoCount listed in Section 6 of Lab4,
     * printing error message if there's an error.
     */
    result = printCameraPhotoCount(conn,951);
    if(result == -1){
        printf("No camera exists whose id is %d\n",951);
    }
    

    result = printCameraPhotoCount(conn,960);
    if(result == -1){
        printf("No camera exists whose id is %d\n",960);
    }
    
    result = printCameraPhotoCount(conn,856);
    if(result == -1){
        printf("No camera exists whose id is %d\n",856);
    }
    

    result = printCameraPhotoCount(conn,904);
    if(result == -1){
        printf("No camera exists whose id is %d\n",904);
    }
    
    /* Extra newline for readability */
    printf("\n");

    
    /* Perform the calls to openAllExits listed in Section 6 of Lab4,
     * and print messages as described.
     */
    result = openAllExits(conn,101);
    if(result == -1){
        printf("There is no highway whose number is %d\n",101);
    }
    else if(result >= 0){
        printf("%d exits were opened by openAllExits\n",result);
    }
    

    result = openAllExits(conn,13);
    if(result == -1){
        printf("There is no highway whose number is %d\n",13);
    }
    else if(result >= 0){
        printf("%d exits were opened by openAllExits\n",result);
    }


    result = openAllExits(conn,280);
    if(result == -1){
        printf("There is no highway whose number is %d\n",280);
    }
    else if(result >= 0){
        printf("%d exits were opened by openAllExits\n",result);
    }
    
    result = openAllExits(conn,17);
    if(result == -1){
        printf("There is no highway whose number is %d\n",17);
    }
    else if(result >= 0){
        printf("%d exits were opened by openAllExits\n",result);
    }
    
    /* Extra newline for readability */
    printf("\n");

    
    /* Perform the calls to determineSpeedingViolationsAndFines listed in Section
     * 6 of Lab4, and print messages as described.
     * You may use helper functions to do this, if you want.
     */
    result = determineSpeedingViolationsAndFines(conn,300);
    if(result >= 0){
        printf("Total fines for maxFineTotal %d is %d\n",300,result);
    }
    else{
        printf("error in stored Function for maxtotalfine 300\n");
        bad_exit(conn);
    }

    result = determineSpeedingViolationsAndFines(conn,240);
    if(result >= 0){
        printf("Total fines for maxFineTotal %d is %d\n",240,result);
    }
    else{
        printf("error in stored Function for maxtotalfine 240\n");
        bad_exit(conn);
    }

    result = determineSpeedingViolationsAndFines(conn,210);
    if(result >= 0){
        printf("Total fines for maxFineTotal %d is %d\n",210,result);
    }
    else{
        printf("error in stored Function for maxtotalfine 210\n");
        bad_exit(conn);
    }

    result = determineSpeedingViolationsAndFines(conn,165);
    if(result >= 0){
        printf("Total fines for maxFineTotal %d is %d\n",165,result);
    }
    else{
        printf("error in stored Function for maxtotalfine 165\n");
        bad_exit(conn);
    }
    good_exit(conn);
    return 0;
}
