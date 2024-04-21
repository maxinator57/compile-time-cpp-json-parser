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