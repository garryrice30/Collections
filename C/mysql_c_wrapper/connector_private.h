/**
 * @file connector_private.h
 * @author Garry Rice (Arprog)
 * @date 10/12/2019
 * @brief Helpers shared by the MySQL C Wrapper source files.
 *
 * Nothing in here is part of the public interface. Include connector.h for that.
 */

#ifndef connector_private_h
#define connector_private_h

#include "connector.h"

#define CONNECTOR_NO_ERROR "No error to report."

/**
 * Length of a string, as a size_t.
 * @param str String to measure.
 * @return Its length.
 */
size_t Connector_getStrSize(const char* str);

/**
 * Replaces the error message stored in the Connector.
 * @param m Connector this function belongs to.
 * @param message Message to store, NULL is treated as an empty message.
 */
void Connector_set_error(Connector* m, const char* message);

/**
 * Clears the error state and resets the error message.
 * @param m Connector this function belongs to.
 */
void Connector_clear_error(Connector* m);

/**
 * Replaces the scalar value with a copy of value.
 * @param m Connector this function belongs to.
 * @param value Value to copy, NULL stores NULL.
 * @return The copy was allocated or not.
 */
bool Connector_store_scalar(Connector* m, const char* value);

/**
 * Releases the result set currently held by the Connector, if any.
 * @param m Connector this function belongs to.
 */
void Connector_release_result(Connector* m);

#endif
