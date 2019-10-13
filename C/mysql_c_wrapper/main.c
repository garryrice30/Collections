/**
 * @file main.cpp
 * @author Garry Rice (Arprog)
 * @date 10/12/2019
 * @brief MySQL C Wrapper driver test file.
 *
 * This is the driver file to demonstrate/test how MySQL C Wrapper works.
 */

#include <stdio.h>
#include <stdlib.h>
#include "connector.h"

/*
 **************Note***********
 * I'm sure in C there's some macro fun I could do to make things easier,
 * but I'm trying to keep the code as friendly as possible. Check notes
 * through out to see the hardships I had.
 */

int main(void)
{
    //Declare the Connector for use.
    Connector m;
    //Check if it initializes.
    if(!Connector_init(&m,NULL))
    {
        //Reporting error via stdout.
        printf("Error: %s\n",m.error);
    }
    else
    {
        //Check if it connects to target.
        if(!Connector_connect(&m,"localhost","root","test","testdb",3306,0,0))
        {
            //Reporting error via stdout.
            printf("Error: %s\n",m.error);
        }
        else
        {
            //Check if query executes.
            if(!Connector_query(&m,"select * from Cars"))
            {
                //Reporting error via stdout.
                printf("Error: %s\n",m.error);
            }
            else
            {
                //Traverse fields to retrieve field names.
                for(int i = 0; i < m.num_fields; i++)
                {
                    //Print names found.
                    printf("%s | ",m.field[i].name);
                }
                //Here to reduce clutter.
                puts("");

                /*
                 *********Note**********
                 * This was the biggest portion I had trouble with.
                 * I tried my best to store the data in something like
                 * a void* so data can be converted when needed.
                 * I finally punted and left it up to the end user
                 * to use some MySQL Connector C functionality to retrieve
                 * their data. Good news is, that leaves more flexibility and
                 * it's easier on me. Bad news, it's SLOPPY!
                 * Maybe I'll figure something else out in the future.
                 */

                 //Use MYSQL_ROW struct to use mysql_fetch_row to traverse
                 //the rows for data.
                while((m.row = mysql_fetch_row(m.result)))
                {
                    //Use a for loop, linked with the number of fields
                    //(AKA column count).
                    for(size_t i = 0; i < m.num_fields; i++)
                    {
                        //If the row is NULL, only print NULL.
                        if(m.row[i] == NULL)
                        {
                            printf("NULL | ");
                        }
                        //Otherwise, print the data to stdout.
                        else
                        {
                            printf("%s | ", m.row[i]);
                        }
                    }
                    //Clearing out the clutter.
                    puts("");
                }
                //Freeing up resources.
                Connector_destroy(&m);
            }
        }
    }
    //End of Program
    return 0;
}
