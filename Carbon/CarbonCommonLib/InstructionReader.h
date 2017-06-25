#pragma once

#include "InstructionType.h"

class InstructionReader
{
public:
	virtual bool MoveNext() = 0;
	virtual InstructionType ReadInstructionType() = 0;
	virtual const char* ReadStringData() = 0;
	inline virtual ~InstructionReader() {};
};