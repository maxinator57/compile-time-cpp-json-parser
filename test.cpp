#include "parser.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>


using namespace NCompileTimeJsonParser;


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
            json.AsMapping()["data"].AsArray()[1].AsMapping()["x"].AsInt() == 57
        );
        // Same operations, shorter syntax:
        static_assert(
            json["data"][1]["x"].AsInt() == 57
        );
    }

    {   // Traverse maps and arrays in familiar ways
        constexpr auto supportedCompilers = [json]() {
            auto compilerNames = std::array<std::string_view, 2>{};
            size_t i = 0;
            constexpr auto compilersInfo = json["params"]["compilers"].AsArray();
            for (auto&& info : compilersInfo) {
                compilerNames[i] = info["name"].AsString().Value();
                ++i;
            }
            return compilerNames;
        }();
        static_assert(supportedCompilers == std::array<std::string_view, 2>{"clang", "gcc"});
    }

    {   // Safely lookup in maps and arrays and continue without the need to check for errors on each level
        constexpr auto wrongLookup = json["non-existent_key"][42].AsString();
        // It is enough to check for errors only at the very end:
        static_assert(wrongLookup.HasError());

        constexpr auto rightLookup = json["params"]["cpp_standard"].AsInt();
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
        constexpr auto params = json["params"].AsMapping();
        // No error yet:
        static_assert(!params.HasError()); // equivalent to params.HasValue():
        static_assert(params.HasValue());

        // Now perform some operations that would result in an error:
        constexpr auto wrongLookup =
            params["interpreters"][0]["name"].AsString(); // "interpreters" is a wrong key
        static_assert(wrongLookup.HasError());
        static_assert(wrongLookup.Error() == NError::TError{
            .BasicInfo = {
                .LineNumber = 5,
                .Position = 15, // points at the starting position of the "params" mapping object
                .Code = NError::ErrorCode::MappingKeyNotFound
            },
            .AdditionalInfo = NError::TMappingKeyNotFoundAdditionalInfo{"interpreters"},
                // the requested key "interpreters" that doesn't exist in the mapping
        }); 
        // std::cout << wrongLookup.Error() << "\n"; // uncomment and run to see what it prints
    }
}




auto main() -> int {
    TestBasicErrorHandling();
    TestComplexStructure();
}
