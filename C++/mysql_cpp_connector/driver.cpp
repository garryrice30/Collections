/**
 *
 * @file driver.cpp
 * @author Garry Rice
 * @date 10/12/2019
 * @brief Driver test file for MySQL CPP Connector
 */

#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
using std::cin;
#include <string>
using std::string;
#include <cstdlib>
// Uncomment this line to utilize boost::any if needed.
//#include <boost/any.hpp>
#include <any>
#include "connector.h"

int main()
{
    // Declaring Connector to be used.
    Connector con;
    // Always use those boolean methods! Always check for errors!
    if(!con.connect("localhost","root","test","testdb",3306,nullptr,0))
    {
        // Report error as output and exit program.
        cerr << "Error: " << con.getError() << endl;
        exit(1);
    }

    // String buffer to take in user input. Easier than using a single char.
    string buffer;

    // Starting a do-while loop so end-users can play as much as they want.
    do
    {
        // Stores the query to be processed.
        string myQuery;
        // Don't forget output as to not to lose the end-user.
        cout << "Type a query for MySQL to execute: ";
        // Take in the user information. Don't worry too much about that dangling newline character.
        getline(cin,myQuery);
        // Not only do we check the query for errors but we also need to pass the string using the c_str() method.
        // This is because the foundation of the object is pure C structures. They can only process c-strings.
        if(!con.query(myQuery.c_str()))
        {
            // Report errors to output.
            // Choosing not to exit to give users many chances to play.
            cerr << "Error: " << con.getError() << endl;
        }
        else
        {
            // Checking the statement passed into the query.
            // If you're unsure what a Definition Statement is, read this article:
            // https://onlineqa.com/types-of-statements-in-mysql/
            // The type of statement passed depends on the type of return we need to deal with.
            if(con.isDefinitionStatement())
            {
                // Nothing needs retrieved in a Definition Statement. Give the all clear and move on.
                cout << "Table(s) has been added/updated successfully!\n\n";
            }
            else
            {
                // At this stage, we must be dealing with a Manipulation Statement.
                // We need to retrieve the data for processing.
                for(const auto& i: con.getFieldNames())
                {
                    // Using range-based for loops are possible to use in data retrieval!
                    // This is because they are just std::vectors.
                    // All standard containers tends to support range-based for loops.
                    cout << i << " ";
                }
                // We're done retrieving the field names. Use std::endl to end the current line and flush the output buffer.
                cout << endl;

                // It is now time to retrieve the actual data.
                // This might look a little complicated, but it's not.
                // Again, Connector stores information in std::vectors. Utilize a range-based for loop.
                for(const auto& i: con.getData())
                {
                    // The data is contained inside of a 2 dimensional std::vector.
                    // The reason why is simple. To support multiple rows.
                    for(const auto& j: i)
                    {
                        // Uncomment this line to utilize boost::any if needed.
                        //cout << boost::any_cast<char*>(j) << " ";

                        // What is being utilized is the std::any object.
                        // Read up about them here:
                        // https://en.cppreference.com/w/cpp/utility/any
                        // Do NOT forget to use std::any_cast when dealing with std::any.
                        // Seriously, this is not optional. It has little to no idea how you want your data treated.
                        // I'm omitting it here, but this REALLY should be surrounded by try-catch blocks!
                        // std::any has the fairly decent chance of throwing a bad_any_cast exception. Thus my comment about the try-catch blocks!
                        cout << std::any_cast<char*>(j) << " ";
                    }
                    // Data has not been retrieved. End the line and flush the output buffer.
                    cout << endl;
                }
                // This is just here to give extra space so output isn't so cluttered up. Not important.
                cout << endl;
            }
        }
        // Output for the user to signify that they are in a continuous loop until they say otherwise.
        cout << "Would you like to execute another command?(Y|N): ";
        // Take in the user's choice.
        getline(cin,buffer);

      // Play the while loop based on the user's choice (buffer).
    } while(buffer != "n" && buffer != "N");

    // End of program. Return 0 is not needed in std c++17, but I'm old-school and I like it ;)
    return 0;
}
