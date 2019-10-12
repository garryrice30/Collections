MySQL CPP Connector

This was actually one of my first projects I originally submitted to github!
I'm proud to be resubmitting it again with a few minor adjustments from the original.

The only thing the Connector object lacks is a friendly way to execute MySQL stored functions and MySQL stored procedures.
The reason being is because the was created using Oracle's MySQL C Connector as it's base (version 5.7 32 bit).
Reason for the old version is because my version also has the MySQL C Connector Embedded library and thus far it's
truly been the best library compilation I've used yet. I have issues with newer versions (though I could use MariaDB's C Connector).

Here is what you need in order to fully utilize this Connector:

MySQL C Connector - Use any version really that you like. I have my picks, but it should work with other versions, as long as the
MYSQL structure functions hasn't changed or anything like that. If so, it'd be little to no effort to adjust the code accordingly.

Boost Library (Optional) - This connector was first designed with std c++11 in mind. Therefore, it used to utilize the Boost Library
object boost::any. However, do to updates and such, this connector instead utilizes std::any object from the <any> C++ Standard Library.
It is still possible to omit std::any and revert back to using boost::any without any issues at all. Just comment out the lines that
utilize std::any and instead uncomment the lines utilizing boost::any. As a bonus, there's barely any lines to change at all! If I was
to make this into a library, one #ifdef macro could easily enable and disable one or the other. Again, this is your call.

Environments (Operating Systems, etc) that was used in creation of this project:
Linux Mint Cinnamon 19.1 x64 (Tessa) - libmysqlclient-dev 5.7.27, gcc/g++ 7.4.0 (directly from repositories), (Optional) libboost 1.65.1 (directly from repositories
Windows 7 Professional x64 - libmysql.a (MySQL Server 5.7.12), mingw32-w64 8.1.0 i686 win32_dwarf-rt_v6-rev0, (Optional) boost 1.71.0 (headers only)

Both header and sources are fully commented. There is also a demo driver file called driver.cpp that can be used to test the connector.

Things left to do:
Develop methods for stored functions and stored procedures. Something done is worth doing all the way!
Optimize more. Connector is fairly quick but it can be quicker. Also need to think about how to utilize smart pointers. C structs make it a pain...
Read up and practice with how to make a doxygen project for better communication. Communication is always key in the programming world!

That's all I can think of for now. If anyone has any comments and/or questions, fill free to send me an e-mail at:
garryrice4383@gmail.com
I take criticism very well and I'm always open to new view points and new ideas. I hope someone out there finds this useful!

Please go here to read up on Oracles Preface and Legal notice for thier MySQL Connectors:
https://dev.mysql.com/doc/connector-c/en/preface.html

Please go here to read up on the detail for Boost's software license:
https://www.boost.org/users/license.html

This software either utilizes or has been designed to use one or both libraries above. As such, I consider this project open source
and it must adhere to the open source policies of both Oracle and Boost. Read the README at the root of this repository for all the
warnings and such.
