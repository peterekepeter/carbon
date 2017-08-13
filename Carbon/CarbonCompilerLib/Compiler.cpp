#include "Compiler.h"
#include "Lexer.h"
#include "Parser.h"

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

void Carbon::CompileStream(std::istringstream stream, InstructionWriter& Output)
{
	// create instances
	Lexer lexer(stream);
	Parser parser(lexer);
	PipeInstructions(parser, Output);
}
