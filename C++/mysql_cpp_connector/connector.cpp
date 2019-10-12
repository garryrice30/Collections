/**
 *
 * @file connector.cpp
 * @author Garry Rice
 * @date 10/12/2019
 * @brief MySQL CPP Connector source file
 */

#include "connector.h"

/**
 * Basic Constructor
 * @param con passes in another main MYSQL C Structure pointer that can be used for initialization.
 */
Connector::Connector(MYSQL* con) :
    _row{MYSQL_ROW()},
    _data{},
    _fieldNames{},
    _error{}
{
    if(mysql_library_init(0,nullptr,nullptr))
	{
		_error = "Could not initialize MySQL client library.";
		_lib_failed = true;
	}
	else
	{
	    _lib_initialized = true;
		_con = mysql_init(con);
		if(!_con)
		{
			_error = mysql_error(_con);
		}
	}
}

/**
 * Copy Constructor
 * @param con Connector to be copied.
 */
Connector::Connector(const Connector& con) :
    _row{MYSQL_ROW()},
    _data{},
    _fieldNames{},
    _error{}
{
    _num_fields = 0;
    _affectedRows = 0;
    _connected = false;
    _lib_failed = false;
    _lib_initialized = con.isLibraryInitialized();
    _definitionStatement = false;
    _field = nullptr;
    _res = nullptr;
    _con = nullptr;
    if(!_lib_initialized)
    {
        if(mysql_library_init(0,nullptr,nullptr))
        {
            _error = "Could not initialize MySQL client library.";
            _lib_failed = true;
        }
        else
        {
            _lib_initialized = true;
        }
    }

    mysql_init(con.getMYSQL_Ptr());
    if(!con.getMYSQL_Ptr())
    {
        _error = mysql_error(con.getMYSQL_Ptr());
    }
}

/**
 * Overloaded assignment operator
 * @param rhs Connector that will be copied based on which it is assigned.
 * @return The current object after assignment has been complete.
 */
Connector& Connector::operator=(const Connector& rhs)
{
    _error = rhs.getError();
    _lib_initialized = rhs.isLibraryInitialized();
    _lib_failed = rhs.MYSQL_lib_failed();
    _definitionStatement = rhs.isDefinitionStatement();
    _connected = rhs.isConnected();
    _affectedRows = rhs.getNumAffectedRows();
    _fieldNames = rhs.getFieldNames();
    _num_fields = rhs.getNumFields();
    _row = rhs.getMYSQL_ROW_Struct();
    _field = rhs.getMYSQL_FIELD_Ptr();
    _res = rhs.getMYSQL_RES_Ptr();
    _con = rhs.getMYSQL_Ptr();

    for(size_t i = 0; i < rhs.getData().size(); i++)
    {
        _data[i] = rhs.getData()[i];
    }
    return *this;
}

/**
 * Used to connect to a target database
 * @param host stores host name to target mysql server.
 * @param user stores mysql user name.
 * @param pass stores mysql password.
 * @param db stores target mysql database/schema.
 * @param port stores port number to target mysql server that mysql runs on.
 * @param unix_port stores the unix_port that an be used to connect to mysql on target server.
 * @param client_flags stores flag information passed to main MYSQL C Structure to enable/disable features.
 * @return If Connector has successfully connected or not.
 */
bool Connector::connect(const char* host, const char* user, const char* pass, const char* db, const unsigned& port, const char* unix_port, const unsigned long& client_flags)
{
	if(mysql_real_connect(_con,host,user,pass,db,port,unix_port,client_flags) == nullptr)
	{
		_error = mysql_error(_con);
		_connected = false;
	}
	else
	{
		_connected = true;
	}
	return _connected;
}

/**
 * Used to execute a query on a target database
 * @param query stores query in a const char* to be executed on target database.
 * @return If query was successfully executed or not.
 */
bool Connector::query(const char* query)
{
    _error.clear();
    _data.clear();
    _fieldNames.clear();
    _affectedRows = 0;
    _num_fields = 0;
    bool rval = true;
	if(mysql_query(_con,query))
	{
	    rval = false;
		_error = mysql_error(_con);
	}
	else
	{
		_res = mysql_store_result(_con);
		if(!_res)
		{
			if(mysql_field_count(_con) != 0)
			{
			    rval = false;
				_error = mysql_error(_con);
			}
			else
			{
			    _definitionStatement = true;
				_affectedRows = mysql_affected_rows(_con);
			}
		}
		else
		{
		    _definitionStatement = false;
			_num_fields = mysql_num_fields(_res);
			while((_field = mysql_fetch_field(_res)))
			{
				_fieldNames.push_back(_field->name);
			}

			while((_row = mysql_fetch_row(_res)))
			{
				//vector<boost::any> d;
				vector<any> d;
				for(int i = 0; i < _num_fields; i++)
				{
					d.push_back(_row[i]);
				}
				_data.push_back(d);
				d.clear();
			}
		}
	}
	return rval;
}

/**
 * Basic Destructor
 */
Connector::~Connector()
{
	if(_res)
	{
		mysql_free_result(_res);
		_res = nullptr;
	}
	if(_con)
	{
		mysql_close(_con);
	}
	if(!_lib_failed && _lib_initialized)
	{
		mysql_library_end();
	}
}
