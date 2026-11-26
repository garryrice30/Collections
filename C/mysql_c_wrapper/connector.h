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
#include <stdarg.h>
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
	unsigned num_result_sets; /**<Stores how many result sets the last stored procedure call returned*/
	char* scalar; /**<Stores the value returned by Connector_call_function (NULL when the function returned NULL)*/
	bool scalar_is_null; /**<Stores if that value was NULL*/
} Connector;

/**
 * @brief Row callback used by Connector_call_rows
 *
 * Called once for every row of every result set a stored procedure produces.
 * @param row MySQL row, valid until the callback returns.
 * @param num_fields Number of fields in the result set this row belongs to.
 * @param user Pointer passed to Connector_call_rows.
 * @return false to stop reading rows early (the call then reports an error).
 */
typedef bool (*Connector_row_callback)(MYSQL_ROW row, unsigned num_fields, void* user);

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
 * @brief Executes a query built from a printf style format
 *
 * Use this function when the query has values in it, instead of building the
 * string yourself. Values read from the format are quoted for you, so a name
 * containing a quote or a backslash cannot break the statement.
 *
 * Supported specifiers:
 * - %s string, escaped and wrapped in single quotes. A NULL pointer becomes NULL.
 * - %d, %i, %ld, %lld signed integers.
 * - %u, %lu, %llu unsigned integers.
 * - %f double.
 * - %% a literal percent sign.
 *
 * Anything else is rejected and reported through hasError, rather than
 * producing a broken statement.
 * @param m Connector this function belongs to.
 * @param format Query with format specifiers.
 * @param ... One value per specifier, in order.
 * @return Query was successfully executed or not.
 * @code
 * Connector_queryf(&m, "SELECT * FROM Cars WHERE make = %s AND year > %d", "Ford", 2000);
 * @endcode
 */
bool Connector_queryf(Connector* m, const char* format, ...);

/**
 * @brief Same as Connector_queryf, taking an already started va_list.
 * @param m Connector this function belongs to.
 * @param format Query with format specifiers.
 * @param args Values for the format specifiers.
 * @return Query was successfully executed or not.
 */
bool Connector_vqueryf(Connector* m, const char* format, va_list args);

/**
 * @brief Calls a stored procedure
 *
 * Executes CALL procedure(arguments) and reads every result set the
 * procedure produces, so a procedure that returns rows and then a status
 * result still leaves the connection usable. The rows themselves are not
 * stored; use Connector_call_rows if you need them. num_result_sets holds
 * how many result sets the call produced.
 * @param m Connector this function belongs to.
 * @param procedure Name of the stored procedure.
 * @param arguments Argument list as written in SQL (for example "'Ford', 2000"), or NULL for none.
 * @return Call was successfully executed or not.
 */
bool Connector_call(Connector* m, const char* procedure, const char* arguments);

/**
 * @brief Calls a stored procedure and hands every row to a callback
 *
 * Works like Connector_call, but every row of every result set is passed to
 * callback while it is still valid. This is the way to read a procedure that
 * returns more than one result set, which Connector_next cannot do.
 * @param m Connector this function belongs to.
 * @param procedure Name of the stored procedure.
 * @param arguments Argument list as written in SQL, or NULL for none.
 * @param callback Called once per row. NULL behaves like Connector_call.
 * @param user Passed through to callback untouched.
 * @return Call was successfully executed or not.
 * @code
 * static bool print_row(MYSQL_ROW row, unsigned num_fields, void* user)
 * {
 *     for(unsigned i = 0; i < num_fields; i++)
 *     {
 *         printf("%s | ", row[i] ? row[i] : "NULL");
 *     }
 *     puts("");
 *     return true;
 * }
 *
 * Connector_call_rows(&m, "cars_by_make", "'Ford'", print_row, NULL);
 * @endcode
 */
bool Connector_call_rows(Connector* m, const char* procedure, const char* arguments, Connector_row_callback callback, void* user);

/**
 * @brief Calls a stored function
 *
 * Executes SELECT function(arguments) and stores the single value it
 * returns in the scalar member, which makes a stored function usable the same
 * way a built in one is. Call Connector_destroy (or call this again) to free it.
 * @param m Connector this function belongs to.
 * @param function Name of the stored function.
 * @param arguments Argument list as written in SQL, or NULL for none.
 * @return Call was successfully executed or not.
 * @code
 * Connector_call_function(&m, "cars_by_make", "'Ford'");
 * printf("%s\n", m.scalar_is_null ? "NULL" : m.scalar);
 * @endcode
 */
bool Connector_call_function(Connector* m, const char* function, const char* arguments);

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
