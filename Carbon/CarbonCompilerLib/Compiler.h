#pragma once

#include <istream>
#include "../CarbonCommonLib/InstructionWriter.h"

namespace Carbon
{
	// compiles a string stream
	void CompileStream(std::istream& InputStream, InstructionWriter &Output);

	// helper function, calls CompileStream with std::cin
	void CompileStdin(InstructionWriter &Output);

	// helper function, calls CompileStream with a new file
	void CompileFile(const char* InputFilePath, InstructionWriter &Output);

	// helper function, calls CompileStream with a new file
	void CompileFile(const std::string& Path, InstructionWriter &Output);

	// helper function, calls CompileStream with a string stream
	void CompileString(const char* SourceCode, InstructionWriter &Output);

	// helper function, calls CompileStream with a string stream
	void CompileString(const std::string& SourceCode, InstructionWriter &Output);
}
