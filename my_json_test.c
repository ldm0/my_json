#include<stdio.h>
#include<limits.h>
#include<string.h>

#include"my_json.h"

#define JSON_BUFFER_LENGTH 100
#define PERMITTED_DOUBLE_ERROR 0.0000001

static int assert_count = 0;
static int pass_count = 0;

// should not be used directly
#define TEST_BASE(equality, format, expect_result, reality_result) \
    do {\
        ++assert_count;\
        if (equality) {\
            ++pass_count;\
			printf("%dth test Passed \t(Line: %d)\n", assert_count, __LINE__);\
        } else {\
            fprintf(stderr, "\nERROR: %s:%d\n"\
            "%dth test:\n"\
            "Expect  : " format "\n"\
            "Reality : " format "\n", __FILE__, __LINE__, assert_count, expect_result, reality_result);\
        }\
    } while(0)

// to avoid call one function twice, use tmp location to store result
#define ASSERT_EQUAL_INT(expect, reality)\
    do {\
        long long int _expect_result = expect;\
        long long int _reality_result = reality;\
        TEST_BASE(_expect_result == _reality_result, "%lld", _expect_result, _reality_result);\
    } while (0)

#define ASSERT_EQUAL_DOUBLE(expect, reality)\
    do {\
        double expect_result = expect;\
        double reality_result = reality;\
        int equality = ((expect_result - reality_result) < PERMITTED_DOUBLE_ERROR && (expect_result - reality_result) > -PERMITTED_DOUBLE_ERROR);\
        TEST_BASE(equality, "%lf", expect_result, reality_result);\
    } while (0)

#define ASSERT_EQUAL_STRING(expect, reality)\
	do {\
		const char *expect_result = expect;\
		const char *reality_result = reality;\
		int equality = (!strcmp(expect_result, reality_result));\
        TEST_BASE(equality, "%s", expect_result, reality_result);\
    } while (0)

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


void test_parse_json_0()
{
	struct my_json_value root;
	ASSERT_EQUAL_INT(0, my_json_parse(&root, "[666, 777, 888, 999, 111]"));
	ASSERT_EQUAL_INT(MY_JSON_TYPE_ARRAY, root.type);
	ASSERT_EQUAL_INT(5, root.value.val_array.length);
	for (int i = 0; i < 5; ++i)
		ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_array.values[i].type);
	ASSERT_EQUAL_INT(666, root.value.val_array.values[0].value.val_int);
	ASSERT_EQUAL_INT(777, root.value.val_array.values[1].value.val_int);
	ASSERT_EQUAL_INT(888, root.value.val_array.values[2].value.val_int);
	ASSERT_EQUAL_INT(999, root.value.val_array.values[3].value.val_int);
	ASSERT_EQUAL_INT(111, root.value.val_array.values[4].value.val_int);
	my_json_free_document(&root);
}

void test_parse_json_1()
{
	struct my_json_value root;
	ASSERT_EQUAL_INT(0, my_json_parse(&root, "[\"emm\", 10.1, true, 999, 111]"));
	ASSERT_EQUAL_INT(MY_JSON_TYPE_ARRAY, root.type);
	ASSERT_EQUAL_INT(5, root.value.val_array.length);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_STRING, root.value.val_array.values[0].type);
	ASSERT_EQUAL_STRING(root.value.val_array.values[0].value.val_string.c_str, "emm");
	ASSERT_EQUAL_INT(MY_JSON_TYPE_DOUBLE, root.value.val_array.values[1].type);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_TRUE, root.value.val_array.values[2].type);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_array.values[3].type);
	ASSERT_EQUAL_INT(999, root.value.val_array.values[3].value.val_int);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_array.values[4].type);
	ASSERT_EQUAL_INT(111, root.value.val_array.values[4].value.val_int);
	my_json_free_document(&root);
}

void test_parse_json_2()
{
	struct my_json_value root;
	ASSERT_EQUAL_INT(0, my_json_parse(&root, "{\"name\" : \"donoughliu\", \"male\" : true, \"height\" : 186.3, \"money_remain\" : false, \"grade\" : 2}"));
	ASSERT_EQUAL_INT(MY_JSON_TYPE_OBJECT, root.type);
	ASSERT_EQUAL_INT(5, root.value.val_object.length);
	ASSERT_EQUAL_STRING(root.value.val_object.pairs[0].key.c_str, "name");
	ASSERT_EQUAL_STRING(root.value.val_object.pairs[1].key.c_str, "male");
	ASSERT_EQUAL_STRING(root.value.val_object.pairs[2].key.c_str, "height");
	ASSERT_EQUAL_STRING(root.value.val_object.pairs[3].key.c_str, "money_remain");
	ASSERT_EQUAL_STRING(root.value.val_object.pairs[4].key.c_str, "grade");
	ASSERT_EQUAL_STRING(root.value.val_object.pairs[0].value.value.val_string.c_str, "donoughliu");
	ASSERT_EQUAL_INT(MY_JSON_TYPE_TRUE, root.value.val_object.pairs[1].value.type);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_DOUBLE, root.value.val_object.pairs[2].value.type);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_FALSE, root.value.val_object.pairs[3].value.type);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_object.pairs[4].value.type);
	ASSERT_EQUAL_INT(2, root.value.val_object.pairs[4].value.value.val_int);
	my_json_free_document(&root);
}


void test_parse_json()
{
	test_parse_json_0();
	test_parse_json_1();
	test_parse_json_2();
}

void test_write_json_0()
{
	char buffer[JSON_BUFFER_LENGTH] = {0};
	struct my_json_value root;
	const char *array_test = "[\"fuck\", 777, 888, 999, 111]";
	ASSERT_EQUAL_INT(0, my_json_parse(&root, array_test));
	ASSERT_EQUAL_INT(0, my_json_write(&root, buffer, JSON_BUFFER_LENGTH) < 0);
	my_json_free_document(&root);
	ASSERT_EQUAL_INT(0, _json_equal(buffer, array_test));
}

void test_write_json_1()
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
	ASSERT_EQUAL_INT(0, my_json_write(&root, buffer, JSON_BUFFER_LENGTH) < 0);
	ASSERT_EQUAL_INT(0, _json_equal(buffer, expect_result));
	struct my_json_value get_value;
	my_json_object_get_value(&root.value.val_object, "emm", &get_value);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_STRING, get_value.type);
	ASSERT_EQUAL_INT(0, strcmp(get_value.value.val_string.c_str, "abcd"));
	my_json_free_document(&root);
}

void test_write_json()
{
	test_write_json_0();
	test_write_json_1();
}

void test_parse_write_json()
{
	char buffer[JSON_BUFFER_LENGTH] = {0};
	struct my_json_value root;
	const char *array_test = "[\"fuck\", 777, 888, 999, 111]";
	ASSERT_EQUAL_INT(0, my_json_parse(&root, array_test));
	ASSERT_EQUAL_INT(0, my_json_write(&root, buffer, JSON_BUFFER_LENGTH) < 0);
	ASSERT_EQUAL_INT(0, _json_equal(buffer, array_test));
	my_json_free_document(&root);
}

// These are test when library are substantially built up.
void test_all()
{
	test_parse_json();
	test_write_json();
	test_parse_write_json();
}

int main()
{
	test_all();
	fprintf(stdout, "Test result\nPass rate: %d/%d(%3.2f%%).\n", pass_count, assert_count, 100.f * pass_count / assert_count);
	return 0;
}