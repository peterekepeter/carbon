#include "Compiler.h"
#include "Lexer.h"
#include "Parser.h"
#include <iostream>
#include <fstream>
#include "../CarbonCommonLib/Instruction.h"

void Carbon::CompileStream(std::istream& stream, InstructionWriter& Output)
{
	// easy peasy
	Lexer lexer(stream);
	Parser parser(lexer);
	PipeInstructions(parser, Output);
}

void Carbon::CompileStdin(InstructionWriter& Output)
{
	CompileStream(std::cin, Output);
}

void Carbon::CompileFile(const char* Path, InstructionWriter& Output)
{
	std::ifstream file(Path);
	CompileStream(file, Output);
}

void Carbon::CompileFile(const std::string& Path, InstructionWriter& Output)
{
	std::ifstream file(Path);
	CompileStream(file, Output);
}

void Carbon::CompileString(const char* SourceCode, InstructionWriter& Output)
{
	std::stringstream stream(SourceCode);
	CompileStream(stream, Output);
}

void Carbon::CompileString(const std::string& SourceCode, InstructionWriter& Output)
{
	std::stringstream stream(SourceCode);
	CompileStream(stream, Output);
}
