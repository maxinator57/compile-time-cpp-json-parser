#include "../parser.hpp"

#include <cassert>
#include <sstream>


using namespace NCompileTimeJsonParser;


auto TestBasicErrorHandling() -> void {
    static constexpr auto json = TJsonValue{
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

    {   // When trying to parse json value as a wrong type, we get a `TypeError`:
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
            params["interpreters"][0]["name"].AsString(); // the requested key "interpreters" 
                                                          // doesn't exist in the mapping
        static_assert(wrongLookup.HasError());
        static_assert(wrongLookup.Error() == NError::TError{
            .BasicInfo = {
                .LineNumber = 5,
                .Position = 15,
                .Code = NError::ErrorCode::MappingKeyNotFound
            },
            .AdditionalInfo = NError::TMappingKeyNotFoundAdditionalInfo{"interpreters"},
        }); 
        // Line 5, position 15 is the place where the "params" json mapping starts
        const auto errorMessage = (std::stringstream{} << wrongLookup.Error()).str();
        assert(errorMessage ==
            "Got \"mapping key not found\" error (key \"interpreters\" doesn't exist in mapping) at line 5, position 15"
        );
    }
}
