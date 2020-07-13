#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include <iostream>

#define MAX_ARGS 1024

extern "C"
{
    my_bool hello_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    void hello_deinit(UDF_INIT *initid);
    char *hello(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
}

int main(int argc, char *argv[])
{
    fprintf(stdout, "Test started\n");

    UDF_INIT uinit;
    UDF_ARGS uargs;
    char message[MYSQL_ERRMSG_SIZE], result[1024];
    unsigned long length;
    char is_null, error;
    enum Item_result arg_types[MAX_ARGS];
    long long long_args[MAX_ARGS];
    unsigned long lengths[MAX_ARGS], attribute_lengths[MAX_ARGS];
    char maybe_null[MAX_ARGS];
    char attributes[MAX_ARGS][8];
    char *attr_ptrs[MAX_ARGS];
    const char *ret_res;
    char *long_end;
    int arg;

    uinit.maybe_null = true;
    uinit.decimals = 31;
    uinit.max_length = 1024;
    uinit.ptr = NULL;
    uinit.const_item = false;
    uinit.extension = NULL;

    uargs.arg_count = argc - 1;
    uargs.arg_type = arg_types;
    uargs.args = argv + 1;
    uargs.lengths = lengths;
    uargs.maybe_null = maybe_null;
    uargs.attributes = attr_ptrs;
    uargs.attribute_lengths = attribute_lengths;
    uargs.extension = NULL;

    if (hello_init(&uinit, &uargs, message))
    {
        fprintf(stderr, "init error\n", message);
        return 1;
    }
    is_null = error = 0;
    ret_res = hello(&uinit, &uargs, result, &length, &is_null, &error);
    if (error)
    {
        fprintf(stderr, "returned error\n");
        hello_deinit(&uinit);
        return 2;
    }
    if (is_null)
    {
        fprintf(stderr, "returned NULL\n");
    }
    else
    {
        fprintf(stdout, ret_res);
    }
    hello_deinit(&uinit);
    return 0;
}