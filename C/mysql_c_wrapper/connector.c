/**
 * @file connector.cpp
 * @author Garry Rice (Arprog)
 * @date 10/12/2019
 * @brief MySQL C Wrapper
 *
 * This is the source file for the Connector structure.
 */

#include "connector.h"

//Utility functions

size_t Connector_getStrSize(const char* str)
{
	return strlen(str);
}

bool Connector_init(Connector* m, MYSQL* con)
{
	m->con = NULL;
	m->result = NULL;
	m->field = NULL;
	m->num_fields = 0;
	m->error = NULL;
	m->hasError = false;
	m->num_affected_rows = 0;
	m->lib_initialized = false;
	m->definitionStatement = false;
	size_t size = Connector_getStrSize("No error to report.");
	m->error = (char*)malloc(size+1);
	strcpy(m->error,"No error to report.");
	m->error[size] = '\0';
	bool rval = false;
	if(mysql_library_init(0,NULL,NULL))
	{
		m->hasError = true;
		size = Connector_getStrSize("Could not initialize MySQL client library.");
		m->error = (char*)realloc(m->error,size+1);
		strcpy(m->error,"Could not initialize MySQL client library.");
		m->error[size] = '\0';
	}
	else
	{
		m->lib_initialized = true;
		m->con = mysql_init(con);
		if(m->con == NULL)
		{
			m->hasError = true;
			size_t size = Connector_getStrSize(mysql_error(m->con));
			m->error = (char*)realloc(m->error,size+1);
			strcpy(m->error,mysql_error(m->con));
			m->error[size] = '\0';
		}
		else
		{
			rval = true;
		}
	}
	return rval;
}

bool Connector_connect(Connector* m, const char* host, const char* user, const char* pass, const char* db, unsigned port, const char* unix_port, unsigned long client_flags)
{
	bool rval = true;
	m->hasError = false;
	size_t size = Connector_getStrSize("No error to report.");
	m->error = (char*)realloc(m->error,size+1);
	strcpy(m->error,"No error to report.");
	m->error[size] = '\0';
	if(mysql_real_connect(m->con, host, user, pass, db, port, unix_port, client_flags) == NULL)
	{
		m->hasError = true;
		size_t size = Connector_getStrSize(mysql_error(m->con));
		m->error = (char*)realloc(m->error,size+1);
		strcpy(m->error,mysql_error(m->con));
		m->error[size] = '\0';
		rval = false;
	}
	return rval;
}

bool Connector_query(Connector* m, const char* query)
{
	bool rval = true;
	m->hasError = false;
	size_t size = Connector_getStrSize("No error to report.");
	m->error = (char*)realloc(m->error,size+1);
	strcpy(m->error,"No error to report");
	m->error[size] = '\0';
	if(mysql_query(m->con,query))
	{
		m->hasError = true;
		size_t size = Connector_getStrSize(mysql_error(m->con));
		m->error = (char*)realloc(m->error, size+1);
		strcpy(m->error,mysql_error(m->con));
		m->error[size] = '\0';
		rval = false;
	}
	else
	{
		m->result = mysql_store_result(m->con);
		if(m->result == NULL)
		{
			if(mysql_field_count(m->con) != 0)
			{
				m->hasError = true;
				size_t size = Connector_getStrSize(mysql_error(m->con));
				m->error = (char*)realloc(m->error,size+1);
				strcpy(m->error,mysql_error(m->con));
				m->error[size] = '\0';
				rval = false;
			}
			else
			{
			    m->definitionStatement = true;
				m->num_affected_rows = mysql_affected_rows(m->con);
			}
		}
		else
		{
		    m->definitionStatement = false;
		    m->num_fields = mysql_num_fields(m->result);
            m->field = mysql_fetch_fields(m->result);
		}
	}
	return rval;
}

MYSQL_ROW Connector_next(Connector* m)
{
	m->row = mysql_fetch_row(m->result);
	return m->row;
}

void Connector_destroy(Connector* m)
{
	if(m->result != NULL)
	{
		mysql_free_result(m->result);
		m->result = NULL;
	}
	if(m->con != NULL)
	{
		mysql_close(m->con);
		m->con = NULL;
	}
	if(m->error != NULL)
	{
		free(m->error);
		m->error = NULL;
	}
	if(m->lib_initialized == true)
	{
		mysql_library_end();
	}
}
