#include"../src/my_json.h"
#include"gtest/gtest.h"

TEST(json_parse, 0)
{
    struct my_json_value root;
	ASSERT_EQ(0, my_json_parse(&root, "[666, 777, 888, 999, 111]"));
	ASSERT_EQ(MY_JSON_TYPE_ARRAY, root.type);
	ASSERT_EQ(5, root.value.val_array.length);
	for (int i = 0; i < 5; ++i)
		ASSERT_EQ(MY_JSON_TYPE_INT, root.value.val_array.values[i].type);
	ASSERT_EQ(666, root.value.val_array.values[0].value.val_int);
	ASSERT_EQ(777, root.value.val_array.values[1].value.val_int);
	ASSERT_EQ(888, root.value.val_array.values[2].value.val_int);
	ASSERT_EQ(999, root.value.val_array.values[3].value.val_int);
	ASSERT_EQ(111, root.value.val_array.values[4].value.val_int);
	my_json_free_document(&root);
}

TEST(json_parse, 1)
{
	struct my_json_value root;
	ASSERT_EQ(0, my_json_parse(&root, "[\"emm\", 10.1, true, 999, 111]"));
	ASSERT_EQ(MY_JSON_TYPE_ARRAY, root.type);
	ASSERT_EQ(5, root.value.val_array.length);
	ASSERT_EQ(MY_JSON_TYPE_STRING, root.value.val_array.values[0].type);
	ASSERT_STREQ(root.value.val_array.values[0].value.val_string.c_str, "emm");
	ASSERT_EQ(MY_JSON_TYPE_DOUBLE, root.value.val_array.values[1].type);
	ASSERT_EQ(MY_JSON_TYPE_TRUE, root.value.val_array.values[2].type);
	ASSERT_EQ(MY_JSON_TYPE_INT, root.value.val_array.values[3].type);
	ASSERT_EQ(999, root.value.val_array.values[3].value.val_int);
	ASSERT_EQ(MY_JSON_TYPE_INT, root.value.val_array.values[4].type);
	ASSERT_EQ(111, root.value.val_array.values[4].value.val_int);
	my_json_free_document(&root);
}

TEST(json_parse, 2)
{
    struct my_json_value root;
	ASSERT_EQ(0, my_json_parse(&root, "{\"name\" : \"donoughliu\", \"male\" : true, \"height\" : 186.3, \"money_remain\" : false, \"grade\" : 2}"));
	ASSERT_EQ(MY_JSON_TYPE_OBJECT, root.type);
	ASSERT_EQ(5, root.value.val_object.length);
	ASSERT_STREQ(root.value.val_object.pairs[0].key.c_str, "name");
	ASSERT_STREQ(root.value.val_object.pairs[1].key.c_str, "male");
	ASSERT_STREQ(root.value.val_object.pairs[2].key.c_str, "height");
	ASSERT_STREQ(root.value.val_object.pairs[3].key.c_str, "money_remain");
	ASSERT_STREQ(root.value.val_object.pairs[4].key.c_str, "grade");
	ASSERT_STREQ(root.value.val_object.pairs[0].value.value.val_string.c_str, "donoughliu");
	ASSERT_EQ(MY_JSON_TYPE_TRUE, root.value.val_object.pairs[1].value.type);
	ASSERT_EQ(MY_JSON_TYPE_DOUBLE, root.value.val_object.pairs[2].value.type);
	ASSERT_EQ(MY_JSON_TYPE_FALSE, root.value.val_object.pairs[3].value.type);
	ASSERT_EQ(MY_JSON_TYPE_INT, root.value.val_object.pairs[4].value.type);
	ASSERT_EQ(2, root.value.val_object.pairs[4].value.value.val_int);
	my_json_free_document(&root);
}

#define JSON_BUFFER_LENGTH 100

void _ws_remove(const char *str, int *ptr)
{
	while (str[*ptr] == (char)0x20		// space
		   || str[*ptr] == (char)0x9	// tab
		   || str[*ptr] == (char)0xA	// CR
		   || str[*ptr] == (char)0xD)	// LF
		++ * ptr;
}

int _json_equal(const char *one, const char *another)
{
	int one_ptr = 0;
	int another_ptr = 0;
	while (one[one_ptr] != 0 || another[another_ptr] != 0) {
		_ws_remove(one, &one_ptr);
		_ws_remove(another, &another_ptr);
		if (one[one_ptr] != another[another_ptr])
			return -1;
		++one_ptr;
		++another_ptr;
	}
	return 0;
}

TEST(json_write, 0)
{
	char buffer[JSON_BUFFER_LENGTH] = {0};
	struct my_json_value root;
	const char *array_test = "[\"fuck\", 777, 888, 999, 111]";
	ASSERT_EQ(0, my_json_parse(&root, array_test));
	ASSERT_EQ(0, my_json_write(&root, buffer, JSON_BUFFER_LENGTH) < 0);
	my_json_free_document(&root);
	ASSERT_EQ(0, _json_equal(buffer, array_test));
}

TEST(json_write, 1)
{
    char buffer[JSON_BUFFER_LENGTH] = {0};
	const char *expect_result = "{ \"haha\" : \"abc\", \"emm\" : \"abcd\" }";
	struct my_json_value root;
	my_json_value_init(&root);
	root.type = MY_JSON_TYPE_OBJECT;
	struct my_json_value abc, abcd;
	my_json_value_init(&abc);
	my_json_value_init(&abcd);
	abc.type = MY_JSON_TYPE_STRING;
	abcd.type = MY_JSON_TYPE_STRING;
	my_json_string_pushback(&abc.value.val_string, 'a');
	my_json_string_pushback(&abc.value.val_string, 'b');
	my_json_string_pushback(&abc.value.val_string, 'c');
	my_json_string_duplicate(&abcd.value.val_string, &abc.value.val_string);
	my_json_string_pushback(&abcd.value.val_string, 'd');
	my_json_object_set_value(&root.value.val_object, "haha", &abc);
	my_json_object_set_value(&root.value.val_object, "emm", &abcd);
	ASSERT_EQ(0, my_json_write(&root, buffer, JSON_BUFFER_LENGTH) < 0);
	ASSERT_EQ(0, _json_equal(buffer, expect_result));
	struct my_json_value get_value;
	my_json_object_get_value(&root.value.val_object, "emm", &get_value);
	ASSERT_EQ(MY_JSON_TYPE_STRING, get_value.type);
	ASSERT_EQ(0, strcmp(get_value.value.val_string.c_str, "abcd"));
	my_json_free_document(&root);
}

TEST(json_parse_write, 0)
{
    char buffer[JSON_BUFFER_LENGTH] = {0};
	struct my_json_value root;
	const char *array_test = "[\"fuck\", 777, 888, 999, 111]";
	ASSERT_EQ(0, my_json_parse(&root, array_test));
	ASSERT_EQ(0, my_json_write(&root, buffer, JSON_BUFFER_LENGTH) < 0);
	ASSERT_EQ(0, _json_equal(buffer, array_test));
	my_json_free_document(&root);
}
