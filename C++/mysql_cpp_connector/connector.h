/**
 *
 * @file connector.h
 * @author Garry Rice
 * @date 10/12/2019
 * @brief This is a C++ Connector written using MySQL C Connector
 */

 /*     License Information
  *
  * Licenses that this piece of software adheres to:
  * Oracle: https://dev.mysql.com/doc/connector-c/en/preface.html
  * Boost (Optional): https://www.boost.org/users/license.html
  */

 // Don't forget the header guards ...
#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <mysql.h> /**MySQL header needed for MySQL C library*/

#include <vector> /**Library needed to use std::vector*/
using std::vector;

#include <string> /**Library needed to use std::string*/
using std::string;

#include <any> /**Library needed to use std::any*/
using std::any;
/*
#include <boost/any.hpp> <--- Library needed to use boost::any
*/


class Connector
{
	MYSQL* _con = nullptr; /**<Main MySQL C structure.*/
	MYSQL_RES* _res = nullptr; /**<MySQL Results C structure.*/
	MYSQL_FIELD* _field = nullptr; /**<MYSQL Field C structure.*/
	MYSQL_ROW _row; /**<MYSQL Row C Structure*/
	int _num_fields = 0; /**<Used to store number of fields retrieved*/
	//vector<vector<boost::any> > _data; <-- Two dimensional std::vector used to store data retrieved using boost::any
	vector<vector<any> > _data; /**<Two dimensional std::vector used to store data retrived.*/
	vector<string> _fieldNames; /**<Vector of std::string used to store field names retrieved.*/
	my_ulonglong _affectedRows = 0; /**<Used to store affected rows when no data can be retrieved*/
	bool _connected = false; /**<Boolean that stores if Connector has established a connection with it's target database or not*/
	bool _lib_failed = false; /**<Boolean that stores if the mysql_init_lib fails to initialize or not*/
	bool _lib_initialized = false; /**<Boolean that stores if mysql_init_lib has already be initialized or not*/
	bool _definitionStatement = false; /**<Boolean that stores if the processed query is either a Definition Statement or a Manipulation Statement*/
	string _error; /**<String that stores any error messages that is encountered*/

	public:
	Connector(MYSQL* con = nullptr);
	Connector(const Connector& con);
	Connector& operator=(const Connector& rhs);
	bool connect(const char* host, const char* user, const char* pass, const char* db, const unsigned& port, const char* uport, const unsigned long& flags);
	bool query(const char* query);
	template <typename... Args>
	bool query(const char* q, const Args*... args);
	inline bool isDefinitionStatement() const {return _definitionStatement;}
	inline bool isLibraryInitialized() const {return _lib_initialized;}
	inline bool MYSQL_lib_failed() const {return _lib_failed;}
	inline bool isConnected() const {return _connected;}
	inline string getError() const {return _error;}
	inline my_ulonglong getNumAffectedRows() const {return _affectedRows;}
	inline int getNumFields() const {return _num_fields;}
	//inline vector<vector<boost::any> > getData() const {return _data;} <-- Accessor that returns 2D std::vector of boost::any that possibly houses retrieved data.
	inline vector<vector<any> > getData() const {return _data;}
	inline vector<string> getFieldNames() const {return _fieldNames;}
	inline MYSQL* getMYSQL_Ptr() const {return _con;}
	inline MYSQL_RES* getMYSQL_RES_Ptr() const {return _res;}
	inline MYSQL_FIELD* getMYSQL_FIELD_Ptr() const {return _field;}
	inline MYSQL_ROW getMYSQL_ROW_Struct() const {return _row;}
	~Connector();
};

/**
 * Processes given query to target database supporting multiple arguments.
 * @param q const char* that stores the query to be executed.
 * @param args Variadic Template (Parameter Pack) that houses the arguments needed to add to the query to be executed.
 * @see Connector::query(const char* query)
 * @return If query has successfully been executed or not.
 */
template<typename... Args>
bool Connector::query(const char* q,const Args*... args)
{
	query(q);
	return query(args ...);
}

#endif // CONNECTOR_H
