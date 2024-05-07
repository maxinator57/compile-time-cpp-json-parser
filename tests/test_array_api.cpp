#include "../parser.hpp"

#include <cassert>
#include <numeric>
#include <ranges>
#include <vector>


using namespace NJsonParser;


auto TestArrayAPI() -> void {
    static constexpr auto json = JsonValue{
        "[1, 2, \"fizz\", 4, \"buzz\", \"fizz\", 7, 8, \"fizz\", \"buzz\", 11, \"fizz\", 13, 14, [\"fizz\", \"buzz\"]]"
    };

    // First, explicitly convert json to an array. Arrays don't have to be homogenous in their element type
    static constexpr auto arr = json.As<Array>();
    static_assert(!arr.HasError());

    {   // Get an element by index
        static_assert(arr[2].As<String>() == "fizz");
        // Same thing without explicitly casting `json` to `Array`
        static_assert(json[2].As<String>() == "fizz"); // remember that `arr == json.As<Array>()`
        // Nested array elements can be accessed without explicitly calling `As<Array>()` to get intermediate arrays
        static_assert(json[14][0].As<String>() == "fizz");
        static_assert(json[14][1].As<String>() == "buzz");
        // Same as
        static_assert(json[14].As<Array>()[0].As<String>().Value() == "fizz");
        static_assert(json[14].As<Array>()[1].As<String>().Value() == "buzz");
    }

    {   // Iterate over json arrays in usual ways.
        // `Array` provides `.begin()` and `.end()` iterators of type `Array::Iterator`,
        // which is guaranteed to be at least a `forward_iterator`
        static_assert(std::forward_iterator<Array::Iterator>);
    }

    {   // For example, let's add up all elements of the array that are integers
        // by manually iterating over them
        int sum = 0; 
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            const auto elem = (*it).As<Int>(); // the type of `elem` is `Expected<Int>`
            if (elem.HasValue()) {    // check that the current element is an actual integer
                sum += elem.Value();  // then safely take its value and add it to the sum
            }
        }
        assert(sum == 60);
    }

    {   // Do the same thing using a regular range-based `for` loop
        int sum = 0;
        for (const auto elem : arr)
            if (const auto x = elem.As<Int>(); x.HasValue())
                sum += x.Value();
        assert(sum == 60);
    }

    {   // Do the same thing using c++20 ranges
        // (for some reason related to concept checking this doesn't compile on clang)
        #if !defined (__clang__) && defined (__GNUG__)
        auto processedArr = arr
            | std::views::transform([](Expected<JsonValue> elem) { return elem.As<Int>(); })
            | std::views::filter([](auto maybeInt) { return maybeInt.HasValue(); })
            | std::views::transform([](auto definitelyInt) { return definitelyInt.Value(); });
        const auto sum = std::accumulate(processedArr.begin(), processedArr.end(), 0);
        assert(sum == 60);
        #endif
    }

    {   // Do the same thing at compile time using `std::accumulate`
        // with custom addition operation
        constexpr auto sum = std::accumulate(
            arr.begin(), arr.end(), 0,
            [](int curSum, Expected<JsonValue> elem) {
                return curSum + (elem.As<Int>().HasValue() ? elem.As<Int>().Value() : 0);
            }
        );
        static_assert(sum == 60);
    } 

    {   // This works, but is NOT efficient, as it runs in
        // O({length of underlying string data} * {number of elements in the array})
        int sum = 0;
        for (size_t i = 0; i != arr.size().Value(); ++i) {
            if (arr[i].As<Int>().HasValue()) sum += arr[i].As<Int>().Value();
        }
        assert(sum == 60);
    }

    {   // Get the number of elements in an array (works in O({length of underlying string data}))
        auto ints = std::vector<int>{}; ints.reserve(arr.size().Value()); // have to call `.Value()` after calling `.size()`,
                                                                          // because the type of `arr` is not `Array`,
                                                                          // but `Expected<Array>`
        static_assert(arr.size().Value() == arr.Value().size());
        for (const auto elem : arr) {
            const auto maybeInt = elem.As<Int>();
            if (maybeInt.HasValue()) ints.push_back(maybeInt.Value());
        }
        assert((ints == std::vector{1, 2, 4, 7, 8, 11, 13, 14}));
    }

    {   // Do the same thing as in the previous example, but fully in constexpr context
        constexpr auto ints = []() {
            constexpr auto n = []() {
                int n = 0;
                for (const auto elem : arr) {
                    if (elem.As<Int>().HasValue()) ++n;
                }
                return n;
            }();
            const auto v = []() {
                auto v = std::vector<int>{};
                for (const auto elem : arr) {
                    if (elem.As<Int>().HasValue()) v.push_back(elem.As<Int>().Value());
                }
                return v;
            }();
            auto a = std::array<int, n>{};
            std::copy(v.begin(), v.end(), a.begin());
            return a;
        }();
        static_assert(ints == std::array{1, 2, 4, 7, 8, 11, 13, 14});
    } 
}
