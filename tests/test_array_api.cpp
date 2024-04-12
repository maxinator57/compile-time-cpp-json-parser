#include "../parser.hpp"

#include <cassert>
#include <numeric>
#include <ranges>
#include <vector>


using namespace NCompileTimeJsonParser;


auto TestArrayAPI() -> void {
    constexpr auto json = TJsonValue{
        "[1, 2, \"fizz\", 4, \"buzz\", \"fizz\", 7, 8, \"fizz\", \"buzz\", 11, \"fizz\", 13, 14, \"fizzbuzz\"]"
    };
    // First, explicitly convert json to an array. Arrays don't have to be homogenous in their element type:
    constexpr auto arr = json.AsArray();
    static_assert(!arr.HasError());

    // Iterate over json arrays in usual ways:
    {   // Provides `.begin()` and `.end()` iterators of type `TJsonArray::Iterator`,
        // which is guranteed to be at least a `forward_iterator`:
        static_assert(std::forward_iterator<TJsonArray::Iterator>);

        // For example, let's add up all elements of the array that are integers
        // by manually iterating over them:
        int sum = 0; 
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            auto elem = (*it).AsInt(); // the type of `elem` is `TExpected<Int>`
            if (elem.HasValue()) {    // check that the current element is an actual integer
                sum += elem.Value();  // then safely take its value and add it to the sum
            }
        }
        assert(sum == 60);
    }
    {   // Do the same thing using a regular range-based `for` loop:
        int sum = 0;
        for (auto&& elem : arr) if (auto x = elem.AsInt(); x.HasValue()) sum += x.Value();
        assert(sum == 60);
    }
    {   // Do the same thing using c++20 ranges (for some reason doesn't compile on clang):
    #if !defined (__clang__) && defined (__GNUG__)
        auto processedArr = arr
            | std::views::transform([](auto&& elem) { return elem.AsInt(); })
            | std::views::filter([](auto&& maybeInt) { return maybeInt.HasValue(); })
            | std::views::transform([](auto&& definitelyInt) { return definitelyInt.Value(); });
        const auto sum = std::accumulate(processedArr.begin(), processedArr.end(), 0);
        assert(sum == 60);
    #endif
    }
    {   // Do the same thing at compile time using `std::accumulate`
        // with custom addition operation:
        constexpr auto sum = std::accumulate(
            arr.begin(), arr.end(), 0,
            [](int curSum, auto&& elem) {
                return curSum + (elem.AsInt().HasValue() ? elem.AsInt().Value() : 0);
            }
        );
        static_assert(sum == 60);
    }
 
    {   // Get an element by index:
        static_assert(arr[2].AsString() == "fizz");
        // Same thing without explicitly casting json to array:
        static_assert(json[2].AsString() == "fizz"); // remember that `arr == json.AsArray()`
    }

    {   // Get the number of elements in an array (works in O({length of underlying string data})):
        auto vec = std::vector<int>{}; vec.reserve(arr.size().Value()); // have to call `.Value()` after calling `.size()`,
                                                                        // because the type of `arr` is not `TJsonArray`,
                                                                        // but `TExpected<TJsonArray>`
        for (auto&& elem : arr) {
            const auto maybeInt = elem.AsInt();
            if (maybeInt.HasValue()) vec.push_back(maybeInt.Value());
        }
        assert((vec == std::vector{1, 2, 4, 7, 8, 11, 13, 14}));

        // Can be useful when dealing with containers the size of which has to be known at compile time:
        #if !defined (__clang__) && defined (__GNUG__) // doesn't compile on clang
        struct {
            // size of a `std::array` has to be a compile-time constant
            std::array<int, arr.size().Value()> Data = {}; // initialized with zeros
            size_t Counter = 0;
            auto push(int x) -> bool {
                if (Counter < Data.size()) {
                    Data[Counter] = x;
                    ++Counter;
                    return true;
                }
                return false;
            }
        } buf;

        for (auto&& elem : arr) {
            if (elem.AsInt().HasValue()) assert(buf.push(elem.AsInt().Value()));
        }
        int bufSum = std::accumulate(buf.Data.begin(), buf.Data.end(), 0);
        assert(bufSum == 60);
        #endif
    }

    {   // This works, but is NOT efficient, as it runs in
        // O({length of underlying string data} * {number of elements in the array})
        int sum = 0;
        for (size_t i = 0; i != arr.size().Value(); ++i) {
            if (arr[i].AsInt().HasValue()) sum += arr[i].AsInt().Value();
        }
        assert(sum == 60);
    }
}
