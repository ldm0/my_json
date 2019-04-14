#ifndef _MY_JSON_H_
#define _MY_JSON_H_

enum JSON_TYPE {
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

struct Array_node;
struct Pair;

struct String {
	int length;
	int capacity;
	char *c_str;
};

struct Array {
	struct Array_node *root;
};

struct Object {
	struct Pair *root;
};

union Value {
	// the String may be too big, but this library is not performance sensitive. 
	// So don't cared.(lazy)
	struct String val_string;
	struct Array val_array;
	struct Object val_object;
	double val_double;
	// Attention! the int is actually pretty long 
	long long int val_int;
};

struct Array_node {
	enum JSON_TYPE type;
	struct Array_node *next;
	union Value value;
};

struct Pair {
	enum JSON_TYPE type;
	struct Pair *next;
	struct String key;
	union Value value;
};

extern enum MY_JSON_STATE json_parse(struct Pair * const root, const char * const json);
extern enum MY_JSON_STATE json_write(const struct Pair * const root, char * const json, const unsigned long long int json_length);

#endif
