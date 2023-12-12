
#include <string>
#include "../CarbonCommonLib/InstructionWriter.h"
#include "../CarbonCoreLib/Executor.h"
#include "../CarbonCompilerLib/Compiler.h"
#include "../CarbonCompilerLib/Parser.h"
#include <iostream>
#include <fstream>

#ifndef sprintf_s
	#define sprintf_s sprintf
#endif

std::string prepareString(const char* str) {
	std::string builder = "\"";
	auto p = str;
	while (*p) {
		if (*p == '\\') builder += "\\\\";
		else builder += *p;
		p++;
	}
	builder += '"';
	return builder;
}

using namespace Carbon;

void submitArguments(InstructionWriter &output, int argc, char** argv) {
	output.WriteInstruction(InstructionType::ID, "arguments");
	output.WriteInstruction(InstructionType::ID, "array");
	output.WriteInstruction(InstructionType::CALLBEGIN);
	char buffer[256];
	sprintf_s(buffer, "%d", argc);
	output.WriteInstruction(InstructionType::NUM, buffer);
	output.WriteInstruction(InstructionType::CALLEND);
	output.WriteInstruction(InstructionType::ASSIGN);
	output.WriteInstruction(InstructionType::END_STATEMENT);

	for (int i = 0; i < argc; i++) {
		output.WriteInstruction(InstructionType::ID, "set");
		output.WriteInstruction(InstructionType::CALLBEGIN);
		//1
		output.WriteInstruction(InstructionType::ID, "arguments");
		//2
		char buffer[256];
		sprintf_s(buffer, "%d", i);
		output.WriteInstruction(InstructionType::NUM, buffer);
		//3

		output.WriteInstruction(InstructionType::STR, prepareString(argv[i]).c_str());

		output.WriteInstruction(InstructionType::CALLEND);
		output.WriteInstruction(InstructionType::END_STATEMENT);
	}
}

void submitEnv(InstructionWriter &output, char** env) {
	output.WriteInstruction(InstructionType::ID, "environment");
	output.WriteInstruction(InstructionType::ID, "object");
	output.WriteInstruction(InstructionType::CALLBEGIN);
	output.WriteInstruction(InstructionType::CALLEND);
	output.WriteInstruction(InstructionType::ASSIGN);
	output.WriteInstruction(InstructionType::END_STATEMENT);

	for (int i = 0; env[i] != nullptr; i++) {
		output.WriteInstruction(InstructionType::ID, "set");
		output.WriteInstruction(InstructionType::CALLBEGIN);
		output.WriteInstruction(InstructionType::ID, "environment");

		std::string e = env[i];
		auto pos = e.find('=');

		output.WriteInstruction(InstructionType::STR, prepareString(e.substr(0, pos).c_str()).c_str());
		output.WriteInstruction(InstructionType::STR, prepareString(e.substr(pos + 1, std::string::npos).c_str()).c_str());
		output.WriteInstruction(InstructionType::CALLEND);
		output.WriteInstruction(InstructionType::END_STATEMENT);
	}
}


int main(int argc, char** argv, char** env) {

	Executor executor;
	submitArguments(executor, argc, argv);
	submitEnv(executor, env);

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			const char* filename = argv[i];
			std::ifstream file(filename);
			// check file
			if (file.is_open() == false)
			{
				std::cerr << "Failed to open file: " << filename << '\n';
				break;
			}
			// compile and run
			try
			{
				CompileStream(file, executor);
				executor.Execute();
			}
			catch (Carbon::ParserException exception)
			{
				std::cerr << "Parser Error: " << exception.GetMessageWithLineAndPosition() << "\n";
			}
			catch (Carbon::ExecutorExeption exception)
			{
				std::cerr << "Runtime Error: " << exception.GetMessage() << "\n";
			}
		}
	}
	else
		while (true) {
			executor.SetInteractiveMode(true);
			executor.ClearStatement();
			bool recoverFromError = false;
			try
			{
				CompileStdin(executor);
			}
			catch(Carbon::ParserException exception)
			{
				std::cerr << "Parser Error: " << exception.GetMessageWithLineAndPosition() << "\n";
				recoverFromError = true;
			}
			catch (Carbon::ExecutorExeption exception)
			{
				std::cerr << "Runtime Error: " << exception.GetMessage() << "\n";
				recoverFromError = true;
			}
			// error recovery
			if (recoverFromError)
			{
				// clear cin until end of line
				std::cin.ignore(10000, '\n');
				// clear statement in executor
				executor.ClearStatement();
			}
		}
}
