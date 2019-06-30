// development sticky notes: Don't forget to add return ok to other functions.
#include"my_json.h"
#include<stdio.h>

// points to the place where the program outputs to.
static unsigned ptr = 0;
static unsigned buffer_length = 0;

enum MY_JSON_STATE c_str_write(char * const c_str, const char * const input)
{
	for (int i = 0; input[i]; ++i) {
		if (ptr >= buffer_length - 1)
			return MY_JSON_STATE_ERROR;
		c_str[ptr++] = input[i];
	}
	return MY_JSON_STATE_OK;
}

static enum MY_JSON_STATE write_null(char * const json)
{
	if (c_str_write(json, "null") == MY_JSON_STATE_ERROR)
		goto error;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE write_true(char * const json)
{
	if (c_str_write(json, "true") == MY_JSON_STATE_ERROR)
		goto error;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE write_false(char * const json)
{
	if (c_str_write(json, "false") == MY_JSON_STATE_ERROR)
		goto error;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE write_int(char * const json, const long long int value)
{
	// 19 is the largest size of long long int (contains the minus symbol)
	char integer[20] = {0};
	snprintf(integer, 20, "%lld", value);
	if (c_str_write(json, integer) == MY_JSON_STATE_ERROR)
		goto error;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE write_double(char * const json, const double value)
{
	// 319 is the largest size of long long int (contains the minus symbol)
	char doublef[320] = {0};
	snprintf(doublef, 320, "%lf", value);
	if (c_str_write(json, doublef) == MY_JSON_STATE_ERROR)
		goto error;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE write_string(char * const json, const struct my_json_string *string)
{
	if (c_str_write(json, "\"") == MY_JSON_STATE_ERROR)
		goto error;
	if (c_str_write(json, string->c_str) == MY_JSON_STATE_ERROR)
		goto error;
	if (c_str_write(json, "\"") == MY_JSON_STATE_ERROR)
		goto error;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE write_value(char * const json, const struct my_json_value *value);

static enum MY_JSON_STATE write_object(char * const json, const struct my_json_object * const object)
{
	int length = object->length;
	if (c_str_write(json, "{") == MY_JSON_STATE_ERROR)
		goto error;
	if (length) {
		for (int i = 0;;) {
			if (write_string(json, &(object->pairs[i].key)) == MY_JSON_STATE_ERROR)
				goto error;
			if (c_str_write(json, " : ") == MY_JSON_STATE_ERROR)
				goto error;
			if (write_value(json, &(object->pairs[i].value)) == MY_JSON_STATE_ERROR)
				goto error;

			if (++i >= length)
				break;

			if (c_str_write(json, ", ") == MY_JSON_STATE_ERROR)
				goto error;
		}
	}
	if (c_str_write(json, "}") == MY_JSON_STATE_ERROR)
		goto error;

	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}
static enum MY_JSON_STATE write_array(char * const json, const struct my_json_array *array)
{
	if (c_str_write(json, "[") == MY_JSON_STATE_ERROR)
		goto error;

	int length = array->length;
	struct my_json_value *values = array->values;
	if (length) {
		for (int i = 0; ;) {
			if (write_value(json, &values[i]) == MY_JSON_STATE_ERROR)
				goto error;

			// trailing comma is not permitted
			if (++i >= length)
				break;

			if (c_str_write(json, ", ") == MY_JSON_STATE_ERROR)
				goto error;
		}
	}
	if (c_str_write(json, "]") == MY_JSON_STATE_ERROR)
		goto error;
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

static enum MY_JSON_STATE write_value(char * const json, const struct my_json_value *value)
{
	enum MY_JSON_TYPE type = value->type;
	if (type == MY_JSON_TYPE_NULL) {
		if (write_null(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (type == MY_JSON_TYPE_TRUE) {
		if (write_true(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (type == MY_JSON_TYPE_FALSE) {
		if (write_false(json) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (type == MY_JSON_TYPE_OBJECT) {
		if (write_object(json, &(value->value.val_object)) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (type == MY_JSON_TYPE_ARRAY) {
		if (write_array(json, &(value->value.val_array)) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (type == MY_JSON_TYPE_INT) {
		if (write_int(json, value->value.val_int) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (type == MY_JSON_TYPE_DOUBLE) {
		if (write_double(json, value->value.val_double) == MY_JSON_STATE_ERROR)
			goto error;
	} else if (type == MY_JSON_TYPE_STRING) {
		if (write_string(json, &(value->value.val_string)) == MY_JSON_STATE_ERROR)
			goto error;
	} else {
		goto error;
	}
	return MY_JSON_STATE_OK;
error:
	return MY_JSON_STATE_ERROR;
}

int my_json_write(const struct my_json_value * const root, char * const json, const int json_length)
{
	ptr = 0;
	buffer_length = json_length;

	if (write_value(json, root) == MY_JSON_STATE_ERROR)
		goto error;

	// -1 for the terminated zero
	if (ptr >= buffer_length - 1)
		goto error;
	json[ptr++] = (char)0;
	return ptr;
error:
	return -1;
}
