#pragma once

#ifndef _MY_JSON_H_
#define _MY_JSON_H_

enum MY_JSON_TYPE {
	MY_JSON_TYPE_UNKNOWN = -1,
	MY_JSON_TYPE_NULL,
	MY_JSON_TYPE_TRUE,
	MY_JSON_TYPE_FALSE,
	MY_JSON_TYPE_OBJECT,
	MY_JSON_TYPE_ARRAY,
	MY_JSON_TYPE_INT,
	MY_JSON_TYPE_DOUBLE,
	MY_JSON_TYPE_STRING,
	NUM_JSON_TYPE
};

enum MY_JSON_STATE {
	MY_JSON_STATE_ERROR = -1,
	MY_JSON_STATE_OK,
	NUM_MY_JSON_STATE
};

struct my_json_string {
	int length;
	int capacity;
	char *c_str;
};

union my_json_value_value;

struct my_json_value {
	enum MY_JSON_TYPE type;
	union my_json_value_value value;
};

struct my_json_pair {
	struct my_json_string key;
	struct my_json_value value;
};

struct my_json_array {
	int length;
	int capacity;
	struct my_json_value *values;
};

struct my_json_object {
	int length; 
	int capacity;
	struct my_json_pair *pairs;
};

union my_json_value_value {
	struct my_json_string val_string;
	struct my_json_array val_array;
	struct my_json_object val_object;
	double val_double;
	// Attention! the integer is actually pretty long
	// for relatively big number storage like telephone number
	long long int val_int;
};


// return the real length of the json file
extern int my_json_parse(struct my_json_value * const root, const char * const json);
extern int my_json_write(const struct my_json_value * const root, char * const json, const int json_length);
extern void my_json_free(struct my_json_value * const root);

#endif
