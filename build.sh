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

compiler='zig c++'
compile="$compiler ${files[@]} -O3"

if [ "$1" == "multitarget" ]; then
    # list of supported targets
    targets=(
        x86_64-linux
        # arm-linux
        # aarch64-linux
        # i386-linux
        x86_64-windows
        # arm-windows
        # aarch64-windows
    )
    # compile all targets
    for i in "${targets[@]}"
    do
        # add .exe extension on windows
        outfile="co2-$i"
        if [[ $i =~ "windows" ]]; then
            outfile="$outfile.exe"
        fi
        $compile -target $i -o out/$outfile
    done
else 
    # current os only
    $compile -o out/co2
fi