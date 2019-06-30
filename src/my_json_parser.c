// Principle: 
// The json pair memory allocation is done before calling following parsing functions
// But the type info is added during the parsing function 
// Because in like number parsing function, before number processing, the type is not 
// remove white space before calling each parsing function
#include"my_json.h"
#include"my_json_type.h"
#include<stdio.h>   // for stderr and fprintf
#include<stdlib.h>	// for memory allocation
#include<string.h>  // for memset

#define DEBUG
#define STRICT

#ifdef DEBUG
	// ptr is an global integer defined in the parser which points to where json is currently parsed
#define assert(equality) \
        do {\
            if (!(equality)) {\
                fprintf(stderr, "PARSE ERROR: %s:%d\n", __FILE__, __LINE__);\
            }\
        } while (0)
#else 
#define assert(equation)
#endif

// the json document is strict like:
// Not supports trailing comma 
// Not supports '+'
// Not support digits after 0 in number parsing
// Must have digits after '.' is number parsing

#ifndef STRICT
#define NOT_STRICT
#endif



// the pointer of where the string is currently parsing
static int ptr;

static void reset_ptr(void)
{
	ptr = 0;
}

static int get_ptr(void)
{
	return ptr;
}

static int is_digit(char c)
{
	return (c == '0'
			|| c == '1'
			|| c == '2'
			|| c == '3'
			|| c == '4'
			|| c == '5'
			|| c == '6'
			|| c == '7'
			|| c == '8'
			|| c == '9');
}

static inline void ws_remove(const char * const json)
{
	while (json[ptr] == (char)0x20		// space
		   || json[ptr] == (char)0x9	// tab
		   || json[ptr] == (char)0xA	// CR
		   || json[ptr] == (char)0xD)	// LF
		++ptr;
}

// parse_null parse_true parse_false does nothing but check if string format is right 
enum MY_JSON_STATE parse_null(const char * const json)
{
	if (json[ptr] != 'n')
		goto error;
	++ptr;
	if (json[ptr] != 'u')
		goto error;
	++ptr;
	if (json[ptr] != 'l')
		goto error;
	++ptr;
	if (json[ptr] != 'l')
		goto error;
	++ptr;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE parse_true(const char * const json)
{
	if (json[ptr] != 't')
		goto error;
	++ptr;
	if (json[ptr] != 'r')
		goto error;
	++ptr;
	if (json[ptr] != 'u')
		goto error;
	++ptr;
	if (json[ptr] != 'e')
		goto error;
	++ptr;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE parse_false(const char * const json)
{
	if (json[ptr] != 'f')
		goto error;
	++ptr;
	if (json[ptr] != 'a')
		goto error;
	++ptr;
	if (json[ptr] != 'l')
		goto error;
	++ptr;
	if (json[ptr] != 's')
		goto error;
	++ptr;
	if (json[ptr] != 'e')
		goto error;
	++ptr;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE parse_string(struct my_json_string * const string, const char * const json)
{
	if (json[ptr] != '"')
		goto error;
	++ptr;

	while (json[ptr] != '"') {
		if (json[ptr] == '\0')
			goto error;
		if (json[ptr] == '\\') {
			++ptr;
			// process escaped words
			// unfinished
		} else {
			if (my_json_string_pushback(string, json[ptr]) == -1)
				goto error;
			++ptr;
		}
	}

	// pass the '\"'
	++ptr;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

// parsing number is the only exception that type info cannot be decided with the first word(int or double)
// so the function is designed to change type info within it
static enum MY_JSON_STATE parse_number(struct my_json_value * const value, const char * const json)
{
	assert(json[ptr] == '-' || is_digit(json[ptr]));
	// for overflow test and the possibility of double result, use the large int
	int sign;   // -1 for negative, 1 for positive
	int frac_length = 0;
	int exp_part = 0;
	unsigned long long int int_part = 0;
	unsigned long long int frac_part = 0;
	if (json[ptr] == '-') {
		sign = -1;
		++ptr;
	}
#ifdef NOT_STRICT
	// the plus sign is not allowed 
	else if (json[ptr] == '+') {
		sign = 1;
		++ptr;
	}
#endif
	else {
		// No sign
		sign = 1;
	}

	ws_remove(json);

	// Start parsing the integer part
	if (json[ptr] == '0') {
		int_part = 0;
		++ptr;
#ifdef STRICT
		// the integer part is not allowed after zero
		if (is_digit(json[ptr]))
			goto error;
#endif
	} else if (is_digit(json[ptr])) {
		// if the json[ptr] is a digit except '0'
		while (is_digit(json[ptr])) {
			int_part = int_part * 10 + (json[ptr] - '0');
			++ptr;
		}
	}
#ifdef STRICT
	else {
		// must be a digit after the sign parsing 
		goto error;
	}
#endif

	// Start parsing the fractional part
	if (json[ptr] == '.') {
		++ptr;
		while (is_digit(json[ptr])) {
			frac_part = frac_part * 10 + (json[ptr] - '0');
			++frac_length;
			++ptr;
		}
#ifdef STRICT
		// According to the json document, there must be 1 or more digits after dot
		assert(frac_length != 0);
		if (!frac_length)
			goto error;
#endif
	}

	// exp part
	if (json[ptr] == 'e' || json[ptr] == 'E') {
		// leading '0' and leading '+' is permitted in the exp part
		// no where large exponential will be produced, so avoid complex checking.(lazy) :P
		++ptr;
		int exp_sign;
		if (json[ptr] == '-') {
			exp_sign = -1;
			++ptr;
		} else if (json[ptr] == '+') {
			exp_sign = 1;
			++ptr;
		} else if (!is_digit(json[ptr])) {
			// there must be a digit in the exponential part
			goto error;
		} else {
			exp_sign = 1;
		}

		while (is_digit(json[ptr])) {
			exp_part = exp_part * 10 + (json[ptr] - '0');
			++ptr;
		}
		// while avoid 
		exp_part *= exp_sign;
	}

	//  When frac_part or the exp part is present, the value must be double 
	if (exp_part != 0 || frac_length != 0) {
		value->type = MY_JSON_TYPE_DOUBLE;
		double result = (double)int_part;
		for (int i = 0; i < frac_length; ++i)
			result *= 10.;
		result += frac_part;
		int tmp_pow = exp_part - frac_length;
		if (tmp_pow >= 0) {
			while (tmp_pow > 0) {
				result *= 10.;
				--tmp_pow;
			}
		} else {
			while (tmp_pow < 0) {
				result *= 0.1;
				++tmp_pow;
			}
		}
		value->value.val_double = sign * result;
	} else {
		value->type = MY_JSON_TYPE_INT;
		// at this time thee result is the abs of result(without boundary check)
		value->value.val_int = sign * (long long int)int_part;
	}

	ws_remove(json);
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE parse_value(struct my_json_value * const value, const char * const json);

static enum MY_JSON_STATE parse_array(struct my_json_array * const _array, const char * const json)
{
	if (json[ptr] != '[')
		goto error;
	// pass the leading '['
	++ptr;

	ws_remove(json);

	struct my_json_value value;
	// check type
	while (json[ptr] != ']') {
		my_json_value_init(&value);

		if (parse_value(&value, json) == MY_JSON_STATE_ERROR)
			goto error;
		ws_remove(json);

		my_json_array_pushback(_array, &value);

		// If not comma and not square bracket, the format is error
		if (json[ptr] != ',') {
			if (json[ptr] != ']') {
				goto error;
			} else {
				break;
			}
		} else {
			++ptr;
			ws_remove(json);
#ifdef STRICT
			// According to the document, trailing comma is not permitted
			if (json[ptr] == ']')
				goto error;
#endif
		}
	}
	assert(json[ptr] == ']');
	// pass the ']'
	++ptr;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE parse_object(struct my_json_object * const object, const char * const json)
{
	assert(json[ptr] == '{');
	if (json[ptr] != '{')
		goto error;
	// pass the '{'
	++ptr;

	struct my_json_pair pair;

	ws_remove(json);
	while (json[ptr] != '}') {
		// parse the key of the pair
		my_json_pair_init(&pair);
		if (parse_string(&(pair.key), json) == MY_JSON_STATE_ERROR)
			goto error;

		ws_remove(json);
		if (json[ptr] != ':')
			goto error;
		++ptr;
		ws_remove(json);

		// parse the value of the pair
		if (parse_value(&(pair.value), json) == MY_JSON_STATE_ERROR) {
			int a = ptr;
			goto error;
		}
		ws_remove(json);

		if (my_json_object_pushback(object, &pair) != 0)
			goto error;

		// If not comma and not curly bracket, the format is error
		if (json[ptr] != ',') {
			if (json[ptr] != '}') {
				goto error;
			} else {
				break;
			}
		} else {
			++ptr;
			ws_remove(json);
#ifdef STRICT
			// According to the document, trailing comma is not permitted
			if (json[ptr] == '}')
				goto error;
#endif
		}
	}
	assert(json[ptr] == '}');
	++ptr;

	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE parse_value(struct my_json_value * const value, const char * const json)
{
	if (json[ptr] == 'n') {
		value->type = MY_JSON_TYPE_NULL;
		if (parse_null(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == 't') {
		value->type = MY_JSON_TYPE_TRUE;
		if (parse_true(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == 'f') {
		value->type = MY_JSON_TYPE_FALSE;
		if (parse_false(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '{') {
		value->type = MY_JSON_TYPE_OBJECT;
		my_json_object_init(&(value->value.val_object));
		if (parse_object(&(value->value.val_object), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '[') {
		value->type = MY_JSON_TYPE_ARRAY;
		my_json_array_init(&(value->value.val_array));
		if (parse_array(&(value->value.val_array), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '"') {
		value->type = MY_JSON_TYPE_STRING;
		my_json_string_init(&(value->value.val_string));
		if (parse_string(&(value->value.val_string), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '-' || is_digit(json[ptr])) {
		// cannot decide whether the type is int or double
		if (parse_number(value, json) == MY_JSON_STATE_ERROR)
			goto error;
	} else {
		goto error;
	}

	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}


int my_json_parse(struct my_json_value *const root, const char * const json)
{
	ptr = 0;
	ws_remove(json);
	if (json[ptr] == '[') {
		root->type = MY_JSON_TYPE_ARRAY;
		my_json_array_init(&(root->value.val_array));
		if (parse_array(&(root->value.val_array), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '{') {
		root->type = MY_JSON_TYPE_OBJECT;
		my_json_object_init(&(root->value.val_object));
		if (parse_object(&(root->value.val_object), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else {
		goto error;
	}
	ptr = 0;
	return 0;
error:
	return -1;
}
