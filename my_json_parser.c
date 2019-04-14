// Principle: 
// The json pair memory allocation is done before calling following parsing functions
// But the type info is added during the parsing function 
// Because in like number parsing function, before number processing, the type is not 
// remove white space before calling each parsing function
#include"my_json.h"
#include<stdio.h>   // for stderr and fprintf
#include<stdlib.h>
#include<string.h>  // for memset

#define DEBUG
#define STRICT
#define DEFAULT_STRING_CAPACITY 32

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
static unsigned long long int ptr;

void reset_ptr(void)
{
	ptr = 0;
}

unsigned long long int get_ptr(void)
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

static inline void object_init(struct Object * const object)
{
	object->root = (struct Pair *)0;
}

static inline void array_node_init(struct Array_node *const array_node)
{
	memset(array_node, 0, sizeof(struct Array_node));
}

static inline void array_init(struct Array * const array)
{
	array->root = (struct Array_node*)0;
}

static inline void string_init(struct String * const string)
{
	memset(string, 0, sizeof(struct String));
}

static inline void pair_init(struct Pair *const pair)
{
	memset(pair, 0, sizeof(struct Pair));
}

static int string_pushback(struct String *const string, char new_char)
{
	if (string->capacity == 0) {
		string->capacity = DEFAULT_STRING_CAPACITY;
		string->c_str = (char *)malloc(DEFAULT_STRING_CAPACITY * sizeof(char));
		if (!(string->c_str))
			return -1;
	}

	//  -1 for the '\0'
	if (string->length >= string->capacity - 1) {
		string->capacity += string->capacity >> 1;
		char *new_c_str = (char *)malloc(string->capacity * sizeof(char));
		if (!new_c_str)
			return -1;
		for (int i = 0; i < string->length; ++i)
			new_c_str[i] = string->c_str[i];
		free(string->c_str);
		string->c_str = new_c_str;
	}

	string->c_str[(string->length)++] = new_char;
	string->c_str[(string->length)] = '\0';
	return 0;
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

static enum MY_JSON_STATE parse_string(struct String * const string, const char * const json)
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
			if (string_pushback(string, json[ptr]) == -1)
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

static enum MY_JSON_STATE parse_number(enum JSON_TYPE * type, union Value * value, const char * const json)
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
		*type = JSON_TYPE_DOUBLE;
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
		value->val_double = sign * result;
	} else {
		*type = JSON_TYPE_INT;
		// at this time thee result is the abs of result(without boundary check)
		value->val_int = sign * (long long int)int_part;
	}

	ws_remove(json);
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}


// Pre-declaration, for the function call parse_array and parse_object in parse_value
enum MY_JSON_STATE parse_array(struct Array * const _array, const char * const json);
enum MY_JSON_STATE parse_object(struct Object * const object, const char * const json);

static enum MY_JSON_STATE parse_value(enum JSON_TYPE * type, union Value * value, const char * const json)
{
	if (json[ptr] == 'n') {
		*type = JSON_TYPE_NULL;
		if (parse_null(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == 't') {
		*type = JSON_TYPE_TRUE;
		if (parse_true(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == 'f') {
		*type = JSON_TYPE_FALSE;
		if (parse_false(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '{') {
		*type = JSON_TYPE_OBJECT;
		value->val_object.root = (struct Pair *)malloc(sizeof(struct Pair));
		pair_init(value->val_object.root);
		if (parse_object(&(value->val_object), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '[') {
		*type = JSON_TYPE_ARRAY;
		array_init(&(value->val_array));
		if (parse_array(&(value->val_array), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '"') {
		*type = JSON_TYPE_STRING;
		string_init(&(value->val_string));
		if (parse_string(&(value->val_string), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '-' || is_digit(json[ptr])) {
		// only the number is not pre_set, because it maybe a kind of int or double
		if (parse_number(type, value, json) == MY_JSON_STATE_ERROR)
			goto error;
	} else {
		goto error;
	}

	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE parse_array(struct Array * const _array, const char * const json)
{
	if (json[ptr] != '[')
		goto error;
	// pass the leading '['
	++ptr;

	ws_remove(json);
	struct Array_node ** it = &(_array->root);
	// check type
	while (json[ptr] != ']') {
		*it = (struct Array_node *)malloc(sizeof(struct Array_node));
		if (!*it)
			goto error;
		array_node_init(*it);

		if (parse_value(&((*it)->type), &((*it)->value), json) == MY_JSON_STATE_ERROR)
			goto error;

		// move to next position 
		it = &((*it)->next);
		ws_remove(json);

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

static enum MY_JSON_STATE parse_object(struct Object *const object, const char * const json)
{
	assert(json[ptr] == '{');
	if (json[ptr] != '{')
		goto error;
	// pass the '{'
	++ptr;

	struct Pair ** it = &(object->root);

	ws_remove(json);
	while (json[ptr] != '}') {
		// parse the key of the pair
		*it = (struct Pair *)malloc(sizeof(struct Pair));
		if (!(*it))
			goto error;
		pair_init(*it);
		if (parse_string(&((*it)->key), json) == MY_JSON_STATE_ERROR)
			goto error;
		ws_remove(json);
		if (json[ptr] != ':')
			goto error;
		++ptr;
		ws_remove(json);

		// parse the value of the pair
		if (parse_value(&((*it)->type), &((*it)->value), json) == MY_JSON_STATE_ERROR)
			goto error;

		it = &((*it)->next);
		ws_remove(json);

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

	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

enum MY_JSON_STATE json_parse(struct Pair *const root, const char * const json)
{
	ptr = 0;
	ws_remove(json);
	if (json[ptr] == '[') {
		root->type = JSON_TYPE_ARRAY;
		array_init(&(root->value.val_array));
		if (parse_array(&(root->value.val_array), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (json[ptr] == '{') {
		root->type = JSON_TYPE_OBJECT;
		object_init(&(root->value.val_object));
		if (parse_object(&(root->value.val_object), json) == MY_JSON_STATE_ERROR)
			goto error;
	} else {
		goto error;
	}
	ptr = 0;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}
