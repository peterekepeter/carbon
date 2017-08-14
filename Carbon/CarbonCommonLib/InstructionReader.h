#pragma once

#include "InstructionType.h"

namespace Carbon
{
	// represents a generic type from which instructions can be read
	class InstructionReader
	{
	public:
		// move to next instruction, returns false if no more instructions
		virtual bool MoveNext() = 0;
		// read instruction type, should check if it needs string data with Carbon::InstructionNeedsString
		virtual Carbon::InstructionType ReadInstructionType() = 0;
		// read instruction data
		virtual const char* ReadStringData() = 0;
		inline virtual ~InstructionReader() {};
	};
}
