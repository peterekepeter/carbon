#pragma once

#include "InstructionType.hpp"

class InstructionReader
{
public:
	virtual bool MoveNext() const = 0;
	virtual InstructionType ReadInstructionType() const = 0;
	virtual const char* ReadStringData() const = 0;
	inline virtual ~InstructionReader() {};
};