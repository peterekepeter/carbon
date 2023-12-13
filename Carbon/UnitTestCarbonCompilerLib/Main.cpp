#include "stdafx.h"
#include "ErrorMessageFormatter.h"
#include "LexerUnitTest.cpp"
#include "ParserUnitTest.cpp"
#include "ExecutionTest.cpp"
#include <cstdio>

int main() {
    RUN_TEST_CLASS(UnitTestCarbonCompilerLib::LexerUnitTest);
    RUN_TEST_CLASS(UnitTestCarbonCompilerLib::ParserUnitTest);
    RUN_TEST_CLASS(UnitTestCarbonCompilerLib::ExecutionTest);
    return RUN_TEST_SUMMARY();
}