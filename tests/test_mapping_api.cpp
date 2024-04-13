#include "../parser.hpp"

#include <cassert>
#include <iostream>


using namespace NCompileTimeJsonParser;


auto TestMappingAPI() -> void {
    static constexpr auto json = TJsonValue{
        "{                                      \n"
        "    \"aba\": \"caba\",                       \n"
        "    1: \"daba\",                             \n"
        "    \"lst\" : [1, 2, \"fizz\", 4, \"buzz\"], \n"
        "    \"dct\" : {                              \n"
        "        \"foo\": 3,                          \n"
        "        \"bar\": 5,                          \n"
        "        \"baz\": \"fizz\",                   \n"
        "    },                                       \n"
        "}                                              "
    };
    static constexpr auto map = json.AsMapping();

    // Use `operator[]` to access values by key
    // Works in O({length of underlying string representation})
    static_assert(map["aba"].AsString() == "caba");

    {   // Iterate over (key, value) pairs
        // Also works in O({length of underlying string representation})
        int nChecks = 0;
        for (const auto [k, v] : map) {
            if (k.HasValue() && k.Value() == "aba") {
                assert(v.AsString() == "caba");
                ++nChecks;
            } else if (k.HasError()) { // `k` == 1, `v` == "daba": only strings are allowed to be keys in json maps
                assert(v.AsString() == "daba");
                assert(k.Error() == Error(
                    TLinePositionCounter{.LineNumber = 2, .Position = 4},
                    NError::ErrorCode::TypeError
                ));
                ++nChecks;
            } else if (k.HasValue() && k.Value() == "lst") {
                assert(v.AsArray().HasValue());
                ++nChecks;
            } else {
                assert(k.Value() == "dct");
                assert(v.AsMapping().HasValue());
                ++nChecks;
            }
        }
        assert(nChecks == 4);
    }

    // We can access nested maps and arrays without explicitly
    // casting `TJsonValue` to `TJsonMapping` or `TJsonArray`:
    static_assert(map["lst"][2].AsString() == "fizz");
    static_assert(map["dct"]["bar"].AsInt() == 5);
}
