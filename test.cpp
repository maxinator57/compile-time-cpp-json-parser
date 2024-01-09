#include "api.hpp"
#include "parser.hpp"
#include "utils.hpp"

#include <cassert>
#include <iostream>
#include <iomanip>
#include <string_view>


auto main() -> int {
    using namespace NCompileTimeJsonParser;

    constexpr auto str =
    "[\n"
    "                    123456789.10111213,         \n"
    "     [1,2, {\"aba\": 3, \"caba\" : [4, 5, 6]}], \n"
    "   [\"abacaba\",                                \n"
    "      -789]       ,                             \n"
    "                      -200,                     \n"
    "]";

    constexpr auto arr = TJsonValue{str}.AsArray();
    static_assert(arr.HasValue());
    static_assert(arr.At(0).AsDouble() == 123456789.10111213);
    static_assert(arr.At(1).AsArray().Value().At(0).AsInt() == 1);
    static_assert(arr.At(1).AsArray().Value().At(1).AsInt() == 2);
    static_assert(arr.At(0).AsArray().HasError());
    constexpr auto err = arr.At(0).AsArray().Error();
    std::cout << err.LineNumber << " " << err.Position << " " << static_cast<int>(err.Code) << "\n";
    // std::cout << arr.Value().GetData() << "\n";
    // static_assert(arr.At(0).AsDouble() == 123456789.101112131415);
    
    // static_assert(arr.At(1).AsArray().At(0).AsInt64() == 1);
    // static_assert(arr.At(1).AsArray().At(1).AsInt64() == 2);
    // static_assert(arr.At(1).AsArray().At(2).AsMapping().At("aba").AsInt64() == 3);
    // {
    //     constexpr auto cabaValue =  arr.At(1).AsArray().At(2).AsMapping().At("caba").AsArray();
    //     static_assert(cabaValue.At(0).AsInt64() == 4);
    //     static_assert(cabaValue.At(1).AsInt64() == 5);
    //     static_assert(cabaValue.At(2).AsInt64() == 6);
    // }

    // {
    //     constexpr auto arr2 = arr.At(2).AsArray();
    //     static_assert(arr2.At(0).AsString() == "abacaba");
    //     static_assert(arr2.At(1).AsInt64() == -789);
    // }

    // static_assert(arr.At(3).AsInt64() == -200);

    // // Iterators:

    // static_assert(std::forward_iterator<TJsonArray::Iterator>);
    // static_assert(std::forward_iterator<TJsonMapping::Iterator>);

    // {
    //     // runtime
    //     auto mapping = arr.At(1).AsArray().At(2).AsMapping();
    //     for (auto&& [k, v] : mapping) {
    //         assert(k.ends_with("aba"));
    //         assert(v.AsInt64().has_value() || v.AsArray().has_value());
    //     }
    // } 
}
