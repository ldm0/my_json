#pragma once

#ifndef _MY_JSON_H_
#define _MY_JSON_H_

enum MY_JSON_TYPE {
	JSON_TYPE_UNKNOWN = -1,
	JSON_TYPE_NULL,
	JSON_TYPE_TRUE,
	JSON_TYPE_FALSE,
	JSON_TYPE_OBJECT,
	JSON_TYPE_ARRAY,
	JSON_TYPE_INT,
	JSON_TYPE_DOUBLE,
	JSON_TYPE_STRING,
	NUM_JSON_TYPE
};

enum MY_JSON_STATE {
	MY_JSON_STATE_ERROR = -1,
	MY_JSON_STATE_OK,
	NUM_MY_JSON_STATE
};

struct my_json_array_node;
struct my_json_pair;

struct my_json_string {
	int length;
	int capacity;
	char *c_str;
};

struct my_json_array {
	struct my_json_array_node *root;
};

struct my_json_object {
	struct my_json_pair *root;
};

union my_json_value {
	struct my_json_string val_string;
	struct my_json_array val_array;
	struct my_json_object val_object;
	double val_double;
	// Attention! the integer is actually pretty long 
	long long int val_int;
};

struct my_json_array_node {
	struct my_json_array_node *next;
	enum MY_JSON_TYPE type;
	union my_json_value value;
};

struct my_json_pair {
	struct my_json_pair *next;
	struct my_json_string key;
	enum MY_JSON_TYPE type;
	union my_json_value value;
};

extern enum MY_JSON_STATE my_json_parse(struct my_json_pair * const root, const char * const json);
extern enum MY_JSON_STATE my_json_write(const struct my_json_pair * const root, char * const json, const unsigned long long int json_length);
extern void my_json_free(struct my_json_pair * const root);

#endif
