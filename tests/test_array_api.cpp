#include "../parser.hpp"

#include <cassert>
#include <numeric>
#include <queue>
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

    // Iterate over json array in usual ways:
    {   // Provides `.begin()` and `.end()` iterators of type `TJsonArray::Iterator`:
        static_assert(std::forward_iterator<TJsonArray::Iterator>);
        int sum = 0;
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            auto elem = (*it).AsInt(); // the type of elem is `TExpected<Int>`
            if (elem.HasValue()) sum += elem.Value(); // add up all elements that actually are integers
        }
        assert(sum == 60);
    }
    {   // Use json arrays with regular range-based `for` loop:
        int sum = 0;
        for (auto&& elem : arr) if (auto x = elem.AsInt(); x.HasValue()) sum += x.Value();
        assert(sum == 60);
    }
    {   // Use json arrays with c++20 ranges:
        auto processedArr = arr
            | std::ranges::views::transform([](auto&& elem) { return elem.AsInt(); })
            | std::ranges::views::filter([](auto&& maybeInt) { return maybeInt.HasValue(); })
            | std::ranges::views::transform([](auto&& definitelyInt) { return definitelyInt.Value(); });
        const auto sum = std::accumulate(processedArr.begin(), processedArr.end(), 0);
        assert(sum == 60);
    }
    {   // All array operations are `constexpr` hence can be used at compile time:
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
        // Same thing without explicitly casting json value to array:
        static_assert(json[2].AsString() == "fizz"); // remember that `arr == json.AsArray()`
    }

    {   // Get the length of an array (works in O(length of underlying string data)):
        auto vec = std::vector<int>{}; vec.reserve(arr.size().Value()); // have to call `.Value()` after calling `.size()`,
                                                                        // because the type of `arr` is not `TJsonArray`,
                                                                        // but `TExpected<TJsonArray>`
        for (auto&& elem : arr) {
            const auto maybeInt = elem.AsInt();
            if (maybeInt.HasValue()) vec.push_back(maybeInt.Value());
        }
        assert((vec == std::vector{1, 2, 4, 7, 8, 11, 13, 14}));

        // Can do the same thing with containers the size of which has to be known at compile time:
        struct {
            // size of `Data` has to be a compile-time constant
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
    }

    {   // This works, but is NOT efficient, as it works in
        // O({length of underlying string data} * {number of elements in the array})
        int sum = 0;
        for (size_t i = 0; i != arr.size().Value(); ++i) {
            if (arr[i].AsInt().HasValue()) sum += arr[i].AsInt().Value();
        }
        assert(sum == 60);
    }
}
