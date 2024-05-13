## A fast compile- and run-time json parser written in C++20


### Description

This is a fast and efficient json parser written in C++20 that
- works both at compile- and run-time
- uses only STL, doesn't have any external dependencies
- is header-only: it's enough to `#include parser.hpp` to use the parser
- doesn't own any data, operating on immutable views to the memory where the text describing the json struct is located
- doesn't allocate any memory on the heap
- provides access to json fields via lightweight types that are immutable and thus are thread-safe and have value semantics
- provides a minimalistic and elegant API
- has efficient monadic error-handling which is straightforward and gives a lot of useful information, including the line number and position of the error and is thread- and memory-safe (see **Error handling** section)


### Usage

The parser implementation is header-only, so, to use it, it's enough to `#include parser.hpp` in your code. All the code is located in the namespace `NJsonParser`.

Below are some code snippets that show how to use the parser for basic tasks:

- At compile-time (see `examples/example_compile_time.cpp`)
  ```cpp
    #include "../parser.hpp"
    using namespace NJsonParser;

    // Create a json object at compile-time from anything that is convertible to `std::string_view`:
    constexpr auto json = JsonValue{
    /* line number */
    /*      0      */  "{                                           \n"
    /*      1      */  "    \"aba\": 1,                             \n"
    /*      2      */  "    \"caba\": [1, 2, \"fizz\", 4, \"buzz\"] \n"
    /*      3      */  "}                                           \n"
    };

    // Try to read a value of type `Int` from json mapping by key:
    constexpr auto aba = json["aba"].As<Int>();
    // The type of `aba` is not `Int`, but rather `Expected<Int>`,
    // because the attempt to read the value as an `Int` could be unsuccessful:
    static_assert(std::same_as<decltype(aba), const Expected<Int>>);
    // Check that the read was successful by calling the `.HasValue()` method
    // on an instance of `Expected`:
    static_assert(aba.HasValue());
    // Another way to do the same check is to call the `.HasError()` method
    // and negate the result:
    static_assert(!aba.HasError());
    // Get the actual value by calling the `.Value()` method
    // on an instance of `Expected`:
    static_assert(aba.Value() == 1);
    // Equality comparison can be done without the need to exctract
    // the value from an instance of `Expected`:
    static_assert(aba == 1);

    // Try to read an array from json mapping by key:
    constexpr auto caba = json["caba"].As<Array>();
    // Similarly to the first example, the type of `caba` is `Expected<Array>`
    static_assert(std::same_as<decltype(caba), const Expected<Array>>);
    static_assert(caba.HasValue());
    // `Expected<Array>` has the `.size()` method, which returns an instance of `Expected<size_t>`:
    static_assert(std::same_as<decltype(caba.size()), Expected<size_t>>);
    static_assert(caba.size() == 5);

    // The same can be done with strings:
    // `json["caba"][2]` is equalivalent to `json.As<Mapping>()["caba"].AsArray()[2]`
    constexpr auto fizz = json["caba"][2].As<String>();
    static_assert(std::same_as<decltype(fizz), const Expected<String>>);
    static_assert(fizz.HasValue());
    static_assert(fizz.Value() == "fizz");
    static_assert(fizz == "fizz");

    // Errors are represented by the struct `Error`:
    constexpr auto fizzError = json["caba"][2].As<Int>();
    static_assert(fizzError.HasError());
    static_assert(fizzError.Error() == NError::Error{
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
    constexpr auto fizzbuzz = json["caba"][14].As<String>();
    static_assert(fizzbuzz.HasError());
    static_assert(fizzbuzz.Error() == NError::Error{
        .BasicInfo = {
            .LineNumber = 2,
            .Position = 12, // points at the start of the array (an opening square bracket '[')
            .Code = NError::ErrorCode::ArrayIndexOutOfRange,
        },
        // Additional info here has a different form
        .AdditionalInfo = NError::ArrayIndexOutOfRangeAdditionalInfo{
            .Index = 14,
            .ArrayLen = 5,
        },
    });

    // Maps and arrays provide their own iterators, which are guaranteed to be at least `forward`:
    constexpr auto first = json.As<Mapping>().begin();
    static_assert(std::same_as<decltype(first), const Mapping::Iterator>);
    static_assert(std::forward_iterator<Array::Iterator>);
    static_assert(std::forward_iterator<Mapping::Iterator>);

    // Both iterators provide the dereference operator `*`:
    static_assert(std::same_as<decltype(*first), Mapping::Iterator::value_type>);
    static_assert(std::same_as<decltype((*first).Key), Expected<String>>);
    static_assert((*first).Key == "aba");
    static_assert(std::same_as<decltype((*first).Value), Expected<JsonValue>>);
    static_assert((*first).Value.As<Int>() == 1);

    constexpr auto second = std::next(first);
    static_assert((*second).Key == "caba");
    static_assert((*second).Value.As<Array>().size() == 5);

    // It is possible to iterate over the elements of a json array or mapping.
    // All types used in this json parser are lightweight and have value semantics,
    // therefore, it's better to pass them by value everywhere.
    for (const auto [k, v] : json.As<Mapping>()) {
        static_assert(std::same_as<decltype(k), const Expected<String>>);
        static_assert(std::same_as<decltype(v), const Expected<JsonValue>>);
    }
  ```

- At run-time (see `examples/example_run_time.cpp`)
  ```cpp
    #include "../parser.hpp"
    using namespace NJsonParser;

    // Create a json object at run-time from anything that is convertible to `std::string_view`:
    const auto someString = std::string{
    /* line number */
    /*      0      */  "{                                           \n"
    /*      1      */  "    \"aba\": 1,                             \n"
    /*      2      */  "    \"caba\": [1, 2, \"fizz\", 4, \"buzz\"] \n"
    /*      3      */  "}                                           \n"
    };
    const auto json = JsonValue{someString};

    // Try to read a value of type `Int` from json mapping by key:
    const auto aba = json["aba"].As<Int>();
    assert(aba.HasValue());
    // Another way to do the same check is to call the `.HasError()` method
    // and negate the result:
    assert(!aba.HasError());
    // Get the actual value by calling the `.Value()` method
    // on an instance of `Expected`:
    assert(aba.Value() == 1);
    // Equality comparison can be done without the need to exctract
    // the value from an instance of `Expected`:
    assert(aba == 1);

    // Try to read an array from json mapping by key:
    const auto caba = json["caba"].As<Array>();
    assert(caba.HasValue());
    // `Expected<Array>` has the `.size()` method, which returns an instance of `Expected<size_t>`:
    assert(caba.size() == 5);

    // The same can be done with strings:
    const auto fizz = json["caba"][2].As<String>();
    assert(fizz.HasValue());
    assert(fizz.Value() == "fizz");
    assert(fizz == "fizz");

    // Errors are represented by the struct `Error`:
    const auto fizzError = json["caba"][2].As<Int>();
    assert(fizzError.HasError());
    assert((fizzError.Error() == NError::Error{
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
    const auto fizzbuzz = json["caba"][14].As<String>();
    assert(fizzbuzz.HasError());
    assert((fizzbuzz.Error() == NError::Error{
        .BasicInfo = {
            .LineNumber = 2,
            .Position = 12, // points at the start of the array (an opening square bracket '[')
            .Code = NError::ErrorCode::ArrayIndexOutOfRange,
        },
        // Additional info here has a different form
        .AdditionalInfo = NError::ArrayIndexOutOfRangeAdditionalInfo{
            .Index = 14,
            .ArrayLen = 5,
        },
    }));

    // Maps and arrays provide their own iterators, which are guaranteed to be at least `forward`:
    const auto first = json.As<Mapping>().begin();
    const auto [k, v] = *first;
    assert(k.Value() == "aba");
    assert(v.As<Int>() == 1);

    const auto second = std::next(first);
    assert((*second).Key == "caba");
    assert((*second).Value.As<Array>().size() == 5);

    // It is possible to iterate over the elements of a json array or mapping.
    // All types used in this json parser are lightweight and have value semantics,
    // therefore, it's better to pass them by value everywhere.
    auto keys = std::vector<String>{};
    for (const auto [k, v] : json.As<Mapping>()) {
        keys.push_back(k.Value());
    }
    assert((keys == std::vector<std::string_view>{"aba", "caba"}));

    auto numbers = std::vector<Int>{};
    auto strings = std::vector<String>{};
    for (const auto elem : json["caba"].As<Array>()) {
        if (auto i = elem.As<Int>(); i.HasValue()) {
            numbers.push_back(i.Value());
        } else {
            strings.push_back(elem.As<String>().Value());
        }
    }
    assert((numbers == std::vector<Int>{1, 2, 4}));
    assert((strings == std::vector<String>{"fizz", "buzz"}));
  ```


### Tests

The tests are located in the `tests` directory. Each test is written in its own `.cpp` file with an appropriate name. The source code of the tests includes comments that help to understand what is going on.

To build the tests with `gcc`, just run the following command in terminal:
```console
g++ --std=c++20 tests/* -o run_tests
```
To build the tests with `clang`:
```console
clang++ --std=c++20 tests/* -o run_tests
```
To run the tests after building:
```console
./run_tests
```

### Code structure

The header file `parser.hpp` simply includes all the needed implementation files located in the `impl` directory. The logic is split between these header files in the following way:

| Header file | Contents |
| :---------- | :------- |
| `impl/api.hpp`   | Declarations of all classes that represent json data (`Int`, `Double`, `String`, `Array`, `Mapping` and `JsonValue`) and their methods |
| `impl/array.hpp` | Implementation of the `Array` and `Expected<Array>` class methods and definition of the `Array::Iterator` class |
| `impl/data_holder.hpp` | Definition of the `DataHolderMixin` class |
| `impl/error.hpp` | Definitions of all classes and functions related to error handling |
| `impl/expected.hpp` | Definitions of all the `Expected<T>` classes and the `ExpectedMixin<T>` class |
| `impl/iterator.hpp` | Definition of the `GenericSerializedSequenceIterator` class |
| `impl/json_value.hpp` | Implementation of the `JsonValue` class methods |
| `impl/line_position_counter.hpp` | Definition of the `LinePositionCounter` class |
| `impl/mapping.hpp` | Implementation of the `Mapping` and `Expected<Mapping>` class methods and definition of the `Mapping::Iterator` class |
| `impl/utils.hpp` | Definitions of some utility functions needed to iterate over string symbols in specific ways |

The tests are located in the `tests` directory, and the examples from this documentation -- in the `examples` directory.


### Error handling

All json access operations return instances of `Expected<T>` classes instead of instances of `T`. The class template `Expected<T>` is essentially a `std::variant<T, NError::Error>` with some additional methods. All specializations of the `Expected<T>` class template provide the following methods:

| Method | Return type | Description |
| :----- | :---------- | :---------- |
| `HasValue()` | `bool` | Returns `true` if the `std::variant` contains a value, `false` if it contains an error. |
| `HasError()` | `bool` | Same effect as `!HasValue()`. |
| `Value()` | `const T&` | Returns a const reference to the value of type `T` contained in the `std::variant`. If the `std::variant` contains an error, throws `std::bad_variant_access`. |
| `Error()` | `const NError::Error&` | Returns a const reference to the value of type `NError::Error` contained in the `std::variant`. If the `std::variant` contains a value instead of an error, throws `std::bad_variant_access`. |
| `operator==(const Expected<T>&)` if `T` supports `operator==` | `bool` | Equality comparison |
| `template <class U> operator==(const U&)` if `T` supports `operator==(const U&)` | `bool` | Equality comparison |
| `operator==(const NError::Error&)` | `bool` | Equality comparison |

The comparison operators are needed to facilitate error handling and setting up the control flow by allowing to explicitly compare instances of `Expected<T>` to `T` and `NError::Error` objects.

Specializations of `Expected<T>` for when `T` is one of the json containers (`Array`, `Mapping`, `JsonValue`) provide specific methods that mirror the methods specific to the corresponding container. The following three tables list the methods specific to the `Expected` class template specializations `Expected<Array>`, `Expected<Mapping>` and `Expected<JsonValue>` respectively.

Methods specific to `Expected<Array>`:

| Method | Return type | Description |
| :----- | :---------- | :---------- |
| `size()` | `Expected<size_t>` | Returns an instance of `Expected<size_t>` containing the size of the underlying array if this instance of `Expected<Array>` contains an array; otherwise, returns an `Expected<size_t>` holding the same error as this instance of `Expected<Array>`. |
| `begin()` | `Array::Iterator` | If this instance of `Expected<Array>` contains an array, returns an iterator pointing at the start of this array. Otherwise, returns the same iterator as the iterator returned by the invocation of `end()` on this instance of `Expected<Array>`. |
| `end()` | `Array::Iterator` | If this instance of `Expected<Array>` contains an array, returns an iterator containing a sentinel for the element "after the end of the array". Otherwise, returns the same iterator as the iterator returned by the invocation of `begin()` on this instance of `Expected<Array>`. |

The `begin()` and `end()` methods on `Expected<Array>` allow to write code like
```cpp
const Expected<Array> maybeArr = ...;
for (const auto elem : maybeArr) { ... }
```
If `maybeArr` contains an error, the `for` loop would simply do zero iterations.
