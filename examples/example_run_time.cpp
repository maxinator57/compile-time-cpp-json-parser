#include "../parser.hpp"

#include <cassert>
#include <vector>


using namespace NJsonParser;


constexpr auto Example() -> void {
    const auto json = TJsonValue{
    /* line number */
    /*      0      */  "{                                           \n"
    /*      1      */  "    \"aba\": 1,                             \n"
    /*      2      */  "    \"caba\": [1, 2, \"fizz\", 4, \"buzz\"] \n"
    /*      3      */  "}                                           \n"
    };

    // Try to read a value of type `Int` from json mapping by key:
    const auto aba = json["aba"].AsInt();
    assert(aba.HasValue());
    // Another way to do the same check is to call the `.HasError()` method
    // and negate the result:
    assert(!aba.HasError());
    // Get the actual value by calling the `.Value()` method
    // on an instance of `TExpected`:
    assert(aba.Value() == 1);
    // Equality comparison can be done without the need to exctract
    // the value from an instance of `TExpected`:
    assert(aba == 1);

    // Trey to read an array from json mapping by key:
    const auto caba = json["caba"].AsArray();
    assert(caba.HasValue());
    // `TExpected<TJsonArray>` has the `.size()` method, which returns `TExpected<size_t>`:
    assert(caba.size() == 5);

    // The same can be done with strings:
    const auto fizz = json["caba"][2].AsString();
    assert(fizz.HasValue());
    assert(fizz.Value() == "fizz");
    assert(fizz == "fizz");

    // Errors are represented by the struct `TError`:
    const auto fizzError = json["caba"][2].AsInt();
    assert(fizzError.HasError());
    assert((fizzError.Error() == NError::TError{
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
    }));

    // Another example of error handling:
    const auto fizzbuzz = json["caba"][14].AsString();
    assert(fizzbuzz.HasError());
    assert((fizzbuzz.Error() == NError::TError{
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
    }));

    // Maps and arrays provide their own iterators, which are guaranteed to be at least `forward`:
    const auto first = json.AsMapping().begin();
    const auto [k, v] = *first;
    assert(k.Value() == "aba");
    assert(v.AsInt() == 1);

    const auto second = std::next(first);
    assert((*second).Key == "caba");
    assert((*second).Value.AsArray().size() == 5);

    // It is possible to iterate over the elements of a json array or mapping.
    // All types used in this json parser are lightweight and have value semantics,
    // therefore, it's better to pass them by value everywhere.
    auto keys = std::vector<String>{};
    for (const auto [k, v] : json.AsMapping()) {
        keys.push_back(k.Value());
    }
    assert((keys == std::vector<std::string_view>{"aba", "caba"}));

    auto numbers = std::vector<Int>{};
    auto strings = std::vector<String>{};
    for (const auto elem : json["caba"].AsArray()) {
        if (auto i = elem.AsInt(); i.HasValue()) {
            numbers.push_back(i.Value());
        } else {
            strings.push_back(elem.AsString().Value());
        }
    }
    assert((numbers == std::vector<Int>{1, 2, 4}));
    assert((strings == std::vector<String>{"fizz", "buzz"}));
}

auto main() -> int { Example(); }