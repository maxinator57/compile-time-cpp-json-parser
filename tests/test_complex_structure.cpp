#include "../parser.hpp"


using namespace NJsonParser;


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
        // Same operations, shorter syntax
        static_assert(
            json["data"][1]["x"].AsInt() == 57
        );
    }

    {   // Traverse maps and arrays in familiar ways
        constexpr auto compilers = [json]() {
            auto compilers = std::array<std::string_view, 2>{};
            size_t i = 0;
            constexpr auto compilersInfo = json["params"]["compilers"].AsArray();
            for (const auto info : compilersInfo) {
                compilers[i] = info["name"].AsString().Value();
                ++i;
            }
            return compilers;
        }();
        static_assert(compilers == std::array<std::string_view, 2>{"clang", "gcc"});
    }

    {   // Safely lookup in maps and arrays and continue without the need to check for errors on each level
        constexpr auto wrongLookup = json["non-existent_key"][42].AsString();
        // It is enough to check for errors only at the very end (thanks to monadic operations)
        static_assert(wrongLookup.HasError());

        constexpr auto rightLookup = json["params"]["cpp_standard"].AsInt();
        static_assert(rightLookup.HasValue());
        // returns a `const NJsonParser::Int&`
        static_assert(std::is_same_v<decltype(rightLookup.Value()), const NJsonParser::Int&>);
        static_assert(rightLookup.Value() == 20);
    }
}