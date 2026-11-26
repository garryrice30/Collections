/**
 * @file connector_call.cpp
 * @author Garry Rice (Arprog)
 * @date 10/12/2019
 * @brief Stored procedure and stored function support for the MySQL C Wrapper.
 *
 * A stored procedure is not a normal query: it can return several result sets
 * in a row, and MySQL will not accept the next statement until every one of
 * them has been read. That is what this file takes care of.
 */

#include "connector.h"
#include "connector_private.h"

/**
 * Builds "KEYWORD name(arguments)" for a call.
 * @param keyword CALL or SELECT.
 * @param name Name of the procedure or function.
 * @param arguments Argument list as written in SQL, NULL for none.
 * @return Allocated statement, or NULL when memory ran out.
 */
static char* Connector_build_statement(const char* keyword, const char* name, const char* arguments)
{
	size_t keywordLength = strlen(keyword);
	size_t nameLength = strlen(name);
	size_t argumentsLength = arguments ? strlen(arguments) : 0;
	size_t length = keywordLength + 1 + nameLength + 1 + argumentsLength + 2;
	char* statement = (char*)malloc(length + 1);
	if(statement == NULL)
	{
		return NULL;
	}
	snprintf(statement, length + 1, "%s %s(%s)", keyword, name, arguments ? arguments : "");
	return statement;
}

/**
 * Reads every result set left on the connection.
 * @param m Connector this function belongs to.
 * @param callback Called for each row when not NULL.
 * @param user Passed through to callback.
 * @return Every result set could be read or not.
 */
static bool Connector_drain_results(Connector* m, Connector_row_callback callback, void* user)
{
	m->num_result_sets = 0;
	m->definitionStatement = false;
	m->num_affected_rows = 0;

	for(;;)
	{
		MYSQL_RES* result = mysql_store_result(m->con);
		if(result != NULL)
		{
			m->num_result_sets++;
			m->num_fields = mysql_num_fields(result);
			m->field = mysql_fetch_fields(result);
			if(callback != NULL)
			{
				MYSQL_ROW row;
				while((row = mysql_fetch_row(result)) != NULL)
				{
					if(!callback(row, m->num_fields, user))
					{
						mysql_free_result(result);
						m->field = NULL;
						m->num_fields = 0;
						Connector_set_error(m, "Row callback stopped the stored procedure call.");
						return false;
					}
				}
			}
			mysql_free_result(result);
			m->field = NULL;
			m->num_fields = 0;
		}
		else if(mysql_field_count(m->con) != 0)
		{
			// The call produced a result set but it could not be read.
			Connector_set_error(m, mysql_error(m->con));
			return false;
		}
		else
		{
			m->definitionStatement = true;
			m->num_affected_rows = mysql_affected_rows(m->con);
		}

		if(!mysql_more_results(m->con))
		{
			break;
		}
		if(mysql_next_result(m->con) != 0)
		{
			Connector_set_error(m, mysql_error(m->con));
			return false;
		}
	}

	return true;
}

bool Connector_call(Connector* m, const char* procedure, const char* arguments)
{
	return Connector_call_rows(m, procedure, arguments, NULL, NULL);
}

bool Connector_call_rows(Connector* m, const char* procedure, const char* arguments, Connector_row_callback callback, void* user)
{
	if(m == NULL || m->con == NULL || procedure == NULL)
	{
		return false;
	}

	Connector_clear_error(m);
	Connector_release_result(m);
	m->num_result_sets = 0;
	m->num_affected_rows = 0;

	char* statement = Connector_build_statement("CALL", procedure, arguments);
	if(statement == NULL)
	{
		Connector_set_error(m, "Could not allocate the stored procedure statement.");
		return false;
	}

	bool rval;
	if(mysql_query(m->con, statement))
	{
		Connector_set_error(m, mysql_error(m->con));
		rval = false;
	}
	else
	{
		rval = Connector_drain_results(m, callback, user);
	}

	free(statement);
	return rval;
}

bool Connector_call_function(Connector* m, const char* function, const char* arguments)
{
	if(m == NULL || m->con == NULL || function == NULL)
	{
		return false;
	}

	Connector_clear_error(m);
	Connector_release_result(m);
	m->num_result_sets = 0;
	m->num_affected_rows = 0;

	char* statement = Connector_build_statement("SELECT", function, arguments);
	if(statement == NULL)
	{
		Connector_set_error(m, "Could not allocate the stored function statement.");
		return false;
	}

	bool rval = false;
	if(mysql_query(m->con, statement))
	{
		Connector_set_error(m, mysql_error(m->con));
	}
	else
	{
		MYSQL_RES* result = mysql_store_result(m->con);
		if(result == NULL)
		{
			Connector_set_error(m, mysql_error(m->con));
		}
		else
		{
			m->num_result_sets = 1;
			m->num_fields = mysql_num_fields(result);
			m->field = mysql_fetch_fields(result);
			if(m->num_fields != 1)
			{
				Connector_set_error(m, "Stored function did not return exactly one value.");
			}
			else
			{
				MYSQL_ROW row = mysql_fetch_row(result);
				rval = Connector_store_scalar(m, (row != NULL) ? row[0] : NULL);
				if(!rval)
				{
					Connector_set_error(m, "Could not allocate the stored function result.");
				}
			}
			mysql_free_result(result);
			m->field = NULL;
			m->num_fields = 0;
		}
	}

	free(statement);
	return rval;
}
