#pragma once


class IBytecodeContainer {
public:
	virtual void SubmitInstruction(unsigned char instruction, const char* data) = 0;
	virtual ~IBytecodeContainer() = 0;
};