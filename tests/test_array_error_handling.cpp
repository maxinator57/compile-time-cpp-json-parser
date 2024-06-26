#include "../parser.hpp"

#include <cassert>
#include <numeric>
#include <vector>


using namespace NJsonParser;


auto TestArrayErrorHandling() -> void {
    static constexpr auto json = JsonValue{
        /* line numbers: */
        /* 0 */ "[                \n"
        /* 1 */ "    [1, 2, 3],   \n"
        /* 2 */ "    [4],         \n"
        /* 3 */ "    [5, 6],      \n"
        /* 4 */ "    [7, 8, 9},   \n"
        /* 5 */ "]                \n"
    };

    // Although we have a syntax error on line 4 (4-th array ends
    // with a curly brace '}' instead of square bracket ']'),
    // reading the first three arrays is fine.
    static_assert(json.As<Array>().size() == 3);
    
    {   // Compute the sum of elements of the first array using `std::accumulate`
        constexpr auto zeroth = json[0].As<Array>();
        constexpr auto zerothSum = std::accumulate(
            zeroth.begin(), zeroth.end(), 0,
            [](int sum, Expected<JsonValue> arrayElem){
                return sum + arrayElem.As<Int>().Value(); // unsafe to call `.Value()` without
                                                        // checking for `.HasValue()` first,
                                                        // but will do for a demo
        });
        static_assert(zerothSum == 6);
    }

    {
        constexpr auto first = json[1].As<Array>();
        static_assert(first[0].As<Int>() == 4);
        static_assert(first[1].HasError()); // index 1 is out of range for the array `[4]`
        // As shown by error message
        static_assert(first[1].Error() == NError::Error{
            .BasicInfo = {
                .LineNumber = 2,
                .Position = 4, // points at the start of the array (an opening square bracket ('['))
                .Code = NError::ErrorCode::ArrayIndexOutOfRange,
            },
            .AdditionalInfo = NError::ArrayIndexOutOfRangeAdditionalInfo{
                .Index = 1,
                .ArrayLen = 1,
            },
        });
    }

    {
        constexpr auto second = json[2].As<Array>();
        auto vec = std::vector<int>{}; vec.reserve(second.size().Value()); // need to call `.Value()`,
                                                                           // because `secondArr` has type `Expected<Array>`,
                                                                           // not just `Array`
        static_assert(second.size().Value() == second.Value().size()); // it's not important where exactly the unwrapping of `Expected<T>`
                                                                       // to an instance of `T` happens by calling the `.Value()` method
        for (const auto elem : second) {
            vec.push_back(elem.As<Int>().Value());
        }
        assert((vec == std::vector{5, 6}));
    }

    {
        constexpr auto third = json[3].As<Array>();
        static_assert(third.HasError());
        // In fact, the error emitted in this example should rather be
        // something like "SyntaxError (a closing square bracket is 
        // probably missing at the end of an array)", but, due to the
        // architecture of this json parser, it's virtually impossible
        // for it to make such a guess. So we have to be content with at
        // least some useful information in this case (although not 
        // particularly helpful)
        static_assert(third.Error() == NError::Error{
            .BasicInfo = {
                .LineNumber = 4,
                .Position = 12,
                .Code = NError::ErrorCode::SyntaxError
            },
            .AdditionalInfo = "brackets mismatch: encountered an excess '}'",
        });
    }

    {
        // This error actually emerges not at the last step (when 
        // calling `.As<Array>()` for the second time), but rather
        // when invoking the `[]` operator
        constexpr auto arr = json.As<Array>();
        static_assert(arr.HasValue()); // no error yet
        constexpr auto third = arr[3]; // error occurs here
        static_assert(third.HasError());
        static_assert(third.Error() == NError::Error{
            .BasicInfo = {
                .LineNumber = 4,
                .Position = 12,
                .Code = NError::ErrorCode::SyntaxError
            },
            .AdditionalInfo = "brackets mismatch: encountered an excess '}'",
        });
    }
}
