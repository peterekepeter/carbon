
#include "Instruction.h"

void Carbon::PipeInstructions(InstructionReader& reader, InstructionWriter& writer)
{
	// consume reader
	while (reader.MoveNext())
	{
		// read isntructiont ype
		auto instruction = reader.ReadInstructionType();
		if (InstructionNeedsString(instruction))
		{
			// instruction comes with string data
			writer.WriteInstruction(instruction, reader.ReadStringData());
		}
		else
		{
			// instruction has no string data
			writer.WriteInstruction(instruction);
		}
	}
}
