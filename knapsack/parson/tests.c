/*
 Parson ( http://kgabis.github.com/parson/ )
 Copyright (c) 2012 - 2014 Krzysztof Gabis
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "parson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(A) printf("%-72s-",#A);              \
                if(A){puts(" OK");tests_passed++;} \
                else{puts(" FAIL");tests_failed++;}
#define STREQ(A, B) (A && B ? strcmp(A, B) == 0 : 0)


void test_suite_1(void);
void test_suite_2(JSON_Value *value);
void test_suite_2_no_comments(void);
void test_suite_2_with_comments(void);
void test_suite_3(void);
void test_precincts_file(void);
void test_suite_precincts(JSON_Value *root_value);

char *read_file(const char *filename);
void print_commits_info(const char *username, const char *repo);

static int tests_passed;
static int tests_failed;

int main() {
    /* Example function from readme file:       */
    /* print_commits_info("torvalds", "linux"); */
    /*
    test_suite_1();
    test_suite_2_no_comments();
    test_suite_2_with_comments();
    test_suite_3();
    */
    
    test_precincts_file();

    printf("Tests failed: %d\n", tests_failed);
    printf("Tests passed: %d\n", tests_passed);
    return 0;
}

/* 3 test files from json.org */
void test_suite_1(void) {
    JSON_Value *val;
    TEST((val = json_parse_file("tests/test_1_1.txt")) != NULL);
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file("tests/test_1_2.txt")) != NULL);
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file("tests/test_1_3.txt")) != NULL);
    if (val) { json_value_free(val); }
    
    TEST((val = json_parse_file_with_comments("tests/test_1_1.txt")) != NULL);
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file_with_comments("tests/test_1_2.txt")) != NULL);
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file_with_comments("tests/test_1_3.txt")) != NULL);
    if (val) { json_value_free(val); }

}

/* Testing correctness of parsed values */
void test_suite_2(JSON_Value *root_value) {
    JSON_Object *root_object;
    JSON_Array *array;
    size_t i;
    TEST(root_value);
    TEST(json_value_get_type(root_value) == JSONObject);
    root_object = json_value_get_object(root_value);
    TEST(STREQ(json_object_get_string(root_object, "string"), "lorem ipsum"));
    TEST(STREQ(json_object_get_string(root_object, "utf string"), "lorem ipsum"));
    TEST(STREQ(json_object_get_string(root_object, "utf-8 string"), "あいうえお"));
    TEST(STREQ(json_object_get_string(root_object, "surrogate string"), "lorem𝄞ipsum𝍧lorem"));
    TEST(json_object_get_number(root_object, "positive one") == 1.0);
    TEST(json_object_get_number(root_object, "negative one") == -1.0);
    TEST(json_object_get_number(root_object, "hard to parse number") == -0.000314);
    TEST(json_object_get_boolean(root_object, "boolean true") == 1);
    TEST(json_object_get_boolean(root_object, "boolean false") == 0);
    TEST(json_value_get_type(json_object_get_value(root_object, "null")) == JSONNull);
    
    array = json_object_get_array(root_object, "string array");
    if (array != NULL && json_array_get_count(array) > 1) {
        TEST(STREQ(json_array_get_string(array, 0), "lorem"));
        TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
    } else {
        tests_failed++;
    }
    
    array = json_object_get_array(root_object, "x^2 array");
    if (array != NULL) {
        for (i = 0; i < json_array_get_count(array); i++) {
            TEST(json_array_get_number(array, i) == (i * i));
        }
    } else {
        tests_failed++;
    }
    
    TEST(json_object_get_array(root_object, "non existent array") == NULL);
    TEST(STREQ(json_object_dotget_string(root_object, "object.nested string"), "str"));
    TEST(json_object_dotget_boolean(root_object, "object.nested true") == 1);
    TEST(json_object_dotget_boolean(root_object, "object.nested false") == 0);
    TEST(json_object_dotget_value(root_object, "object.nested null") != NULL);
    TEST(json_object_dotget_number(root_object, "object.nested number") == 123);
    
    TEST(json_object_dotget_value(root_object, "should.be.null") == NULL);
    TEST(json_object_dotget_value(root_object, "should.be.null.") == NULL);
    TEST(json_object_dotget_value(root_object, ".") == NULL);
    TEST(json_object_dotget_value(root_object, "") == NULL);
    
    array = json_object_dotget_array(root_object, "object.nested array");
    if (array != NULL && json_array_get_count(array) > 1) {
        TEST(STREQ(json_array_get_string(array, 0), "lorem"));
        TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
    } else {
        tests_failed++;
    }
    TEST(json_object_dotget_boolean(root_object, "nested true"));
    
    TEST(STREQ(json_object_get_string(root_object, "/**/"), "comment"));
    TEST(STREQ(json_object_get_string(root_object, "//"), "comment"));
}

void test_suite_2_no_comments(void) {
    const char *filename = "tests/test_2.txt";
    JSON_Value *root_value = NULL;
    printf("Testing %s:\n", filename);
    root_value = json_parse_file(filename);
    test_suite_2(root_value);
    json_value_free(root_value);
}

void test_suite_2_with_comments(void) {
    const char *filename = "tests/test_2_comments.txt";
    JSON_Value *root_value = NULL;
    printf("Testing %s:\n", filename);
    root_value = json_parse_file_with_comments(filename);
    test_suite_2(root_value);
    json_value_free(root_value);
}

/* Testing values, on which parsing should fail */
void test_suite_3(void) {
    char nested_20x[] = "[[[[[[[[[[[[[[[[[[[[\"hi\"]]]]]]]]]]]]]]]]]]]]";
    puts("Testing invalid strings:");
    TEST(json_parse_string(NULL) == NULL);
    TEST(json_parse_string("") == NULL); /* empty string */
    TEST(json_parse_string("[\"lorem\",]") == NULL);
    TEST(json_parse_string("{\"lorem\":\"ipsum\",}") == NULL);
    TEST(json_parse_string("{lorem:ipsum}") == NULL);
    TEST(json_parse_string("[,]") == NULL);
    TEST(json_parse_string("[,") == NULL);
    TEST(json_parse_string("[") == NULL);
    TEST(json_parse_string("]") == NULL);
    TEST(json_parse_string("{\"a\":0,\"a\":0}") == NULL); /* duplicate keys */
    TEST(json_parse_string("{:,}") == NULL);
    TEST(json_parse_string("{,}") == NULL);
    TEST(json_parse_string("{,") == NULL);
    TEST(json_parse_string("{:") == NULL);
    TEST(json_parse_string("{") == NULL);
    TEST(json_parse_string("}") == NULL);
    TEST(json_parse_string("x") == NULL);
    TEST(json_parse_string("\"string\"") == NULL);
    TEST(json_parse_string("{:\"no name\"}") == NULL);
    TEST(json_parse_string("[,\"no first value\"]") == NULL);
    TEST(json_parse_string("[\"\\u00zz\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\u00\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\u\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\"\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\0\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\a\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\b\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\t\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\n\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\f\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\r\"]") == NULL); /* control character */
    TEST(json_parse_string(nested_20x) == NULL); /* too deep */
    TEST(json_parse_string("[0x2]") == NULL);    /* hex */
    TEST(json_parse_string("[0X2]") == NULL);    /* HEX */
    TEST(json_parse_string("[07]") == NULL);     /* octals */
    TEST(json_parse_string("[0070]") == NULL);
    TEST(json_parse_string("[07.0]") == NULL);
    TEST(json_parse_string("[-07]") == NULL);
    TEST(json_parse_string("[-007]") == NULL);
    TEST(json_parse_string("[-07.0]") == NULL);
    TEST(json_parse_string("[\"\\uDF67\\uD834\"]") == NULL); /* wrong order surrogate pair */
}

void print_commits_info(const char *username, const char *repo) {
    JSON_Value *root_value;
    JSON_Array *commits;
    JSON_Object *commit;
    size_t i;
    
    char curl_command[512];
    char cleanup_command[256];
    char output_filename[] = "commits.json";
    
    /* it ain't pretty, but it's not a libcurl tutorial */
    sprintf(curl_command, 
        "curl -s \"https://api.github.com/repos/%s/%s/commits\" > %s",
        username, repo, output_filename);
    sprintf(cleanup_command, "rm -f %s", output_filename);
    system(curl_command);
    
    /* parsing json and validating output */
    root_value = json_parse_file(output_filename);
    if (json_value_get_type(root_value) != JSONArray) {
        system(cleanup_command);
        return;
    }
    
    /* getting array from root value and printing commit info */
    commits = json_value_get_array(root_value);
    printf("%-10.10s %-10.10s %s\n", "Date", "SHA", "Author");
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        printf("%.10s %.10s %s\n",
               json_object_dotget_string(commit, "commit.author.date"),
               json_object_get_string(commit, "sha"),
               json_object_dotget_string(commit, "commit.author.name"));
    }
    
    /* cleanup code */
    json_value_free(root_value);
    system(cleanup_command);
}

void test_precincts_file(void) {
    const char *filename = "tests/precincts.json";
    JSON_Value *root_value = NULL;
    printf("Testing %s:\n", filename);
    root_value = json_parse_file(filename);
    test_suite_precincts(root_value);
    json_value_free(root_value);
}

void test_suite_precincts(JSON_Value *root_value) {
    JSON_Object *root_object;
    JSON_Array *array;

    TEST(root_value);
    TEST(json_value_get_type(root_value) == JSONObject);
    root_object = json_value_get_object(root_value);

    unsigned int number_of_precincts = (int) json_object_get_number(root_object, "number_of_precincts");
    printf("number_of_precincts: %d\n", number_of_precincts);

    TEST(number_of_precincts == 60);

    array = json_object_get_array(root_object, "precincts");
    if (array != NULL && json_array_get_count(array) == number_of_precincts) {
        JSON_Object *precinct_object;
        JSON_Array *layers_array;
        int id, coord_x, coord_y;
        unsigned int j;
        
        for(j = 0; j < number_of_precincts; j++) {
            
            precinct_object = json_array_get_object(array, j);
            id = (int) json_object_get_number(precinct_object, "id");
            coord_x = (int) json_object_get_number(precinct_object, "coord_x");
            coord_y = (int) json_object_get_number(precinct_object, "coord_y");

            printf("id: %d \t coord_x: %d \t coord_y: %d\n", id, coord_x, coord_y);

            layers_array = json_object_get_array(precinct_object, "layers");
            if (layers_array != NULL && json_array_get_count(layers_array) >= 1) {
                JSON_Object *layer_object;
                int ql, bytes, psnr;
                unsigned k;

                for(k = 0; k < json_array_get_count(layers_array); k++) {

                    layer_object = json_array_get_object(layers_array, k);
                    ql = (int) json_object_get_number(layer_object, "ql");
                    bytes = (int) json_object_get_number(layer_object, "bytes");
                    psnr = (int) json_object_get_number(layer_object, "psnr");

                    printf("\tql: %d \t bytes: %d \t psnr: %d\n", ql, bytes, psnr);
                }
                
            } else {

            }

        }
    } else {
        tests_failed++;
    }
}