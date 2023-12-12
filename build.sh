files=(
    ./Carbon/Console/Main.cpp
    ./Carbon/CarbonCoreLib/Executor.cpp
    ./Carbon/CarbonCoreLib/AstNodes.cpp
    ./Carbon/CarbonCoreLib/Threading.cpp
    # ./Carbon/UnitTestCarbonCompilerLib/ErrorMessageFormatter.cpp
    # ./Carbon/UnitTestCarbonCompilerLib/ExecutionTest.cpp
    # ./Carbon/UnitTestCarbonCompilerLib/stdafx.cpp
    # ./Carbon/UnitTestCarbonCompilerLib/LexerUnitTest.cpp
    # ./Carbon/UnitTestCarbonCompilerLib/ParserUnitTest.cpp
    ./Carbon/CarbonCompilerLib/Lexer.cpp
    ./Carbon/CarbonCompilerLib/Parser.cpp
    ./Carbon/CarbonCompilerLib/Compiler.cpp
    ./Carbon/BinseqLib/binseq.cpp
    ./Carbon/BinseqLib/bit_sequence.cpp
    ./Carbon/BinseqLib/popcount.cpp
    ./Carbon/CarbonCommonLib/Instruction.cpp
)


zig c++ ${files[@]} -o out/carbon -O3
