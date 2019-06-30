#pragma once

#ifndef _MY_JSON_TYPE_H_
#define _MY_JSON_TYPE_H_

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

struct my_json_string {
	int length;
	int capacity;
	char *c_str;
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

struct my_json_value {
	enum MY_JSON_TYPE type;
	union my_json_value_value value;
};

struct my_json_pair {
	struct my_json_string key;
	struct my_json_value value;
};

extern void my_json_set_allocator(
	void *(*_malloc)(int),
	void (*_free)(void *));

extern void my_json_object_init(struct my_json_object * const object);
extern void my_json_value_init(struct my_json_value *const value);
extern void my_json_array_init(struct my_json_array * const array);
extern void my_json_string_init(struct my_json_string * const string);
extern void my_json_pair_init(struct my_json_pair *const pair);
extern int my_json_object_pushback(struct my_json_object *const object, const struct my_json_pair *pair);
extern int my_json_array_pushback(struct my_json_array *const array, const struct my_json_value *value);
extern int my_json_string_pushback(struct my_json_string *const string, char new_char);
extern void my_json_string_free(struct my_json_string *string);
extern void my_json_array_free(struct my_json_array *array);
extern void my_json_pair_free(struct my_json_pair *pair);
extern void my_json_object_free(struct my_json_object *object);
extern void my_json_value_free(struct my_json_value *value);
extern void my_json_free_document(struct my_json_value * const root);
extern int my_json_object_set_value(struct my_json_object * const object, const char *_key, const struct my_json_value *_value);
extern int my_json_object_get_value(const struct my_json_object * const object, const char *_key, struct my_json_value *_value);
extern int my_json_string_duplicate(struct my_json_string *const destination, const struct my_json_string *const source);

#endif