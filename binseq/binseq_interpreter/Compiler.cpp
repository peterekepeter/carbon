#include "InstructionWriter.hpp"
#include <cstdio>
#include <mutex>          // std::mutex

struct yy_buffer_state;
int yyparse();
extern FILE* yyin;
InstructionWriter* yyInstructionWriter;


extern yy_buffer_state;
typedef yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_buffer(char *, size_t);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern void yyrestart(FILE *input_file);

namespace Compiler
{

	static std::mutex ParserLock;

	static void Compile(FILE* fileptr, InstructionWriter* writer)
	{
		ParserLock.lock();
		{
			yyrestart(fileptr);
			yyInstructionWriter = writer;
			yyparse();
		}
		ParserLock.unlock();
	}

	void CompileFile(const char* FilePath, InstructionWriter &writer)
	{
		auto ptr = std::fopen(FilePath, "r");
		if (ptr == nullptr)
		{
			fprintf(stderr, "CompileFile: Failed to open file '%s'",FilePath);
			return;
		}
		Compile(ptr, &writer);
		fclose(ptr);
	}


	void CompileString(const char* SourceCode, InstructionWriter &writer)
	{
		auto size = strlen(SourceCode);
		char* sourceCopy = new char[size+2];
		sourceCopy[size + 0] = 0;
		sourceCopy[size + 1] = 0;
		strcpy_s(sourceCopy,size+1, SourceCode);
		ParserLock.lock();
		{
			auto buffer = yy_scan_buffer(sourceCopy, size + 2);
			yyInstructionWriter = &writer;
			yyparse();
			yy_delete_buffer(buffer);
		}
		ParserLock.unlock();
		delete[] sourceCopy;
	}


	void CompileStdin(InstructionWriter &Output)
	{
		Compile(stdin, &Output);
	}

}