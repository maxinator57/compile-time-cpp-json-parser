#include "parser.hpp"

#include <cstdlib>
#include <numeric>
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
        "{                                                           \n"
        "    \"data\": [                                             \n"
        "        {\"aba\": 1, \"caba\": 2},                          \n"
        "        {\"x\": 57, \"y\": 179},                            \n"
        "    ],                                                      \n"
        "    \"params\": {                                           \n"
        "        \"cpp_standard\": 20,                               \n"
        "        \"compilers\": [                                    \n"
        "            {\"name\": \"clang\", \"version\": \"14.0.0\"}, \n"
        "            {\"version\": \"11.4.0\", \"name\": \"gcc\"},   \n"
        "        ]                                                   \n"
        "    }                                                       \n"
        "}                                                           \n"
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


auto TestBasicErrorHandling() -> void {
    constexpr auto json = TJsonValue{
    /* line numbers: */
    /* 0  */ "{                                                           \n"
    /* 1  */ "    \"data\": [                                             \n"
    /* 2  */ "        {\"aba\": 1, \"caba\": 2},                          \n"
    /* 3  */ "        {\"x\": 57, \"y\": 179},                            \n"
    /* 4  */ "    ],                                                      \n"
    /* 5  */ "    \"params\": {                                           \n"
    /* 6  */ "        \"cpp_standard\": 20,                               \n"
    /* 7  */ "        \"compilers\": [                                    \n"
    /* 8  */ "            {\"name\": \"clang\", \"version\": \"14.0.0\"}, \n"
    /* 9  */ "            {\"version\": \"11.4.0\", \"name\": \"gcc\"},   \n"
    /* 10 */ "        ]                                                   \n"
    /* 11 */ "    }                                                       \n"
    /* 12 */ "}                                                           \n"
    };

    {   // When trying to parse json value as a wrong type, we get a TypeError:
        constexpr auto asWrongType = json.AsArray();
        static_assert(asWrongType.HasError());
        static_assert(asWrongType.Error() == NError::TError{
            .BasicInfo = { 
                .LineNumber = 0,
                .Position = 0, // points at the first symbol of the underlying data,
                               // not the internal data of the mapping object itself
                               // (which starts just one more position to the right)
                .Code = NError::ErrorCode::TypeError,
            },
            .AdditionalInfo = "either both square brackets are missing or the "
                       "underlying data does not represent an array"
        });
    }

    {   // Lookups made after the first time an error occurs keep the information about this error:
        constexpr auto params = json.AsMapping().At("params").AsMapping();
        // No error yet:
        static_assert(!params.HasError()); // equivalent to params.HasValue():
        static_assert(params.HasValue());

        // Now perform some operations that would result in an error:
        constexpr auto wrongLookup =
            params.At("interpreters").AsArray().At(0).AsMapping().At("name").AsString(); // "interpreters" is a wrong key
        static_assert(wrongLookup.HasError());
        static_assert(wrongLookup.Error() == NError::TError{
            .BasicInfo = {
                .LineNumber = 5,
                .Position = 15, // points at the starting position of the "params" mapping object 
                .Code = NError::ErrorCode::MappingKeyNotFound
            },
            .AdditionalInfo = "interpreters", // the requested key that doesn't exist in the mapping
        });
    }
}

auto TestArray() -> void {
    constexpr auto json = TJsonValue{
        /* line numbers: */
        /* 0 */ "[                  \n"
        /* 1 */ "    [1, 2, 3],   \n"
        /* 2 */ "    [4],         \n"
        /* 3 */ "    [5, 6],      \n"
        /* 4 */ "    [7, 8, 9},   \n"
        /* 5 */ "]                \n"
    };

    // Although we have a syntax error on line 4 (4-th array ends
    // with a curly brace ('}') instead of square bracket (']')),
    // reading the first three arrays is fine:
    
    {   // Compute the sum of elements of the first array using `std::accumulate`:
        constexpr auto zeroth = json.AsArray().At(0).AsArray();
        constexpr auto zerothSum = std::accumulate(
            zeroth.begin(), zeroth.end(), 0,
            [](int sum, auto&& arrayElem){
                return sum + arrayElem.AsInt().Value(); // unsafe to call `.Value()` without
                                                        // checking for `.HasValue()` first,
                                                        // but will do for a demo
        });
        static_assert(zerothSum == 6);
    }

    {
        constexpr auto first = json.AsArray().At(1).AsArray();
        static_assert(first.At(0).AsInt() == 4);
        static_assert(first.At(1).HasError());
        static_assert(first.At(1).Error() == NError::TError{
            .BasicInfo = {
                .LineNumber = 2,
                .Position = 4, // points at the start of the array (an opening square bracket ('['))
                .Code = NError::ErrorCode::ArrayIndexOutOfRange,
            },
            .AdditionalInfo = NError::TArrayIndexOutOfRangeAdditionalInfo{
                .Index = 1,
                .ArrayLen = 1,
            },
        });
    }

    {
        constexpr auto third = json.AsArray().At(3).AsArray();
        static_assert(third.HasError());
        // In fact, the error emitted in this example should rather be
        // "SyntaxError (a closing square bracket is probably missing
        // at the end of an array)", but, due to the architecture of
        // this json parser, it's virtually impossible for it to make
        // such a guess. So we have to be content with at least some
        // useful information in this case (although not particularly
        // helpful)
        static_assert(third.Error() == NError::TError{
            .BasicInfo = {
                .LineNumber = 4,
                .Position = 12,
                .Code = NError::ErrorCode::SyntaxError
            },
            .AdditionalInfo = "brackets mismatch: encountered an excess '}'",
        });
    }

    // This error actually emerges not at the last step (when 
    // calling `.AsArray()` for the second time), but rather
    // when invoking the `.At()` method:
    constexpr auto arr = json.AsArray();
    static_assert(arr.HasValue());
    constexpr auto third = arr.At(3);
    static_assert(third.HasError());
    static_assert(third.Error() == NError::TError{
        .BasicInfo = {
            .LineNumber = 4,
            .Position = 12,
            .Code = NError::ErrorCode::SyntaxError
        },
        .AdditionalInfo = "brackets mismatch: encountered an excess '}'",
    });
}


auto main() -> int {
    TestBasicErrorHandling();
    TestArray();
}
