#include <array>
#include <iostream>


using Test = auto () -> void;
Test TestArrayAPI;
Test TestArrayErrorHandling;
Test TestBasicErrorHandling;
Test TestBasicValueParsing;
Test TestComplexStructure;
Test TestMappingAPI;
Test TestMappingErrorHandling;
Test TestWeirdStringLiterals;


#define RUN_TEST(testName) testName(); std::cout << #testName << " passed!\n"


auto main() -> int {
    RUN_TEST(TestArrayAPI);
    RUN_TEST(TestArrayErrorHandling);
    RUN_TEST(TestBasicErrorHandling);
    RUN_TEST(TestBasicValueParsing);
    RUN_TEST(TestComplexStructure);
    RUN_TEST(TestMappingAPI);
    RUN_TEST(TestMappingErrorHandling);
    RUN_TEST(TestWeirdStringLiterals);
    std::cout << "All tests passed!\n";
}
