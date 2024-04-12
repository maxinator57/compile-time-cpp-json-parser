#include <iostream>


using Test = auto () -> void;


Test TestArrayAPI;
Test TestArrayErrorHandling;
Test TestBasicValueParsing;
Test TestComplexStructure;


auto main() -> int {
    TestArrayAPI();
    TestArrayErrorHandling(); 
    TestBasicValueParsing();
    TestComplexStructure();

    std::cout << "All tests passed!\n";
}
