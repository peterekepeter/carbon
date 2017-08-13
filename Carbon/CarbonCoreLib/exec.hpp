#pragma once

#include "../CarbonCommonLib/InstructionWriter.h"

namespace Carbon {

	class ExecutorImp;

	class Executor : public InstructionWriter{
	public:
		Executor();
		virtual ~Executor();

		//void submitAtom(const InstructionType type, const char* text);
		//void submitCommand(const InstructionType cmd);
		void WriteInstruction(InstructionType instruction) override;
		void WriteInstruction(InstructionType instruction, const char* stringData) override;

		const char* getLastAtomText(); //for better syntax error message
		void SetInteractiveMode(bool interactive);
		bool GetInteractiveMode();
		void Execute();
		void ClearStatement(); //parse error recovery for console mode
	private:
		ExecutorImp* imp;
	};

}
