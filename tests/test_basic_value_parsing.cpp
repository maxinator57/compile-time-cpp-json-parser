#include "../parser.hpp"

#include <cassert>


using namespace NJsonParser;


auto TestBasicValueParsing() -> void {
    {   // Bool

        // Compile-time
        static_assert(JsonValue{"true"}.As<Bool>() == true);
        static_assert(JsonValue{"false"}.As<Bool>() == false);
        static_assert(
            JsonValue{"True"}.As<Bool>().Error().BasicInfo.Code
            == NError::ErrorCode::TypeError
        );
        static_assert(
            JsonValue{""}.As<Bool>().Error().BasicInfo.Code
            == NError::ErrorCode::MissingValueError
        );

        // Run-time
        assert(JsonValue{"true"}.As<Bool>() == true);
        assert(JsonValue{"false"}.As<Bool>() == false);
        assert(
            JsonValue{"True"}.As<Bool>().Error().BasicInfo.Code
            == NError::ErrorCode::TypeError
        );
        assert(
            JsonValue{""}.As<Bool>().Error().BasicInfo.Code
            == NError::ErrorCode::MissingValueError
        );
    }
    {   // Int

        // Compile-time:
        static_assert(JsonValue{"12345" }.As<Int>() == 12345 );
        static_assert(JsonValue{"-54321"}.As<Int>() == -54321);
        static_assert(JsonValue{"0"     }.As<Int>() == 0     );
        static_assert(JsonValue{"-0"    }.As<Int>() == 0     );

        // Run-time:
        assert(JsonValue{"12345" }.As<Int>() == 12345 );
        assert(JsonValue{"-54321"}.As<Int>() == -54321);
        assert(JsonValue{"0"     }.As<Int>() == 0     );
        assert(JsonValue{"-0"    }.As<Int>() == 0     );

        // When an integral value can't be represented by the `Int` type used by library,
        // a `ResultOutOfRange` error is returned:
        const auto err = JsonValue{"12345678910111213141516171819202122"}.As<Int>();
        assert(err.HasError());
        assert(err.Error().BasicInfo.Code == NJsonParser::NError::ErrorCode::ResultOutOfRangeError);
    }
     
    {   // Float
        constexpr auto kEps = 1e-9;

        // Compile-time: 
        static_assert(std::abs(JsonValue{"12345.67891011"}.As<Float>().Value() - 12345.67891011) < kEps);
        static_assert(std::abs(JsonValue{"000.12131415"  }.As<Float>().Value() - 0.12131415    ) < kEps);
        static_assert(std::abs(JsonValue{"-16.17181920"  }.As<Float>().Value() - (-16.17181920)) < kEps);
        static_assert(std::abs(JsonValue{"12345"         }.As<Float>().Value() - 12345         ) < kEps);

        // Run-time:
        assert(std::abs(JsonValue{"12345.67891011"}.As<Float>().Value() - 12345.67891011) < kEps);
        assert(std::abs(JsonValue{"000.12131415"  }.As<Float>().Value() - 0.12131415    ) < kEps);
        assert(std::abs(JsonValue{"-16.17181920"  }.As<Float>().Value() - (-16.17181920)) < kEps);
        assert(std::abs(JsonValue{"12345"         }.As<Float>().Value() - 12345         ) < kEps);
    }
     
    {   // String
        // Compile-time:
        static_assert(JsonValue{"\"abacaba\""}.As<String>() == "abacaba");
        static_assert(JsonValue{"\"\""       }.As<String>() == ""       );

        // Run-time:
        assert(JsonValue{"\"abacaba\""}.As<String>() == "abacaba");
        assert(JsonValue{"\"\""       }.As<String>() == ""       );
    }
     
    {   // Array
        {   // Compile-time
            constexpr auto arr = JsonValue{"[1, 2, 3]"};
            static_assert(arr[0].As<Int>() == 1);
            static_assert(arr[1].As<Int>() == 2);
            static_assert(arr[2].As<Int>() == 3);
        }
        {   // Run-time
            const auto arr = JsonValue{"[1, 2, 3]"};
            assert(arr[0].As<Int>() == 1);
            assert(arr[1].As<Int>() == 2);
            assert(arr[2].As<Int>() == 3);
        }
    }

    {   // Mapping
        {   // Compile-time
            constexpr auto map = JsonValue{"{\"aba\": 4, \"caba\": 5}"};
            static_assert(map["aba" ].As<Int>() == 4);
            static_assert(map["caba"].As<Int>() == 5);
        }
        {   // Run-time
            const auto map = JsonValue{"{\"aba\": 4, \"caba\": 5}"};
            assert(map["aba" ].As<Int>() == 4);
            assert(map["caba"].As<Int>() == 5);
        }
    }
}
