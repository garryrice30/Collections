/**
 * @file connector_queryf.cpp
 * @author Garry Rice (Arprog)
 * @date 10/12/2019
 * @brief printf style query building for the MySQL C Wrapper.
 *
 * Values read out of the format are quoted and escaped with the server's own
 * escaping rules, so a value containing a quote cannot break the statement.
 */

#include "connector.h"
#include "connector_private.h"

/**
 * Growable string used to assemble a formatted query.
 */
typedef struct _connector_buffer
{
	char* data;
	size_t length;
	size_t capacity;
} ConnectorBuffer;

/**
 * Makes sure the buffer can hold length extra characters plus the terminator.
 * @param buffer Buffer to grow.
 * @param extra Number of characters about to be appended.
 * @return The buffer has room or not.
 */
static bool ConnectorBuffer_reserve(ConnectorBuffer* buffer, size_t extra)
{
	if(buffer->length + extra + 1 <= buffer->capacity)
	{
		return true;
	}
	size_t capacity = buffer->capacity ? buffer->capacity : 128;
	while(capacity < buffer->length + extra + 1)
	{
		capacity *= 2;
	}
	char* data = (char*)realloc(buffer->data, capacity);
	if(data == NULL)
	{
		return false;
	}
	buffer->data = data;
	buffer->capacity = capacity;
	return true;
}

static bool ConnectorBuffer_appendLen(ConnectorBuffer* buffer, const char* text, size_t length)
{
	if(!ConnectorBuffer_reserve(buffer, length))
	{
		return false;
	}
	memcpy(buffer->data + buffer->length, text, length);
	buffer->length += length;
	buffer->data[buffer->length] = '\0';
	return true;
}

static bool ConnectorBuffer_append(ConnectorBuffer* buffer, const char* text)
{
	return ConnectorBuffer_appendLen(buffer, text, Connector_getStrSize(text));
}

/**
 * Appends a value as a quoted, escaped SQL string.
 * @param m Connector this function belongs to.
 * @param buffer Buffer the value is appended to.
 * @param value Value to append, NULL appends the SQL keyword NULL.
 * @return The value was appended or not.
 */
static bool ConnectorBuffer_appendValue(Connector* m, ConnectorBuffer* buffer, const char* value)
{
	if(value == NULL)
	{
		return ConnectorBuffer_append(buffer, "NULL");
	}
	size_t length = Connector_getStrSize(value);
	char* escaped = (char*)malloc(length * 2 + 1);
	if(escaped == NULL)
	{
		return false;
	}
	unsigned long written = mysql_real_escape_string(m->con, escaped, value, (unsigned long)length);
	bool rval = ConnectorBuffer_append(buffer, "'")
		&& ConnectorBuffer_appendLen(buffer, escaped, (size_t)written)
		&& ConnectorBuffer_append(buffer, "'");
	free(escaped);
	return rval;
}

/**
 * Appends one number that has already been formatted.
 * @param buffer Buffer the number is appended to.
 * @param format printf format the value is read with.
 * @param ... The value.
 * @return The number was appended or not.
 */
static bool ConnectorBuffer_appendNumber(ConnectorBuffer* buffer, const char* format, ...)
{
	char text[64];
	va_list args;
	va_start(args, format);
	int written = vsnprintf(text, sizeof(text), format, args);
	va_end(args);
	if(written < 0 || (size_t)written >= sizeof(text))
	{
		return false;
	}
	return ConnectorBuffer_appendLen(buffer, text, (size_t)written);
}

bool Connector_vqueryf(Connector* m, const char* format, va_list args)
{
	if(m == NULL || format == NULL)
	{
		return false;
	}

	ConnectorBuffer buffer;
	buffer.data = NULL;
	buffer.length = 0;
	buffer.capacity = 0;

	bool rval = true;
	size_t i = 0;
	while(format[i] != '\0' && rval)
	{
		if(format[i] != '%')
		{
			rval = ConnectorBuffer_appendLen(&buffer, &format[i], 1);
			i++;
			continue;
		}

		i++;
		if(format[i] == '%')
		{
			rval = ConnectorBuffer_append(&buffer, "%");
			i++;
			continue;
		}

		int longs = 0;
		while(format[i] == 'l')
		{
			longs++;
			i++;
		}

		char specifier = format[i];
		if(specifier == 'd' || specifier == 'i' || specifier == 'u')
		{
			i++;
			if(specifier == 'd' || specifier == 'i')
			{
				if(longs == 0)
				{
					rval = ConnectorBuffer_appendNumber(&buffer, "%d", va_arg(args, int));
				}
				else if(longs == 1)
				{
					rval = ConnectorBuffer_appendNumber(&buffer, "%ld", va_arg(args, long));
				}
				else
				{
					rval = ConnectorBuffer_appendNumber(&buffer, "%lld", va_arg(args, long long));
				}
			}
			else
			{
				if(longs == 0)
				{
					rval = ConnectorBuffer_appendNumber(&buffer, "%u", va_arg(args, unsigned int));
				}
				else if(longs == 1)
				{
					rval = ConnectorBuffer_appendNumber(&buffer, "%lu", va_arg(args, unsigned long));
				}
				else
				{
					rval = ConnectorBuffer_appendNumber(&buffer, "%llu", va_arg(args, unsigned long long));
				}
			}
		}
		else if(specifier == 'f')
		{
			i++;
			rval = ConnectorBuffer_appendNumber(&buffer, "%.17g", va_arg(args, double));
		}
		else if(specifier == 's')
		{
			i++;
			rval = ConnectorBuffer_appendValue(m, &buffer, va_arg(args, const char*));
		}
		else
		{
			rval = false;
		}
	}

	if(!rval)
	{
		free(buffer.data);
		Connector_set_error(m, "Unsupported format specifier in Connector_queryf.");
		return false;
	}

	rval = Connector_query(m, buffer.data ? buffer.data : "");
	free(buffer.data);
	return rval;
}

bool Connector_queryf(Connector* m, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	bool rval = Connector_vqueryf(m, format, args);
	va_end(args);
	return rval;
}
