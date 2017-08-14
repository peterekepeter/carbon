#pragma once

#include "InstructionType.h"

namespace Carbon
{
	// represents a generic type to which instructions can be written to
	class InstructionWriter
	{
	public:
		// write instruction
		virtual void WriteInstruction(InstructionType instruction) = 0;
		// write instruction and data, for instruction types that need data
		virtual void WriteInstruction(InstructionType instruction, const char* stringData) = 0;
		inline virtual ~InstructionWriter() {};
	};

}
