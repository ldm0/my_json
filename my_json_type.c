#include "my_json_type.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define DEFAULT_STRING_CAPACITY 16
#define DEFAULT_OBJECT_CAPACITY 4
#define DEFAULT_ARRAY_CAPACITY 4

static void *(*my_json_malloc)(int) = malloc;
static void (*my_json_free)(void *) = free;

void my_json_set_allocator(void *(*_malloc)(int), void (*_free)(void *))
{
	my_json_malloc = _malloc;
	my_json_free = _free;
}

void my_json_object_init(struct my_json_object * const object)
{
	memset(object, 0, sizeof(struct my_json_object));
}

void my_json_value_init(struct my_json_value *const value)
{
	memset(value, 0, sizeof(struct my_json_value));
}

void my_json_array_init(struct my_json_array * const array)
{
	memset(array, 0, sizeof(struct my_json_array));
}

void my_json_string_init(struct my_json_string * const string)
{
	memset(string, 0, sizeof(struct my_json_string));
}

void my_json_pair_init(struct my_json_pair *const pair)
{
	memset(pair, 0, sizeof(struct my_json_pair));
}

int my_json_object_pushback(struct my_json_object *const object, const struct my_json_pair *pair)
{
	if (object->capacity == 0) {
		object->capacity = DEFAULT_OBJECT_CAPACITY;
		object->pairs = (struct my_json_pair*)my_json_malloc(DEFAULT_OBJECT_CAPACITY * sizeof(struct my_json_pair));
		if (!(object->pairs))
			return -1;
	}

	//  (capacity - 1) for the '\0'
	if (object->length >= object->capacity - 1) {
		object->capacity += object->capacity >> 1;
		struct my_json_pair* new_pairs = (struct my_json_pair*)my_json_malloc(object->capacity * sizeof(struct my_json_pair));
		if (!new_pairs)
			return -1;
		for (int i = 0; i < object->length; ++i)
			new_pairs[i] = object->pairs[i];
		my_json_free(object->pairs);
		object->pairs = new_pairs;
	}

	object->pairs[(object->length)++] = *pair;
	return 0;
}

int my_json_array_pushback(struct my_json_array *const array, const struct my_json_value *value)
{
	if (array->capacity == 0) {
		array->capacity = DEFAULT_ARRAY_CAPACITY;
		array->values = (struct my_json_value*)my_json_malloc(DEFAULT_ARRAY_CAPACITY * sizeof(struct my_json_value));
		if (!(array->values))
			return -1;
	}

	//  -1 for the '\0'
	if (array->length >= array->capacity - 1) {
		array->capacity += array->capacity >> 1;
		struct my_json_value* new_values = (struct my_json_value*)my_json_malloc(array->capacity * sizeof(struct my_json_value));
		if (!new_values)
			return -1;
		for (int i = 0; i < array->length; ++i)
			new_values[i] = array->values[i];
		my_json_free(array->values);
		array->values = new_values;
	}

	array->values[(array->length)++] = *value;
	return 0;
}

int my_json_string_pushback(struct my_json_string *const string, char new_char)
{
	if (string->capacity == 0) {
		string->capacity = DEFAULT_STRING_CAPACITY;
		string->c_str = (char *)my_json_malloc(DEFAULT_STRING_CAPACITY * sizeof(char));
		if (!(string->c_str))
			return -1;
	}

	//  -1 for the '\0'
	if (string->length >= string->capacity - 1) {
		string->capacity += string->capacity >> 1;
		char *new_c_str = (char *)my_json_malloc(string->capacity * sizeof(char));
		if (!new_c_str)
			return -1;
		for (int i = 0; i < string->length; ++i)
			new_c_str[i] = string->c_str[i];
		my_json_free(string->c_str);
		string->c_str = new_c_str;
	}

	string->c_str[(string->length)++] = new_char;
	string->c_str[(string->length)] = '\0';
	return 0;
}

void my_json_value_free(struct my_json_value *value);

void my_json_string_free(struct my_json_string *string)
{
	my_json_free(string->c_str);
	memset(string, 0, sizeof(struct my_json_string));
}

void my_json_array_free(struct my_json_array *array)
{
	int length = array->length;
	for (int i = 0; i < length; ++i)
		my_json_value_free(&(array->values[i]));
	my_json_free(array->values);
	memset(array, 0, sizeof(struct my_json_array));
}

void my_json_pair_free(struct my_json_pair *pair)
{
	my_json_string_free(&(pair->key));
	my_json_value_free(&(pair->value));
	memset(pair, 0, sizeof(struct my_json_pair));
}

void my_json_object_free(struct my_json_object *object)
{
	int length = object->length;
	for (int i = 0; i < length; ++i)
		my_json_pair_free(&(object->pairs[i]));
	my_json_free(object->pairs);
	memset(object, 0, sizeof(struct my_json_object));
}

void my_json_value_free(struct my_json_value *value)
{
	switch (value->type) {
	case MY_JSON_TYPE_OBJECT:
		my_json_object_free(&(value->value.val_object));
		break;
	case MY_JSON_TYPE_ARRAY:
		my_json_array_free(&(value->value.val_array));
		break;
	case MY_JSON_TYPE_STRING:
		my_json_string_free(&(value->value.val_string));
		break;
	}
	memset(value, 0, sizeof(struct my_json_value));
}

void my_json_free_document(struct my_json_value * const root)
{
	my_json_value_free(root);
}

int my_json_object_set_value(struct my_json_object * const object, const char *_key, const struct my_json_value *_value)
{
	for (int i = 0; i < object->length; ++i) {
		if (strncmp(object->pairs[i].key.c_str, _key, object->pairs[i].key.length) == 0) {
			my_json_value_free(&(object->pairs[i].value));
			object->pairs[i].value = *_value;
			return 0;
		}
	}
	struct my_json_pair pair_add;
	my_json_pair_init(&pair_add);
	for (int i = 0; _key[i]; ++i)
		if (my_json_string_pushback(&pair_add.key, _key[i]) != 0)
			return -1;
	pair_add.value = *_value;
	return my_json_object_pushback(object, &pair_add);
}

int my_json_object_get_value(const struct my_json_object * const object, const char *_key, struct my_json_value *_value)
{
	for (int i = 0; i < object->length; ++i) {
		if (strncmp(object->pairs[i].key.c_str, _key, object->pairs[i].key.length) == 0) {
			*_value = object->pairs[i].value;
			return 0;
		}
	}
	return -1;
}

int my_json_string_duplicate(struct my_json_string * const destination, const struct my_json_string * const source)
{
	int source_length = source->length;
	int source_capacity = source->capacity;
	char *source_c_str = source->c_str;

	char *c_str_cpy = (char *)my_json_malloc(source_capacity * sizeof(char));
	if (!c_str_cpy)
		return -1;

	for (int i = 0; i < source_length; ++i)
		c_str_cpy[i] = source_c_str[i];

	destination->length = source_length;
	destination->capacity = source_capacity;
	destination->c_str = c_str_cpy;
	return 0;
}


