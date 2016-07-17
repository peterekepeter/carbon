#pragma once

#include "InstructionWriter.hpp"

/// This namespace contains everything needed to parse stuff
namespace Compiler {

	void CompileStdin(InstructionWriter &Output);
	void CompileFile(const char* Path, InstructionWriter &Output);
	void CompileString(const char* SourceCode, InstructionWriter &Output);
}