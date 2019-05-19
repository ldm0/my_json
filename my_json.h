#pragma once

#ifndef _MY_JSON_H_
#define _MY_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include"my_json_type.h"

enum MY_JSON_STATE {
	MY_JSON_STATE_ERROR = -1,
	MY_JSON_STATE_OK,
	NUM_MY_JSON_STATE
};

extern void my_json_set_allocator(
	void *(*_malloc)(int),
	void (*_free)(void *));

// return the real length of the json file, if failed return -1;
extern int my_json_parse(struct my_json_value * const root, const char * const json);
extern int my_json_write(const struct my_json_value * const root, char * const json, const int json_length);
extern void my_json_free_document(struct my_json_value * const root);

extern int my_json_object_set_value(struct my_json_object * const object, const char *_key, const struct my_json_value *_value);
extern int my_json_object_get_value(const struct my_json_object * const object, const char *_key, struct my_json_value *_value);
extern int my_json_string_duplicate(struct my_json_string *const destination, const struct my_json_string *const source);

#ifdef __cplusplus
}
#endif

#endif
