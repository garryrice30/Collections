MySQL C Wrapper

This is a C Project that simply wraps around Oracles's MySQL C Connector. There's really not much to it that's too terribly complicated.
It has the ability to do basic queries (Select, Show, Create, Insert, ect).

Queries that take values can be written with Connector_queryf(), which works like printf() and quotes the values for you:

	Connector_queryf(&m, "select * from Cars where make = %s and year > %d", "Ford", 2000);

%s is a string and becomes a quoted, escaped literal (a NULL pointer becomes NULL), %d / %i / %ld / %lld are signed integers,
%u / %lu / %llu are unsigned integers, %f is a double and %% is a literal percent sign. Any other specifier is rejected and reported
through hasError, instead of quietly building a broken statement.

Stored procedures and stored functions are supported as well:

	Connector_call(&m, "cars_by_make", "'Ford'");                        // CALL, reads every result set
	Connector_call_rows(&m, "cars_by_make", "'Ford'", print_row, NULL);  // same, but hands each row to a callback
	Connector_call_function(&m, "count_cars", NULL);                     // SELECT, value lands in m.scalar

A stored procedure can return more than one result set, and MySQL will not accept the next statement until all of them have been
read. Connector_call() and Connector_call_rows() read every one of them, so the connection stays usable afterwards. The arguments
string is the argument list exactly as you would write it in SQL, so build it with Connector_queryf() style quoting if the values
come from elsewhere.

Here are some things I plan on adding to it in the future:

1. I'm very sure it needs to be optimized. This is verion
1.0.0, so it wouldn't suprise me one bit.

That's all that really comes to mind at the moment. I'd love to talk about it more, but there's just not much to it. It's just a simple
MySQL C Wrapper.

To build it, compile connector.c, connector_queryf.c and connector_call.c together with your own source file, for example:

	gcc -o demo main.c connector.c connector_queryf.c connector_call.c -lmysqlclient

Feel free to use this as you see fit, I hope you find it useful. Keep in mind the warnings in the README.MD at the root of the repository.
This project utilizes Oracle's MySQL C Connector. Therefore, this project respectfully adheres to Oracles open source license:
https://dev.mysql.com/doc/connector-c/en/preface.html

Here is some detailed information about the library I used to build this project:
(copied straight from mysql_version.h)

/* Version numbers for protocol & mysqld */
#define PROTOCOL_VERSION            10
#define MYSQL_SERVER_VERSION       "5.7.12"
#define MYSQL_BASE_VERSION         "mysqld-5.7"
#define MYSQL_SERVER_SUFFIX_DEF    ""
#define FRM_VER                     6
#define MYSQL_VERSION_ID            50712
#define MYSQL_PORT                  3306
#define MYSQL_PORT_DEFAULT          0
#define MYSQL_UNIX_ADDR            "/tmp/mysql.sock"
#define MYSQL_CONFIG_NAME          "my"
#define MYSQL_COMPILATION_COMMENT  "MySQL Community Server (GPL)"
#define LIBMYSQL_VERSION           "5.7.12"
#define LIBMYSQL_VERSION_ID         50712
#define SYS_SCHEMA_VERSION         "1.5.0"

#ifndef LICENSE
#define LICENSE                     GPL
#endif /* LICENSE */

I only used one Operating System/Environment to build this projet:
Windows 7 Professional x64 - mingw32-w64 i686-8.1.0-win32-dwarf-rt_v6-rev0

Thank you for taking the time to look at this project. If anyone has any questions or comments, fill free to send me an e-mail:
garryrice4383@gmail.com
I'm always welcome for the criticism and happy to any any questions.
