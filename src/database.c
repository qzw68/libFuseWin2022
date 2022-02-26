#include "database.h"
#include "ntapfuse_ops.h"
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <sqlite3.h>

#define TIME_MAX 80
#define BUF_MAX 600

/* global pointer to database connection object */
sqlite3 *DB;

/*TODO: add commented documentation here for each function sig*/
/*TODO: figure out meaning of null parameters in SQL exec */
/*TODO: update the quotas database on log_read and log write */

int open_db() {
        int rc1, rc2, rc3;
        char *err = NULL;
        const char *filename = "log.db";
        /* note: backslash method is the preferred way to do multline strings */
        const char *sql1 = "CREATE TABLE IF NOT EXISTS Logs(\
                                   Time TEXT,\
                                   UID INT,\
                                   Operation TEXT,\
                                   Size INT,\
                                   Path TEXT);";

        /* why does this work? the backlash escapes the newline \n char! */
        const char *sql2 = "CREATE TABLE IF NOT EXISTS Quotas(\
                                   Time TEXT,\
                                   UID INT,\
                                   Usage INT,\
                                   Quota INT);";
        
        /* do we need to insert path shenagains here? doesn't look like it */
        rc1 = sqlite3_open(filename, &DB);

        if (rc1 != SQLITE_OK) {
                /* we cast void* to string is this right? */
                fprintf(stderr, "Can't open database connection: %s\n", 
                              (char *) sqlite3_errmsg16(DB)); 
                sqlite3_close(DB);
                return -1;
        }

        rc2 = sqlite3_exec(DB, sql1, NULL, NULL,&err);

        if (rc2 != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err);
                sqlite3_free(err);
        }

        rc3 = sqlite3_exec(DB, sql2, NULL, NULL, &err);

        if (rc3 != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err);
                sqlite3_free(err);
        }

        return rc1 && rc2 && rc3;
}


void close_db() {
        sqlite3_close(DB);
}

int log_read(char *operation, char *path, size_t size) {
        /* SQL commands to compile */
        const char *sql = "CREATE TABLE if not exists Logs(\
                               Time TEXT,\
                               UID INT,\
                               Operation TEXT,\
                               Size INT,\
                               Path TEXT);\
                           INSERT INTO Logs\
                               VALUES (%s, %d, %s, %d, %s);";

        char *sqlbuf = NULL; 
        char *timebuf = NULL;

        char *err = NULL;

        time_t now;
        int rc;

        /* allocate head data and check for succcess */
        sqlbuf = malloc(BUF_MAX);
        if (!sqlbuf) {
                fprintf(stderr,"ERROR: couldn't allocate SQL buffer\n");
                return -ENOMEM;
        }
        
        timebuf = malloc(TIME_MAX);
        if (!timebuf) {
                fprintf(stderr,"ERROR: couldn't allocate Time buffer\n");
                return -ENOMEM;
        }

        /* get current time and convert it to readable */
        time(&now);
        strftime(timebuf, TIME_MAX, "%c", localtime(&now));

        /* read information into buffer s */
        sprintf(sqlbuf, sql, addquote(timebuf), 
                        "0", /*hard code user rn */
                        addquote(operation),
                        size, addquote(path));

        /* execute the SQL statement & check for success*/
        rc = sqlite3_exec(DB, sqlbuf, NULL, NULL, &err);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err);
                sqlite3_free(err);
        }

        /* free all heap data */
        free(sqlbuf);
        free(timebuf);
        return rc;
}

int log_write(char *operation, char *path, size_t size) {
        /* SQL commands to compile */
        const char *sql = "CREATE TABLE if not exists Logs(\
                               Time TEXT,\ 
                               UID INT,\ 
                               Operation TEXT,\
                               Size INT,\
                               Path TEXT);\ 
                           INSERT INTO Logs\
                               VALUES(%s, %d, %s, %d, %s);";  
        char *err = NULL;
        char *sqlbuf = NULL;
        char *timebuf = NULL;
        time_t now;
        int rc;

        /* allocate head data and check for succcess */
        sqlbuf = malloc(BUF_MAX);
        if (!sqlbuf) {
                fprintf(stderr,"ERROR: couldn't allocate SQL buffer\n");
                return -ENOMEM;
        }


        timebuf = malloc(TIME_MAX);
        if (!timebuf) {
                fprintf(stderr,"ERROR: couldn't allocate Time buffer\n");
                return -ENOMEM;
        }

        /* get current time and convert it to readable */
        time(&now);
        strftime(timebuf, TIME_MAX,"%c",localtime(&now)); // convert time to readable

        /* read information into buffer s */
        sprintf(bufsql, sql, addquote(timebuf),
                        "0", addquote(operation), // hard code user rn
                        size, addquote(path));

        /* execute the SQL statement & check for success*/
        rc = sqlite3_exec(DB, bufsql, NULL, NULL, &err);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err);
                sqlite3_free(err);
        }
	
        /* free all heap data */
        free(sqlbuf);
        free(timebuf);
        return rc;
}
