#include <iostream>


using Test = auto () -> void;
Test TestArrayAPI;
Test TestArrayErrorHandling;
Test TestBasicErrorHandling;
Test TestBasicValueParsing;
Test TestComplexStructure;


auto main() -> int {
    TestArrayAPI();
    TestArrayErrorHandling();
    TestBasicErrorHandling();
    TestBasicValueParsing();
    TestComplexStructure();
    std::cout << "All tests passed!\n";
}
