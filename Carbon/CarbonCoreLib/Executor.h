#pragma once

#include "../CarbonCommonLib/InstructionWriter.h"
#include "AstNodes.h"

namespace Carbon {

	class ExecutorImp;

	class Executor : public InstructionWriter{
	public:
		Executor();
		virtual ~Executor();

		void WriteInstruction(InstructionType instruction) override;
		void WriteInstruction(InstructionType instruction, const char* stringData) override;

		// extend the interpreter with function written in C++
		void RegisterNativeFunction(const char* name, native_function_ptr, bool pure);

		//for better syntax error message
		const char* getLastAtomText(); 
		void SetInteractiveMode(bool interactive);
		bool GetInteractiveMode();

		// run the submitted commands
		std::shared_ptr<Node> Execute();

		//parse error recovery for console mode
		void ClearStatement();
	private:
		ExecutorImp* imp;
	};

}
