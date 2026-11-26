/**
 * @file main.cpp
 * @author Garry Rice (Arprog)
 * @date 10/12/2019
 * @brief MySQL C Wrapper driver test file.
 *
 * This is the driver file to demonstrate/test how MySQL C Wrapper works.
 * Point it at your own server, user, password and database before running it.
 */

#include <stdio.h>
#include <stdlib.h>
#include "connector.h"

/**
 * Prints one row of a result set.
 * @param row Row handed over by Connector_call_rows.
 * @param num_fields Number of fields in that row.
 * @param user Optional label printed in front of the row.
 * @return Always true, so the call keeps reading.
 */
static bool print_row(MYSQL_ROW row, unsigned num_fields, void* user)
{
    const char* label = (const char*)user;
    if(label != NULL)
    {
        printf("%s: ", label);
    }
    for(unsigned i = 0; i < num_fields; i++)
    {
        printf("%s | ", row[i] ? row[i] : "NULL");
    }
    puts("");
    return true;
}

/**
 * Prints the field names and every row of the result set the Connector holds.
 * @param m Connector this function belongs to.
 */
static void print_table(Connector* m)
{
    //Traverse fields to retrieve field names.
    for(unsigned i = 0; i < m->num_fields; i++)
    {
        printf("%s | ", m->field[i].name);
    }
    //Here to reduce clutter.
    puts("");

    //Use MYSQL_ROW struct to use mysql_fetch_row to traverse the rows for data.
    while(Connector_next(m) != NULL)
    {
        print_row(m->row, m->num_fields, NULL);
    }
}

int main(void)
{
    //Declare the Connector for use.
    Connector m;
    //Check if it initializes.
    if(!Connector_init(&m,NULL))
    {
        printf("Error: %s\n",m.error);
        return 1;
    }

    //Check if it connects to target.
    if(!Connector_connect(&m,"localhost","root","test","testdb",3306,NULL,0))
    {
        printf("Error: %s\n",m.error);
        Connector_destroy(&m);
        return 1;
    }

    //A plain query, written out by hand.
    if(Connector_query(&m,"select * from Cars"))
    {
        print_table(&m);
    }
    else
    {
        printf("Error: %s\n",m.error);
    }

    //The same query, with the values passed as arguments. The string is
    //quoted and escaped for us, so a name containing a quote cannot break it.
    if(Connector_queryf(&m,"select * from Cars where make = %s and year > %d","Ford",2000))
    {
        print_table(&m);
    }
    else
    {
        printf("Error: %s\n",m.error);
    }

    //Stored procedure. Every row of every result set goes to print_row, which
    //is what makes a procedure that returns several result sets usable.
    if(Connector_call_rows(&m,"cars_by_make","'Ford'",print_row,(void*)"cars_by_make"))
    {
        printf("Result sets: %u\n",m.num_result_sets);
    }
    else
    {
        printf("Error: %s\n",m.error);
    }

    //Stored procedure without a callback: the rows are read and thrown away,
    //which still leaves the connection ready for the next statement.
    if(!Connector_call(&m,"cars_by_make","'Ford'"))
    {
        printf("Error: %s\n",m.error);
    }

    //Stored function. The single value it returns lands in m.scalar.
    if(Connector_call_function(&m,"count_cars",NULL))
    {
        printf("count_cars() = %s\n",m.scalar_is_null ? "NULL" : m.scalar);
    }
    else
    {
        printf("Error: %s\n",m.error);
    }

    //Freeing up resources.
    Connector_destroy(&m);

    //End of Program
    return 0;
}
