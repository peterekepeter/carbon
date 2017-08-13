#pragma once
#include "InstructionWriter.h"
#include "InstructionReader.h"

namespace Carbon
{
	// pipes instructions from reader to writer, reader is consumed in the process
	// always use this instead of your own method, check implementation to see why
	void PipeInstructions(InstructionReader& reader, InstructionWriter& writer);
}
