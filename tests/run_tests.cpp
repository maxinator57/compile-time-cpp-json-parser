#include <iostream>


using Test = auto () -> void;


Test TestBasicValueParsing;
Test TestArrayAPI;
Test TestArrayErrorHandling;


auto main() -> int {
    TestBasicValueParsing();
    TestArrayAPI();
    TestArrayErrorHandling();

    std::cout << "All tests passed!\n";
}
