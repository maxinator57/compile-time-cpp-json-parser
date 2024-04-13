#include <iostream>


using Test = auto () -> void;
Test TestArrayAPI;
Test TestArrayErrorHandling;
Test TestBasicErrorHandling;
Test TestBasicValueParsing;
Test TestComplexStructure;
Test TestMappingAPI;


auto main() -> int {
    TestArrayAPI();
    TestArrayErrorHandling();
    TestBasicErrorHandling();
    TestBasicValueParsing();
    TestComplexStructure();
    TestMappingAPI();
    std::cout << "All tests passed!\n";
}
