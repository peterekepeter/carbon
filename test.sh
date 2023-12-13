files=(
    ./Carbon/UnitTestCarbonCompilerLib/Main.cpp
    ./Carbon/UnitTestCarbonCompilerLib/stdafx.cpp
    ./Carbon/UnitTestCarbonCompilerLib/ErrorMessageFormatter.cpp
    ./Carbon/CarbonCoreLib/Executor.cpp
    ./Carbon/CarbonCoreLib/AstNodes.cpp
    ./Carbon/CarbonCoreLib/Threading.cpp
    ./Carbon/CarbonCompilerLib/Lexer.cpp
    ./Carbon/CarbonCompilerLib/Parser.cpp
    ./Carbon/CarbonCompilerLib/Compiler.cpp
    ./Carbon/BinseqLib/binseq.cpp
    ./Carbon/BinseqLib/bit_sequence.cpp
    ./Carbon/BinseqLib/popcount.cpp
    ./Carbon/CarbonCommonLib/Instruction.cpp
)

zig c++ ${files[@]} -o out/unittest -DUSE_MAIN && ./out/unittest
