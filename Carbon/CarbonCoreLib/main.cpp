#include "exec.hpp"
#include <cstdio> 
#include <string>
#include "Compiler.hpp"

std::string prepareString(const char* str) {
	std::string builder = "\"";
	char* p = (char*) str;
	while (*p) {
		if (*p == '\\') builder += "\\\\";
		else builder += *p;
		p++;
	}
	builder += '"';
	return builder;
}

void submitArguments(InstructionWriter &output, int argc, char** argv) {
	output.WriteInstruction(InstructionType::ID, "arguments");
	output.WriteInstruction(InstructionType::ID, "array");
	output.WriteInstruction(InstructionType::CALLBEGIN);
	char buffer[256];
	std::sprintf(buffer, "%d", argc);
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
		std::sprintf(buffer, "%d", i);
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


class ConsoleInstructionWriter : public InstructionWriter
{
private:
	const char* name;
public:
	ConsoleInstructionWriter(const char* name)
	{
		this->name = name;
	}
	void WriteInstruction(InstructionType instruction) override
	{
		printf("%s:%x\n", name, instruction);
	}
	void WriteInstruction(InstructionType instruction, const char* stringData) override
	{
		printf("%s:%x %s\n", name, instruction, stringData);
	}
};

int main(int argc, char** argv, char** env) {

	exec::Executor executor;
	submitArguments(executor, argc, argv);
	submitEnv(executor, env);

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			Compiler::CompileFile(argv[i], executor);
			executor.Execute();
		}
	} else
		while (true) {
			executor.SetInteractiveMode(true);
			executor.ClearStatement();
			Compiler::CompileStdin(executor);
		}
}
