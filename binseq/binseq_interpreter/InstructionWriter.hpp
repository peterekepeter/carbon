#pragma once

#include "InstructionType.hpp"

class InstructionWriter
{
public:
	virtual void WriteInstruction(InstructionType instruction) = 0;
	virtual void WriteInstruction(InstructionType instruction, const char* stringData) = 0;
	inline virtual ~InstructionWriter() {};
};
