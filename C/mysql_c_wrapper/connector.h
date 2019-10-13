/**
 * @file connector.h
 * @author Garry Rice (Arprog)
 * @date 10/12/2019
 * @brief MySQL C Wrapper
 *
 * This is a structure that simply wraps around the structures
 * used by MySQL C Connector.
 *
 * This wrapper utilizes Oracle's MySQL C Connnector. Therefore,
 * this piece of software adhere's to their open source license.
 * @see https://dev.mysql.com/doc/connector-c/en/preface.html
 */

#ifndef connector_h
#define connector_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql.h>
#include <stdbool.h>

/**
 * @brief Connector base structure
 *
 * A structure used to wrap around the MySQL C Connector.
 */
typedef struct _connector
{
	MYSQL* con; /**<Main MySQL structure*/
	MYSQL_RES* result; /**<MySQL results structure*/
	MYSQL_ROW row; /**<MySQL Row structure*/
	MYSQL_FIELD* field; /**<MySQL Field structure*/
	unsigned num_fields; /**<Stores number of field names*/
	my_ulonglong num_affected_rows; /**<Stores number of affected rows*/
	char* error; /**<Stores any error messages encountered*/
	bool hasError; /**<Stores Connector error state*/
	bool lib_initialized; /**<Stores if mysql_client_lib has been initialized or not*/
	bool definitionStatement; /**<Stores if active query is a Definition Statement or not*/
} Connector;

/**
 * @brief Initializes Connector structure
 *
 * Use this function to properly initialize the Connector.
 * @param m Connector this function belongs to.
 * @param con MYSQL main structure to be used for initialization.
 * @return Connector is successfully initialized or not.
 */
bool Connector_init(Connector* m, MYSQL* con);

/**
 * @brief Connects to target MySQL database
 *
 * Use this function to connect the Connector to a target database.
 * @param m Connector this function belongs to.
 * @param host Target location of MySQL database.
 * @param user MySQL user name.
 * @param pass MySQL password.
 * @param db Target database/schema to connect to.
 * @param port Port number that target MySQL database uses.
 * @param unix_socket Unix Socket that can be used to connect to target MySQL database.
 * @param client_flag Flag to tell MySQL what features you need to do what you need to do.
 * @return Connector successfully connected to target or not.
 */
bool Connector_connect(Connector* m, const char* host, const char* user, const char* pass, const char* db, unsigned port, const char* unix_socket, unsigned long client_flag);

/**
 * @brief Executes a query
 *
 * Use this function to execute queries on the target database.
 * @param m Connector this function belongs to.
 * @param query Query to be executed.
 * @return Query was successfully executed or not.
 */
bool Connector_query(Connector* m, const char* query);

/**
 * @brief Checks for rows left
 *
 * Use this function if you need to check if there are more rows to be
 * processed.
 * @param m Connector this function belongs to.
 * @return A MYSQL_ROW structure for the next row, or NULL. @see https://dev.mysql.com/doc/refman/5.7/en/mysql-fetch-row.html
 */
MYSQL_ROW Connector_next(Connector* m);

/**
 * @brief Frees up all resources
 *
 * Use this function when finished using the connector. This function
 * will properly free all resources from inside the Connector.
 * @param m Connector this function belongs to.
 */
void Connector_destroy(Connector* m);

#ifdef __cplusplus
}
#endif

#endif

