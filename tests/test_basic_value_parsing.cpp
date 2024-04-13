#include "../parser.hpp"


using namespace NCompileTimeJsonParser;


auto TestBasicValueParsing() -> void { 
    {   // Int
        static_assert(TJsonValue{"12345"}.AsInt() == 12345);
        static_assert(TJsonValue{"-54321"}.AsInt() == -54321);
        static_assert(TJsonValue{"0"}.AsInt() == 0);
        static_assert(TJsonValue{"-0"}.AsInt() == 0);
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
                TJsonValue{"000.12131415"}.AsDouble().Value() - 0.12131415
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
        static_assert(arr[0].AsInt() == 1);
        static_assert(arr[1].AsInt() == 2);
        static_assert(arr[2].AsInt() == 3);
    }

    {   // Mapping
        constexpr auto map = TJsonValue{"{\"aba\": 4, \"caba\": 5}"}.AsMapping();
        static_assert(map["aba"].AsInt() == 4);
        static_assert(map["caba"].AsInt() == 5);
    }
}
