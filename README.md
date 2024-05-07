## A fast compile- and run-time json parser written in C++20

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
- At compile-time
  ```cpp
  using namespace NJsonParser;

  constexpr auto json = TJsonValue{
  /* line number */
  /*      0      */  "{                                        \n"
  /*      1      */  "    \"aba\": 1,                          \n"
  /*      2      */  "    \"caba\": [2, \"fizz\", 4, \"buzz\"] \n"
  /*      3      */  "}                                        \n"
  };

  constexpr auto aba = json["aba"].AsInt();
  static_assert(aba.HasValue());
  static_assert(aba.Value() == 1);
  static_assert(aba == 1);

  constexpr auto two = json["caba"][0].AsInt();
  static_assert(two == 2);

  constexpr auto fizz = json["caba"][3].AsString();
  static_assert(fizz.HasValue());
  static_assert(fizz.Value() == "fizz");
  static_assert(fizz == "fizz");

  constexpr auto fizzbuzz = json["caba"][13].AsString();
  static_assert(fizzbuzz.HasError());
  static_assert(fizzbuzz.Error() == NError::TError{
      .BasicInfo = {
          .LineNumber = 2,
          .Position = 13, // points at the start of the array (an opening square bracket '[')
          .Code = NError::ErrorCode::ArrayIndexOutOfRange,
      },
      .AdditionalInfo = NError::TArrayIndexOutOfRangeAdditionalInfo{
          .Index = 13,
          .ArrayLen = 4,
      },
  });
  ```