#include "api.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "utils.hpp"

#include <cassert>
#include <iostream>
#include <iomanip>
#include <string_view>


using namespace NCompileTimeJsonParser;


auto TestErrors() -> void {
    constexpr auto str =
    "{\n"
    "\"aba\": 1,\n"
    "    \"caba\": [2, 3]\n"
    "}";
    constexpr auto mapping = TJsonValue{str}.AsMapping();

    auto it = mapping.begin();
    auto [k, v] = *it;
    auto result = v.AsArray();
    // NError::PrintErrAtCompileTime<result.Error()>{};
    std::cout << result.Error() << "\n";
    auto val = mapping.At("caba");
    auto err = val.AsDouble().Error();
    std::cout << err << "\n";
}


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
    static_assert(arr.At(1).AsArray().Value().At(3).HasError());
    static_assert(arr.At(0).AsArray().HasError());
    constexpr auto err = arr.At(0).AsArray().Error();
    std::cout << err.LineNumber << " " << err.Position << " " << static_cast<int>(err.Code) << "\n";
    
    static_assert(arr.At(1).AsArray().At(0).AsInt() == 1);
    static_assert(arr.At(1).AsArray().At(1).AsInt() == 2); 
    {
        constexpr auto cabaValue =  arr.At(1).AsArray().At(2).AsMapping().At("caba");
        static_assert(cabaValue.AsArray().At(0).AsInt() == 4);
        static_assert(cabaValue.AsArray().At(1).AsInt() == 5);
        static_assert(cabaValue.AsArray().At(2).AsInt() == 6);
        static_assert(
            arr.At(1).AsArray().At(2).AsMapping().At("aba").AsArray().Error()
            ==
            NError::TError{.LineNumber = 2, .Position = 19, .Code = NError::ErrorCode::TypeError}
        );
        static_assert(
            cabaValue.AsMapping().Error()
            ==
            NError::TError{.LineNumber = 2, .Position = 31, .Code = NError::ErrorCode::TypeError}
        );
        static_assert(
            arr.At(1).AsArray().At(2).AsMapping().At("caba").AsMapping().Error()
            ==
            NError::TError{.LineNumber = 2, .Position = 31, .Code = NError::ErrorCode::TypeError}
        );
    }

    {
        constexpr auto arr2 = arr.At(2).AsArray();
        static_assert(arr2.At(0).AsString() == String{"abacaba"});
        static_assert(arr2.At(1).AsInt() == -789);
    }

    static_assert(arr.At(3).AsInt() == -200);

    // Iterators:

    static_assert(std::forward_iterator<TJsonArray::Iterator>);
    static_assert(std::forward_iterator<TJsonMapping::Iterator>);

    {
        // runtime
        auto mapping = arr.At(1).AsArray().At(2).AsMapping();
        for (auto&& [k, v] : mapping) {
            assert(k.Value().ends_with("aba"));
            assert(v.AsInt().HasValue() || v.AsArray().HasValue());
        }
    }
 
    TestErrors();
}
