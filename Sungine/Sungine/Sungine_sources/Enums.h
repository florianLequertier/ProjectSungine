#pragma once

#include <vector>
#include <string>

#include "boost/preprocessor/variadic.hpp"
#include "boost/preprocessor/stringize.hpp"
#include "boost/preprocessor/tuple.hpp"

namespace Enum {

template<typename T>
const std::vector<std::string>& EnumNames() {}
template<typename T>
bool GetEnumName(void* data, int idx, const char** outText) {}

#define ENUM(ENUM_NAME, ...)\
enum EmissionType {BOOST_PP_TUPLE_REM(BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))};\
template<>\
const std::vector<std::string>& EnumNames<ENUM_NAME>() { static std::vector<std::string> value = { BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_REM(BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))) }; return value; }\
template<>\
bool GetEnumNames<ENUM_NAME>(void* data, int idx, const char** outText) { return EnumNames<ENUM_NAME>()[idx].c_str(); }

}
