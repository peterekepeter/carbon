#pragma once

#include "../CarbonCommonLib/InstructionWriter.h"
#include <sstream>
#include "InstructionReader.h"

namespace Carbon
{
	// pipes instructions from reader to writer, reader is consumed in the process
	// always use this instead of your own method, check implementation to see why
	void PipeInstructions(InstructionReader& reader, InstructionWriter& writer);

	// compiles a string stream
	void CompileStream(std::istringstream, InstructionWriter &Output);
	void CompileStdin(InstructionWriter &Output);
	void CompileFile(const char* Path, InstructionWriter &Output);
	void CompileString(const char* SourceCode, InstructionWriter &Output);
	void CompileFile(const std::string& Path, InstructionWriter &Output);
	void CompileString(const std::string& SourceCode, InstructionWriter &Output);
}
