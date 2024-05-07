#include "../parser.hpp"


using namespace NJsonParser;


constexpr auto Example() -> void {
    constexpr auto json = TJsonValue{
    /* line number */
    /*      0      */  "{                                           \n"
    /*      1      */  "    \"aba\": 1,                             \n"
    /*      2      */  "    \"caba\": [1, 2, \"fizz\", 4, \"buzz\"] \n"
    /*      3      */  "}                                           \n"
    };

    // Try to read a value of type `Int` from json mapping by key:
    constexpr auto aba = json["aba"].AsInt();
    // The type of `aba` is not `Int`, but rather `TExpected<Int>`,
    // because the attempt to read the value as an `Int` could be unsuccessful:
    static_assert(std::same_as<decltype(aba), const TExpected<Int>>);
    // Check that the read was successful by calling the `.HasValue()` method
    // on an instance of `TExpected`:
    static_assert(aba.HasValue());
    // Another way to do the same check is to call the `.HasError()` method
    // and negate the result:
    static_assert(!aba.HasError());
    // Get the actual value by calling the `.Value()` method
    // on an instance of `TExpected`:
    static_assert(aba.Value() == 1);
    // Equality comparison can be done without the need to exctract
    // the value from an instance of `TExpected`:
    static_assert(aba == 1);

    // Trey to read an array from json mapping by key:
    constexpr auto caba = json["caba"].AsArray();
    // Similarly to the first example, the type of `caba` is `TExpected<TJsonArray>`
    static_assert(std::same_as<decltype(caba), const TExpected<TJsonArray>>);
    static_assert(caba.HasValue());
    // `TExpected<TJsonArray>` has the `.size()` method, which returns `TExpected<size_t>`:
    static_assert(std::same_as<decltype(caba.size()), TExpected<size_t>>);
    static_assert(caba.size() == 5);

    // The same can be done with strings:
    constexpr auto fizz = json["caba"][2].AsString();
    static_assert(std::same_as<decltype(fizz), const TExpected<String>>);
    static_assert(fizz.HasValue());
    static_assert(fizz.Value() == "fizz");
    static_assert(fizz == "fizz");

    // Errors are represented by the struct `TError`:
    constexpr auto fizzError = json["caba"][2].AsInt();
    static_assert(fizzError.HasError());
    static_assert(fizzError.Error() == NError::TError{
        // Basic info includes the line number and position where the error occurred
        // as well as the error code:
        .BasicInfo = {
            .LineNumber = 2,
            .Position = 19, // points at the start of the string "fizz"
            .Code = NError::ErrorCode::TypeError,
        },
        // Additional info can contain basically any other useful information
        // about the error, if applicable:
        .AdditionalInfo = "expected int, got something else",
    });

    // Another example of error handling:
    constexpr auto fizzbuzz = json["caba"][14].AsString();
    static_assert(fizzbuzz.HasError());
    static_assert(fizzbuzz.Error() == NError::TError{
        .BasicInfo = {
            .LineNumber = 2,
            .Position = 12, // points at the start of the array (an opening square bracket '[')
            .Code = NError::ErrorCode::ArrayIndexOutOfRange,
        },
        // Additional info here has a different form
        .AdditionalInfo = NError::TArrayIndexOutOfRangeAdditionalInfo{
            .Index = 14,
            .ArrayLen = 5,
        },
    });

    // Maps and arrays provide their own iterators, which are guaranteed to be at least `forward`:
    constexpr auto first = json.AsMapping().begin();
    static_assert(std::same_as<decltype(first), const TJsonMapping::Iterator>);
    static_assert(std::forward_iterator<TJsonArray::Iterator>);
    static_assert(std::forward_iterator<TJsonMapping::Iterator>);

    // Both iterators provide the dereference operator `*`:
    static_assert(std::same_as<decltype(*first), TJsonMapping::Iterator::value_type>);
    static_assert(std::same_as<decltype((*first).Key), TExpected<String>>);
    static_assert((*first).Key == "aba");
    static_assert(std::same_as<decltype((*first).Value), TExpected<TJsonValue>>);
    static_assert((*first).Value.AsInt() == 1);

    constexpr auto second = std::next(first);
    static_assert((*second).Key == "caba");
    static_assert((*second).Value.AsArray().size() == 5);

    // It is possible to iterate over the elements of a json array or mapping.
    // All types used in this json parser are lightweight and have value semantics,
    // therefore, it's better to pass them by value everywhere.
    for (const auto [k, v] : json.AsMapping()) {
        static_assert(std::same_as<decltype(k), const TExpected<String>>);
        static_assert(std::same_as<decltype(v), const TExpected<TJsonValue>>);
    }
}

auto main() -> int { Example(); }
