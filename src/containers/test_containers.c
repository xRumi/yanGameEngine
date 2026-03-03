#include "test_containers.h"

void test_darray() {
    // for int elements
    // create darray with int data type and default capacity
    int* darray = darray_create(int);
    assert(darray_get_capacity(darray) == _DARRAY_INITIAL_CAPACITY);
    // push variable
    int a = 55;
    darray_push(darray, a);
    assert(darray_at_type(darray, 0, int) == a);
    // push immediate value
    darray_push(darray, 5);
    assert(darray_at_type(darray, 1, int) == 5);
    // push int 0 to 4
    for (int i = 0; i < 5; i++) darray_push(darray, i);
    assert(darray_get_length(darray) == 5 + 2);
    // erase
    darray_erase_at(darray, 0);
    assert(darray_at_type(darray, 0, int) == 5);
    darray_erase_at(darray, 3);
    assert(darray_at_type(darray, 3, int) == 3);

    // insert at 50 position
    darray_insert_at(darray, 99, 50);
    assert(darray[50] == 99);
    assert(darray_at_type(darray, 50, int) == 99);
    assert(darray_get_length(darray) == 51);
    // destroy darray
    darray_destroy(darray);

    // for string(char*) elements
    // create darray with string data type and default capacity
    darray = darray_create(const char*);
    // push string variable
    const char* cstr = "hello world!\n";
    darray_push(darray, cstr);
    assert(darray_at_type(darray, 0, const char*) == cstr);
    // destroy darray
    darray_destroy(darray);
}

void test_containers() {
    TEST("test darray - start");
    test_darray();
    TEST("test darray - completed");
}
