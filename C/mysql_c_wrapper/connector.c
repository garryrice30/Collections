/**
 * @file connector.cpp
 * @author Garry Rice (Arprog)
 * @date 10/12/2019
 * @brief MySQL C Wrapper
 *
 * This is the source file for the Connector structure.
 */

#include "connector.h"
#include "connector_private.h"

size_t Connector_getStrSize(const char* str)
{
	return strlen(str);
}

void Connector_set_error(Connector* m, const char* message)
{
	const char* text = message ? message : "";
	size_t size = Connector_getStrSize(text);
	char* buffer = (char*)realloc(m->error, size + 1);
	if(buffer == NULL)
	{
		return;
	}
	memcpy(buffer, text, size);
	buffer[size] = '\0';
	m->error = buffer;
	m->hasError = true;
}

void Connector_clear_error(Connector* m)
{
	Connector_set_error(m, CONNECTOR_NO_ERROR);
	m->hasError = false;
}

bool Connector_store_scalar(Connector* m, const char* value)
{
	free(m->scalar);
	m->scalar = NULL;
	m->scalar_is_null = (value == NULL);
	if(value == NULL)
	{
		return true;
	}
	size_t size = Connector_getStrSize(value);
	m->scalar = (char*)malloc(size + 1);
	if(m->scalar == NULL)
	{
		m->scalar_is_null = true;
		return false;
	}
	memcpy(m->scalar, value, size);
	m->scalar[size] = '\0';
	return true;
}

bool Connector_init(Connector* m, MYSQL* con)
{
	m->con = NULL;
	m->result = NULL;
	m->field = NULL;
	m->row = NULL;
	m->num_fields = 0;
	m->num_affected_rows = 0;
	m->error = NULL;
	m->hasError = false;
	m->lib_initialized = false;
	m->definitionStatement = false;
	m->num_result_sets = 0;
	m->scalar = NULL;
	m->scalar_is_null = true;
	Connector_clear_error(m);

	bool rval = false;
	if(mysql_library_init(0,NULL,NULL))
	{
		Connector_set_error(m, "Could not initialize MySQL client library.");
	}
	else
	{
		m->lib_initialized = true;
		m->con = mysql_init(con);
		if(m->con == NULL)
		{
			//mysql_error() cannot be used here, there is no structure to ask.
			Connector_set_error(m, "Could not initialize MySQL structure.");
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
	Connector_clear_error(m);
	if(mysql_real_connect(m->con, host, user, pass, db, port, unix_port, client_flags) == NULL)
	{
		Connector_set_error(m, mysql_error(m->con));
		rval = false;
	}
	return rval;
}

void Connector_release_result(Connector* m)
{
	if(m->result != NULL)
	{
		mysql_free_result(m->result);
		m->result = NULL;
	}
	m->field = NULL;
	m->row = NULL;
	m->num_fields = 0;
}

bool Connector_query(Connector* m, const char* query)
{
	bool rval = true;
	Connector_clear_error(m);
	//A previous result set has to go before the next statement is sent.
	Connector_release_result(m);
	m->num_affected_rows = 0;
	m->definitionStatement = false;

	if(mysql_query(m->con, query))
	{
		Connector_set_error(m, mysql_error(m->con));
		rval = false;
	}
	else
	{
		m->result = mysql_store_result(m->con);
		if(m->result == NULL)
		{
			if(mysql_field_count(m->con) != 0)
			{
				Connector_set_error(m, mysql_error(m->con));
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
	Connector_release_result(m);
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
	if(m->scalar != NULL)
	{
		free(m->scalar);
		m->scalar = NULL;
	}
	m->scalar_is_null = true;
	if(m->lib_initialized == true)
	{
		mysql_library_end();
		m->lib_initialized = false;
	}
}
