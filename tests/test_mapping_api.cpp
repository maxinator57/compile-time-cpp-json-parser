#include "../parser.hpp"

#include <cassert>


using namespace NJsonParser;


auto TestMappingAPI() -> void {
    static constexpr auto json = JsonValue{
        "{                                      \n"
        "    \"aba\": \"caba\",                       \n"
        "    \"lst\" : [1, 2, \"fizz\", 4, \"buzz\"], \n"
        "    \"dct\" : {                              \n"
        "        \"foo\": 3,                          \n"
        "        \"bar\": 5,                          \n"
        "        \"baz\": \"fizz\",                   \n"
        "    },                                       \n" 
        "    1: \"daba\",                             \n"
        "}                                              "
    };
    static constexpr auto map = json.As<Mapping>();

    {   // Use `operator[]` to access values by key
        // Works in O({length of underlying string representation})
        static_assert(map["aba"].As<String>() == "caba");
        // We can access nested maps and arrays without explicitly
        // casting `JsonValue` to `Mapping` or `Array`
        static_assert(map["lst"][2].As<String>() == "fizz");
        static_assert(map["dct"]["bar"].As<Int>() == 5);
    }

    {   // Iterate over json maps in usual ways.
        // `JsonMap` provides `.begin()` and `.end()` iterators of type `Mapping::Iterator`,
        // which is guaranteed to be at least a `forward_iterator`
        static_assert(std::forward_iterator<Mapping::Iterator>);
    }

    {   // Iterate over (key, value) pairs
        // Also works in O({length of underlying string representation})
        int nChecks = 0;
        for (const auto [k, v] : map) {
            if (k.HasValue() && k.Value() == "aba") {
                assert(v.As<String>() == "caba");
                ++nChecks;
            } else if (k.HasError()) {
                // `k` == 1, `v` == "daba": only strings are allowed as keys in json maps
                assert(v.As<String>() == "daba");
                assert(k.Error() == MakeError(
                    LinePositionCounter{.LineNumber = 8, .Position = 4},
                    NError::ErrorCode::TypeError,
                    "expected string, got something else"
                ));
                ++nChecks;
            } else if (k.HasValue() && k.Value() == "lst") {
                assert(v.As<Array>().HasValue());
                ++nChecks;
            } else {
                assert(k.Value() == "dct");
                assert(v.As<Mapping>().HasValue());
                ++nChecks;
            }
        }
        assert(nChecks == 4);
    } 
}
