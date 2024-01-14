#include "parser.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>


using namespace NCompileTimeJsonParser;


auto TestBasicJsonValueParsing() -> void { 
    {   // Int
        static_assert(TJsonValue{"12345"}.AsInt() == 12345);
        static_assert(TJsonValue{"-54321"}.AsInt() == -54321);
    }
     
    {   // Double
        constexpr auto kEps = 1e-9;
        static_assert(
            std::abs(
                TJsonValue{"12345.67891011"}.AsDouble().Value() - 12345.67891011
            ) < kEps 
        );
        static_assert(
            std::abs(
                TJsonValue{"0.12131415"}.AsDouble().Value() - 0.12131415
            ) < kEps
        );
        static_assert(
            std::abs(
                TJsonValue{"-16.17181920"}.AsDouble().Value() - (-16.17181920)
            ) < kEps
        );
        static_assert(
            std::abs(
                TJsonValue{"12345"}.AsDouble().Value() - 12345
            ) < kEps
        );
    }
     
    {   // String
        static_assert(TJsonValue{"\"abacaba\""}.AsString() == "abacaba");
        static_assert(TJsonValue{"\"\""}.AsString() == "");
    }
     
    {   // Array
        constexpr auto arr = TJsonValue{"[1, 2, 3]"}.AsArray();
        static_assert(arr.At(0).AsInt() == 1);
        static_assert(arr.At(1).AsInt() == 2);
        static_assert(arr.At(2).AsInt() == 3);
    }

    {   // Mapping
        constexpr auto map = TJsonValue{"{\"aba\": 4, \"caba\": 5}"}.AsMapping();
        static_assert(map.At("aba").AsInt() == 4);
        static_assert(map.At("caba").AsInt() == 5);
    }
}


auto TestComplexStructure() -> void {
    constexpr auto json = TJsonValue{
        "{                                                            \n"
        "    \"data\": [                                              \n"
        "        {\"aba\": 1, \"caba\": 2},                           \n"
        "        {\"x\": 57, \"y\": 179},                             \n"
        "    ],                                                       \n"
        "    \"params\": {                                            \n"
        "        \"cpp_standard\": 20,                                \n"
        "        \"compilers\": [                                     \n"
        "            {\"name\": \"clang\", \"version\" : \"14.0.0\"}, \n"
        "            {\"version\" : \"11.4.0\", \"name\": \"gcc\"},   \n"
        "        ]                                                    \n"
        "    }                                                        \n"
        "}                                                            \n"
    };

    {   // Easily navigate a complex structure
        static_assert(
            json.AsMapping().At("data").AsArray().At(1).AsMapping().At("x").AsInt() == 57
        );
    }

    {   // Traverse maps and arrays in familiar ways
        constexpr auto supportedCompilers = [json]() {
            auto compilerNames = std::array<std::string_view, 2>{};
            size_t i = 0;
            constexpr auto compilersInfo = json.AsMapping().At("params").AsMapping().At("compilers").AsArray();
            for (auto&& info : compilersInfo) {
                compilerNames[i] = info.AsMapping().At("name").AsString().Value();
                ++i;
            }
            return compilerNames;
        }();
        static_assert(supportedCompilers == std::array<std::string_view, 2>{"clang", "gcc"});
    }

    {   // Safely lookup in maps and arrays and continue without the need to check for errors on each level
        constexpr auto wrongLookup = json.AsMapping().At("non-existent_key").AsArray().At(42).AsString();
        // It is enough to check for errors only at the very end:
        static_assert(wrongLookup.HasError());

        constexpr auto rightLookup = json.AsMapping().At("params").AsMapping().At("cpp_standard").AsInt();
        static_assert(rightLookup.HasValue());
        // returns a `const int64_t&`
        static_assert(std::is_same_v<decltype(rightLookup.Value()), const int64_t&>);
        static_assert(rightLookup.Value() == 20);
    }
}


auto TestErrorHandling() -> void {

}


auto main() -> int {
    TestBasicJsonValueParsing();
    TestComplexStructure();
    TestErrorHandling();
}
