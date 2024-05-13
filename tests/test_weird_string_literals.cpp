#include "../parser.hpp"


using namespace NJsonParser;


auto TestWeirdStringLiterals() -> void {
    {
        constexpr auto jsonMap = JsonValue{
        "{                      \n"
        "    \"aba[{\": \"aba\",\n"
        "}                      \n"
        };
        constexpr auto aba = jsonMap["aba[{"];
        static_assert(aba.As<String>() == "aba");
    }
    {
        constexpr auto jsonArr = JsonValue{
            "[\n"
            "    \"aba[{\","
            "]"
        };
        constexpr auto aba = jsonArr[0];
        static_assert(aba.As<String>() == "aba[{");
    }
}
