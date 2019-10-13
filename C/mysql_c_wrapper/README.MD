MySQL C Wrapper

This is a C Project that simply wraps around Oracles's MySQL C Connector. There's really not much to it that's too terribly complicated.
It has the ability to do basic queries (Select, Show, Create, Insert, ect). Here are some things I plan on adding to it in the future:
1. I plan on having a query function that will take arguments, much like printf(). It's not that I can't do it, I just chose not to
implement it right now.
2. Sooner or later, functionality will be added for stored procedures and stored functions. I wouldn't count on any time soon.
3. I'm very sure it needs to be optimized. This is verion 1.0.0, so it wouldn't suprise me one bit.

That's all that really comes to mind at the moment. I'd love to talk about it more, but there's just not much to it. It's just a simple
MySQL C Wrapper.

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
