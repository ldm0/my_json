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
			printf("%dth test Passed(Line: %d)\n", assert_count, __LINE__);\
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
        long long int expect_result = expect;\
        long long int reality_result = reality;\
        TEST_BASE(expect_result == reality_result, "%lld", expect_result, reality_result);\
    } while (0)

#define ASSERT_EQUAL_DOUBLE(expect, reality)\
    do {\
        double expect_result = expect;\
        double reality_result = reality;\
        int equality = ((expect_result - reality_result) < PERMITTED_DOUBLE_ERROR && (expect_result - reality_result) > -PERMITTED_DOUBLE_ERROR);\
        TEST_BASE(equality, "%lf", expect_result, reality_result);\
    } while (0)

/*
#define ASSERT_EQUAL_STRING(expect, reality)\
	do {\
		int equality = (!strcmp(expect, reality));\
*/	



// These tests are single unit test during development.
// If test, delete static in source code before individual functions
#ifdef IN_DEVELOPMENT

extern void reset_ptr(void);
extern unsigned long long int get_ptr(void);
extern enum PARSE_STATE parse_null(const char * const json);
extern enum PARSE_STATE parse_false(const char * const json);
extern enum PARSE_STATE parse_true(const char * const json);
extern enum PARSE_STATE parse_number(enum JSON_TYPE * type, union Value * value, const char * const json);
extern enum PARSE_STATE parse_string(struct String * const _string, const char * const json);
extern enum PARSE_STATE parse_array(struct Array * const _array, const char * const json);
extern enum PARSE_STATE parse_object(struct Object * const object, const char * const json);

void test_int_ok(const int i, const char * const number)
{
	reset_ptr();
	struct Pair pair;
	memset(&pair, 0, sizeof(struct Pair));
	pair.type = JSON_TYPE_UNKNOWN;
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_number(&(pair.type), &(pair.value), number));
	ASSERT_EQUAL_INT(i, pair.value.val_int);
	ASSERT_EQUAL_INT(JSON_TYPE_INT, pair.type);
}

void test_double_ok(const double lf, const char * const number)
{
	reset_ptr();
	struct Pair pair;
	memset(&pair, 0, sizeof(struct Pair));
	pair.type = JSON_TYPE_UNKNOWN;
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_number(&(pair.type), &(pair.value), number));
	ASSERT_EQUAL_INT(JSON_TYPE_DOUBLE, pair.type);
	ASSERT_EQUAL_DOUBLE(lf, pair.value.val_double);
}

void test_true_ok(const char *const str)
{
	reset_ptr();
	struct Pair pair;
	memset(&pair, 0, sizeof(struct Pair));
	pair.type = JSON_TYPE_UNKNOWN;
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_true(str));
}

void test_false_ok(const char *const str)
{
	reset_ptr();
	struct Pair pair;
	memset(&pair, 0, sizeof(struct Pair));
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_false(str));
}

void test_null_ok(const char *const str)
{
	reset_ptr();
	struct Pair pair;
	memset(&pair, 0, sizeof(struct Pair));
	pair.type = JSON_TYPE_UNKNOWN;
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_null(str));
}

void test_string()
{
	struct Pair root;

	reset_ptr();
	memset(&root, 0, sizeof(struct Pair));
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_string(&(root.value.val_string), "\"666777888999\""));
	ASSERT_EQUAL_INT(12, root.value.val_string.length);
}

void test_array()
{
	struct Pair root;

	reset_ptr();
	memset(&root, 0, sizeof(struct Pair));
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_array(&(root.value.val_array), "[]"));

	reset_ptr();
	memset(&root, 0, sizeof(struct Pair));
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_array(&(root.value.val_array), "[1, 2]"));
	ASSERT_EQUAL_INT(JSON_TYPE_INT, root.value.val_array.root->type);
	ASSERT_EQUAL_INT(1, root.value.val_array.root->value.val_int);
	ASSERT_EQUAL_INT(JSON_TYPE_INT, root.value.val_array.root->next->type);
	ASSERT_EQUAL_INT(2, root.value.val_array.root->next->value.val_int);

	reset_ptr();
	memset(&root, 0, sizeof(struct Pair));
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_array(&(root.value.val_array), "[666]"));
	ASSERT_EQUAL_INT(666, root.value.val_array.root->value.val_int);

	reset_ptr();
	memset(&root, 0, sizeof(struct Pair));
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_array(&(root.value.val_array), "[9.99, \"haoha\", 4]"));
	ASSERT_EQUAL_INT(JSON_TYPE_DOUBLE, root.value.val_array.root->type);
	ASSERT_EQUAL_DOUBLE(9.99, root.value.val_array.root->value.val_double);
	ASSERT_EQUAL_INT(JSON_TYPE_STRING, root.value.val_array.root->next->type);
	ASSERT_EQUAL_INT(5, root.value.val_array.root->next->value.val_string.length);
	ASSERT_EQUAL_INT(JSON_TYPE_INT, root.value.val_array.root->next->next->type);
	ASSERT_EQUAL_INT(4, root.value.val_array.root->next->next->value.val_int);

}

void test_number()
{
	test_int_ok(-100, "-100");
	test_int_ok(100, "100");
	test_int_ok(10, "10");

	test_int_ok(INT_MAX, "2147483647");
	test_int_ok(INT_MIN, "-2147483648");
	test_double_ok(10000, "100E02");
	test_double_ok(100000, "100E+03");
	test_double_ok(-10100000., "-1.01e7");
	// This test need special attention
	// I'm not sure is it a double, but for program simplicity, assume it as a double
	test_double_ok(10., "100E-01");
	test_double_ok(100., "100.0");

	test_double_ok(10.678, "10.678");
	test_double_ok(-100., "-100.0");
	test_double_ok(0.1, "1E-01");
	test_double_ok(0.1, "1e-1");
	test_double_ok(-0.1, "-1e-1");

	test_double_ok(-0.00000112, "-1.12e-6");
	test_double_ok(-1.12e-666, "-1.12e-666");
	test_double_ok(-1.12e-666, "-1.12e-0666");
	test_double_ok(1.0000000000000002, "1.0000000000000002");
	// minimum denormal
	test_double_ok(4.9406564584124654e-324, "4.9406564584124654e-324");

	test_double_ok(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	// Max subnormal double
	test_double_ok(2.2250738585072009e-308, "2.2250738585072009e-308");
	test_double_ok(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	// Min normal positive double
	test_double_ok(2.2250738585072014e-308, "2.2250738585072014e-308");
	test_double_ok(-2.2250738585072014e-308, "-2.2250738585072014e-308");

	// cannot support max double because the double inner mechanism(lazy) XP
	// test_double_ok(1.7976931348623157e+308, "1.7976931348623157e+308");
	// test_double_ok(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

void test_object()
{
	reset_ptr();
	struct Pair root;
	memset(&root, 0, sizeof(struct Pair));
	ASSERT_EQUAL_INT(PARSE_STATE_OK, parse_object(&(root.value.val_object), "{ \"hahaha\" : \"fuck\", \"emm\" : null}"));
	// length of hahaha
	ASSERT_EQUAL_INT(6, root.value.val_object.root->key.length);			
	// length of fuck
	ASSERT_EQUAL_INT(4, root.value.val_object.root->value.val_string.length);			
	// length of emm
	ASSERT_EQUAL_INT(3, root.value.val_object.root->next->key.length);		
	// type of emm
	ASSERT_EQUAL_INT(JSON_TYPE_NULL, root.value.val_object.root->next->type);		
}

void test_true_false_null()
{
	// does not test string with leading ws because function assumes no leading ws
	test_true_ok("true");
	test_true_ok("true  ");

	test_false_ok("false");
	test_false_ok("false  ");

	test_null_ok("null");
	test_null_ok("null  ");
}

#endif

void test_parse_json()
{
	struct my_json_pair root;
	ASSERT_EQUAL_INT(0 , my_json_parse(&root, "[666, 777, 888, 999, 111]"));
	ASSERT_EQUAL_INT(MY_JSON_TYPE_ARRAY, root.type);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_array.root->type);
	ASSERT_EQUAL_INT(666, root.value.val_array.root->value.val_int);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_array.root->next->type);
	ASSERT_EQUAL_INT(777, root.value.val_array.root->next->value.val_int);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_array.root->next->next->type);
	ASSERT_EQUAL_INT(888, root.value.val_array.root->next->next->value.val_int);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_array.root->next->next->next->type);
	ASSERT_EQUAL_INT(999, root.value.val_array.root->next->next->next->value.val_int);
	ASSERT_EQUAL_INT(MY_JSON_TYPE_INT, root.value.val_array.root->next->next->next->next->type);
	ASSERT_EQUAL_INT(111, root.value.val_array.root->next->next->next->next->value.val_int);
}

void test_write_json()
{
	char buffer[JSON_BUFFER_LENGTH];
	struct my_json_pair root;
	ASSERT_EQUAL_INT(0, my_json_parse(&root, "[666, 777, 888, 999, 111]"));
	ASSERT_EQUAL_INT(0, my_json_write(&root, buffer, JSON_BUFFER_LENGTH) < 0);
	ASSERT_EQUAL_INT(0, strcmp(buffer, "[666, 777, 888, 999, 111]"));
}

void test_parse_write_json()
{
	char buffer[JSON_BUFFER_LENGTH];
	struct my_json_pair root;
	const char *array_test = "[\"fuck\", 777, 888, 999, 111]";
	ASSERT_EQUAL_INT(0, my_json_parse(&root, array_test));
	ASSERT_EQUAL_INT(0, my_json_write(&root, buffer, JSON_BUFFER_LENGTH) < 0);
	ASSERT_EQUAL_INT(0, strcmp(buffer, array_test));
}

void test_all()
{
#ifdef IN_DEVELOPMENT
	test_number();
	test_true_false_null();
	test_array();
	test_string();
	test_object();
#endif
	test_parse_json();
	test_write_json();
	test_parse_write_json();
}
// These are test when library are substantially built up.


int main()
{
	test_all();
	fprintf(stdout, "Test end!\nPass rate %d/%d(%3.2f%%).\n", pass_count, assert_count, 100.f * pass_count / assert_count);
	return 0;
}