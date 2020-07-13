extern "C"
{
#include <mysql/mysql.h>
#include <string.h>
}
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#define MAX_STR_LEN 256

enum InputTypes
{
    LIST_AS_STR,
    SET_AS_STR
};

size_t string_find_first_not(const std::string *str, size_t pos, char delimeter)
{
    for (size_t i = pos; i < str->size(); i++)
    {
        if ((*str)[i] != delimeter)
            return i;
    }
    return std::string::npos;
}

void split_to_list(const std::string *str, std::vector<std::string> *tokens, char delimeter)
{
    tokens->clear();
    size_t lastPos = string_find_first_not(str, 0, delimeter);
    size_t pos = str->find(delimeter, lastPos);
    while (lastPos != std::string::npos)
    {
        tokens->emplace_back(str->substr(lastPos, pos - lastPos));
        lastPos = string_find_first_not(str, pos, delimeter);
        pos = str->find(delimeter, lastPos);
    }
}

void split_to_set(const std::string *str, std::unordered_set<std::string> *tokens, char delimeter)
{
    tokens->clear();
    size_t lastPos = string_find_first_not(str, 0, delimeter);
    size_t pos = str->find(delimeter, lastPos);
    while (lastPos != std::string::npos)
    {
        tokens->emplace(str->substr(lastPos, pos - lastPos));
        lastPos = string_find_first_not(str, pos, delimeter);
        pos = str->find(delimeter, lastPos);
    }
}

extern "C" my_bool size_as_list_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if (args->arg_count != 1 || args->arg_type[0] != STRING_RESULT)
    {
        strcpy(message, "Wrong arguments type for size_as_list");
        return true;
    }
    initid->maybe_null = false;
    return false;
}

extern "C" void size_as_list_deinit(UDF_INIT *initid)
{
    // pass
}

extern "C" long long size_as_list(UDF_INIT *initid, UDF_ARGS *args,
                                  char *result, unsigned long *length, char *is_null, char *error)
{
    long long res = 0;
    if (!args->args[0])
    {
        return res;
    }
    auto input = std::string(args->args[0]);
    auto str_list = new std::vector<std::string>;
    split_to_list(&input, str_list, ',');
    res = str_list->size();
    delete str_list;
    return res;
}

extern "C" my_bool size_as_set_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if (args->arg_count != 1 || args->arg_type[0] != STRING_RESULT)
    {
        strcpy(message, "Wrong arguments type for size_as_set");
        return true;
    }
    initid->maybe_null = false;
    return false;
}

extern "C" void size_as_set_deinit(UDF_INIT *initid)
{
    // pass
}

extern "C" long long size_as_set(UDF_INIT *initid, UDF_ARGS *args,
                                 char *result, unsigned long *length, char *is_null, char *error)
{
    long long res = 0;
    if (!args->args[0])
    {
        return res;
    }
    auto input = std::string(args->args[0]);
    auto str_set = new std::unordered_set<std::string>;
    split_to_set(&input, str_set, ',');
    res = str_set->size();
    delete str_set;
    return res;
}

my_bool common_str_count_map_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if (args->arg_count != 1 || args->arg_type[0] != STRING_RESULT)
    {
        strcpy(message, "Wrong arguments type");
        return true;
    }
    std::unordered_map<std::string, long long> *data_map = new std::unordered_map<std::string, long long>;
    initid->ptr = (char *)data_map;
    initid->maybe_null = false;
    return false;
}

void common_str_count_map_deinit(UDF_INIT *initid)
{
    std::unordered_map<std::string, long long> *data_map = (std::unordered_map<std::string, long long> *)initid->ptr;
    data_map->clear();
    delete data_map;
}

void common_str_count_map_clear(UDF_INIT *initid)
{
    std::unordered_map<std::string, long long> *data_map = (std::unordered_map<std::string, long long> *)initid->ptr;
    data_map->clear();
}

void common_str_count_map_add(UDF_INIT *initid, UDF_ARGS *args, InputTypes input_type)
{
    std::unordered_map<std::string, long long> *data_map = (std::unordered_map<std::string, long long> *)initid->ptr;
    if (!args->args[0])
    {
        return;
    }
    std::unordered_map<std::string, long long>::iterator it;
    auto entire_str = std::string(args->args[0]);

    auto str_list = new std::vector<std::string>;
    if (input_type == LIST_AS_STR)
    {
        split_to_list(&entire_str, str_list, ',');
    }
    else if (input_type == SET_AS_STR)
    {
        auto *str_set = new std::unordered_set<std::string>();
        split_to_set(&entire_str, str_set, ',');
        for (const auto &it : *str_set)
        {
            str_list->emplace_back(it);
        }
        delete str_set;
    }

    for (auto data_str : *str_list)
    {
        it = data_map->find(data_str);
        if (it != data_map->end())
        {
            (*data_map)[data_str] += 1;
        }
        else
        {
            (*data_map)[data_str] = 1;
        }
    }
    delete str_list;
}

void common_str_count_map(UDF_INIT *initid, UDF_ARGS *args, std::string *most_freq,
                          long long *most_cnt, long long *items_total)
{
    auto *data_map = (std::unordered_map<std::string, long long> *)initid->ptr;
    for (auto it = data_map->begin(); it != data_map->end(); it++)
    {
        *items_total += it->second;
        if (it->second > *most_cnt)
        {
            *most_freq = it->first;
            *most_cnt = it->second;
        }
    }
}

extern "C" my_bool most_freq_as_list_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    return common_str_count_map_init(initid, args, message);
}

extern "C" void most_freq_as_list_deinit(UDF_INIT *initid)
{
    common_str_count_map_deinit(initid);
}

extern "C" void most_freq_as_list_clear(UDF_INIT *initid,
                                        unsigned char *, unsigned char *)
{
    common_str_count_map_clear(initid);
}

extern "C" void most_freq_as_list_add(UDF_INIT *initid, UDF_ARGS *args,
                                      unsigned char *, unsigned char *)
{
    common_str_count_map_add(initid, args, LIST_AS_STR);
}

extern "C" const char *most_freq_as_list(UDF_INIT *initid, UDF_ARGS *args,
                                         char *result, unsigned long *length, char *is_null, char *error)
{
    std::string most_freq;
    long long most_cnt = 0;
    long long items_total = 0;
    common_str_count_map(initid, args, &most_freq, &most_cnt, &items_total);
    *is_null = 0;
    *length = most_freq.length();
    sprintf(result, most_freq.c_str());
    return result;
}

extern "C" my_bool most_cnt_as_list_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    return common_str_count_map_init(initid, args, message);
}

extern "C" void most_cnt_as_list_deinit(UDF_INIT *initid)
{
    common_str_count_map_deinit(initid);
}

extern "C" void most_cnt_as_list_clear(UDF_INIT *initid,
                                       unsigned char *, unsigned char *)
{
    common_str_count_map_clear(initid);
}

extern "C" void most_cnt_as_list_add(UDF_INIT *initid, UDF_ARGS *args,
                                     unsigned char *, unsigned char *)
{
    common_str_count_map_add(initid, args, LIST_AS_STR);
}

extern "C" long long most_cnt_as_list(UDF_INIT *initid, UDF_ARGS *args,
                                      char *result, unsigned long *length, char *is_null, char *error)
{
    std::string most_freq;
    long long most_cnt = 0;
    long long items_total = 0;
    common_str_count_map(initid, args, &most_freq, &most_cnt, &items_total);
    *is_null = 0;
    return most_cnt;
}

extern "C" my_bool most_freq_as_set_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    return common_str_count_map_init(initid, args, message);
}

extern "C" void most_freq_as_set_deinit(UDF_INIT *initid)
{
    common_str_count_map_deinit(initid);
}

extern "C" void most_freq_as_set_clear(UDF_INIT *initid,
                                       unsigned char *, unsigned char *)
{
    common_str_count_map_clear(initid);
}

extern "C" void most_freq_as_set_add(UDF_INIT *initid, UDF_ARGS *args,
                                     unsigned char *, unsigned char *)
{
    common_str_count_map_add(initid, args, SET_AS_STR);
}

extern "C" const char *most_freq_as_set(UDF_INIT *initid, UDF_ARGS *args,
                                        char *result, unsigned long *length, char *is_null, char *error)
{
    std::string most_freq;
    long long most_cnt = 0;
    long long items_total = 0;
    common_str_count_map(initid, args, &most_freq, &most_cnt, &items_total);
    *is_null = 0;
    *length = most_freq.length();
    sprintf(result, most_freq.c_str());
    return result;
}

extern "C" my_bool most_cnt_as_set_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    return common_str_count_map_init(initid, args, message);
}

extern "C" void most_cnt_as_set_deinit(UDF_INIT *initid)
{
    common_str_count_map_deinit(initid);
}

extern "C" void most_cnt_as_set_clear(UDF_INIT *initid,
                                      unsigned char *, unsigned char *)
{
    common_str_count_map_clear(initid);
}

extern "C" void most_cnt_as_set_add(UDF_INIT *initid, UDF_ARGS *args,
                                    unsigned char *, unsigned char *)
{
    common_str_count_map_add(initid, args, SET_AS_STR);
}

extern "C" long long most_cnt_as_set(UDF_INIT *initid, UDF_ARGS *args,
                                     char *result, unsigned long *length, char *is_null, char *error)
{
    std::string most_freq;
    long long most_cnt = 0;
    long long items_total = 0;
    common_str_count_map(initid, args, &most_freq, &most_cnt, &items_total);
    *is_null = 0;
    return most_cnt;
}
