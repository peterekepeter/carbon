#include "Executor.h"

#include <cstdio>
#include <string>
#include <stack>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>

#include "../BinseqLib/binseq.hpp"
#include "AstNodes.h"
#include "ExecutorException.h"
#include <chrono>
#include <sstream>
#include "Threading.h"

#pragma warning(disable:4482)

namespace Carbon {

	/**
	 * \brief 
	 * Pointer to thread pool instance.
	 */
	static ThreadPool* threadPool;
	/**
	 * \brief 
	 * Performs lazy initialization for thread pool.
	 * \return 
	 * Returns pointer to thread pool instance.
	 */
	static ThreadPool* GetThreadPool()
	{
		if (threadPool == nullptr)
		{
			return threadPool = new ThreadPool;
		} 
		else
		{
			return threadPool;
		}
	}

	namespace native
	{
		static std::shared_ptr<Node> view(std::vector<std::shared_ptr<Node>>& node); //forwarddecl
	};

	class SymbolTableStack {
	public:
		bool LocalMode;
		void Push();
		void Pop();
		std::shared_ptr<Node>& operator [](const std::string&);
		std::shared_ptr<Node>& Global(const std::string&);
		std::shared_ptr<Node>& Local(const std::string&);
		std::vector<std::string> GlobalKeys();
		int GetLevel();
		SymbolTableStack();
	private:
		std::vector<std::unordered_map<std::string, std::shared_ptr<Node>>> table;

	};

	class ExecutorImp {
	public:
		int ControlLevel;
		bool VERBOSE_SUBMIT;
		bool VERBOSE_TREE;
		bool VERBOSE_PERFORMANCE;
		bool ShowPrompt;
		SymbolTableStack SymbolTable;
		std::stack<std::shared_ptr<Node>> stack;
		std::vector<std::shared_ptr<Node>> StatementList;
		ThreadPool* threadPool;
		std::shared_ptr<Node> ReplaceIdIfPossible(std::shared_ptr<Node>);
		std::shared_ptr<Node> OptimizeIfPossible(std::shared_ptr<Node>);
		void RegisterNativeFunction(const char* name, native_function_ptr, bool pure);
		void RegisterInternalNativeFunction(const char* name, std::shared_ptr<Node> (*fptr)(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node), bool pure);
		ExecutorImp::ExecutorImp();
		std::shared_ptr<Node> ExecuteStatement(std::shared_ptr<Node>);
		std::shared_ptr<Node> ExecuteStatementList();
		void ClearStatementList();

		inline std::shared_ptr<Node> ExecuteInfixArithmetic(NodeCommand& node);
		inline std::shared_ptr<Node> ExecutePrefixArithmetic(NodeCommand& node);
		inline std::shared_ptr<Node> ExecuteAssignment(NodeCommand& node);
		inline std::shared_ptr<Node> ExecuteCall(NodeCommand& node);
		inline std::shared_ptr<Node> ExecuteCommand(NodeCommand& node);
		inline std::shared_ptr<Node> ExecuteBlock(NodeCommand& node);
		inline std::shared_ptr<Node> ExecuteConditional(NodeCommand& node);
		inline std::shared_ptr<Node> ExecuteLoop(NodeCommand& node);
		inline std::shared_ptr<Node> ExecuteMemberOperator(NodeCommand& node);
		inline std::shared_ptr<Node> Error(std::string message);
	};

	Executor::Executor() {
		this->imp = new ExecutorImp;
		this->SetInteractiveMode(false);
		this->imp->VERBOSE_SUBMIT = false;
		this->imp->VERBOSE_TREE = false;
		this->imp->VERBOSE_PERFORMANCE = true;
	}

	Executor::~Executor() {
		delete this->imp;
	}

	static int AsciiHexCharToInt(const char c) {
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		if (c >= 'a' && c <= 'f') return c - 'A' + 10;
		//invalid char
		return 0;
	}

	static std::string ParseEscapedString(const char* text) {
		std::string acc;
		char* p = (char*)text + 1;
		int ival;
		while (*p != '"') {
			if (*p == '\\') {
				p++;
				switch (*p) {
					case 'b': acc.push_back('\b');
						break;
					case 't': acc.push_back('\t');
						break;
					case 'n': acc.push_back('\n');
						break;
					case 'f': acc.push_back('\f');
						break;
					case 'r': acc.push_back('\r');
						break;
					case 'x': p++;
						ival = AsciiHexCharToInt(*p);
						p++;
						ival += AsciiHexCharToInt(*p);
						acc.push_back(ival);
						break;
					default: acc.push_back(*p);
						break;
				}
			} else {
				acc.push_back(*p);
			}
			p++;
		}
		return acc;
	}

	static binseq::bit_sequence ParseBSTR(const char* text) {
		auto p = (char*)text;
		unsigned i = 0;
		unsigned count = 0;
		for (unsigned i = 0; p[i]; i++)
			if (p[i] == '0' || p[i] == '1')
				count++;
		auto bs = binseq::bit_sequence();
		bs.reallocate(count);
		count = 0;
		for (unsigned i = 0; p[i]; i++) {
			if (p[i] == '0')
				bs[count++] = false;
			else if (p[i] == '1')
				bs[count++] = true;
		}
		return bs;
	}

	void Executor::WriteInstruction(InstructionType type, const char* text) {
		switch (type) {
			case InstructionType::NUM:
				imp->stack.push(std::make_shared<NodeInteger>(atol(text)));
				break;
			case InstructionType::ONUM:
				imp->stack.push(std::make_shared<NodeInteger>(std::stoi(text, nullptr,8)));
				break;
			case InstructionType::XNUM:
				if (text[0]!=0 && text[1]!=0)
					imp->stack.push(std::make_shared<NodeInteger>(std::stoi(text + 2, nullptr, 16)));
				else
					imp->stack.push(std::make_shared<NodeInteger>(std::stoi(text, nullptr, 16)));
				break;
			case InstructionType::BNUM:
				if (text[0] != 0 && text[1] != 0)
					imp->stack.push(std::make_shared<NodeInteger>(std::stoi(text + 2, nullptr, 2)));
				else
					imp->stack.push(std::make_shared<NodeInteger>(std::stoi(text, nullptr, 2)));
				break;
			case InstructionType::FLOAT:
				imp->stack.push(std::make_shared<NodeFloat>(atof(text)));
				break;
			case InstructionType::STR:
				imp->stack.push(std::make_shared<NodeString>(ParseEscapedString(text)));
				break;
			case InstructionType::BSTR:
				imp->stack.push(std::make_shared<NodeBits>(ParseBSTR(text)));
				break;
			default:
				imp->stack.push(std::make_shared<NodeAtom>(type, text));
				break;
		}

		if (imp->VERBOSE_SUBMIT)
			printf("ATM %d %s\n", type, imp->stack.top()->GetText());
	}

	void Executor::RegisterNativeFunction(const char* name, native_function_ptr fn, bool pure)
	{
		this->imp->RegisterNativeFunction(name, fn, pure);
	}

	void recursive_print(Node& node, int level = 0) {
		for (int i = 0; i < level; i++) printf("    ");
		printf("%s\n", node.GetText());
		if (node.IsCommand()) {
			auto x = reinterpret_cast<NodeCommand*>(&node);
			for (auto i = x->Children.begin(); i != x->Children.end(); i++) {
				recursive_print(**i, level + 1);
			}
		}
	}

	void Executor::WriteInstruction(InstructionType cmd) {
		//transform into tree
		auto node = std::make_shared<NodeCommand>(cmd);
		if (imp->VERBOSE_SUBMIT)
			printf("CMD %s\n", node->GetText());
		switch (cmd) {
			case InstructionType::ASSIGN:
			case InstructionType::ADD:
			case InstructionType::SUBTRACT:
			case InstructionType::MULTIPLY:
			case InstructionType::DIVIDE:
			case InstructionType::MEMBER:
			case InstructionType::COMP_EQ:
			case InstructionType::COMP_NE:
			case InstructionType::COMP_GE:
			case InstructionType::COMP_LE:
			case InstructionType::COMP_GT:
			case InstructionType::COMP_LT:
				// infix operators
				node->Children.resize(2);
				node->Children[1] = imp->ReplaceIdIfPossible(imp->stack.top());
				imp->stack.pop();
				node->Children[0] = (cmd == InstructionType::ASSIGN) ?
					                    imp->stack.top() : imp->ReplaceIdIfPossible(imp->stack.top());
				imp->stack.pop();
				imp->stack.push(imp->OptimizeIfPossible(node));
				break;
			case InstructionType::CONTROL:
				//imp->ControlLevel++;
				imp->SymbolTable.Push();
				break;
			case InstructionType::LOOP3:
				node->DoesPushStack = true;
				node->Children.resize(4);
				node->Children[3] = imp->stack.top();
				imp->stack.pop();
				node->Children[2] = imp->stack.top();
				imp->stack.pop();
				node->Children[1] = imp->stack.top();
				imp->stack.pop();
				node->Children[0] = imp->stack.top();
				imp->stack.pop();
				imp->stack.push(node);
				//imp->ControlLevel--;    
				imp->SymbolTable.Pop();
				goto end_statement;
				break;
				break;
			case InstructionType::LOOP2:
				node->DoesPushStack = true;
				node->Children.resize(3);
				node->Children[2] = imp->stack.top();
				imp->stack.pop();
				node->Children[1] = imp->stack.top();
				imp->stack.pop();
				node->Children[0] = imp->stack.top();
				imp->stack.pop();
				imp->stack.push(node);
				//imp->ControlLevel--;    
				imp->SymbolTable.Pop();
				goto end_statement;
				break;
			case InstructionType::LOOP0:
				node->DoesPushStack = true;
				node->Children.resize(1);
				node->Children[0] = imp->stack.top();
				imp->stack.pop();
				imp->stack.push(node);
				//imp->ControlLevel--;    
				imp->SymbolTable.Pop();
				goto end_statement;
				break;
			case InstructionType::LOOP1:
			case InstructionType::IF:
				node->DoesPushStack = true;
				node->Children.resize(2);
				node->Children[1] = imp->stack.top();
				imp->stack.pop();
				node->Children[0] = imp->stack.top();
				imp->stack.pop();
				imp->stack.push(node);
				//imp->ControlLevel--;    
				imp->SymbolTable.Pop();
				goto end_statement;
				break;
			case InstructionType::IFELSE:
				node->DoesPushStack = true;
				node->Children.resize(3);
				node->Children[2] = imp->stack.top();
				imp->stack.pop();
				node->Children[1] = imp->stack.top();
				imp->stack.pop();
				node->Children[0] = imp->stack.top();
				imp->stack.pop();
				imp->stack.push(node);
				//imp->ControlLevel--;    
				imp->SymbolTable.Pop();
				goto end_statement;
				break;
			case InstructionType::FUNCTION_OPERATOR: {
				auto fimpl = imp->stack.top();
				imp->stack.pop();
				auto params = imp->stack.top();
				imp->stack.pop();
				std::vector<std::string> plist;
				if (params->IsAtom()) {
					plist.push_back(params->GetAtomText());
				}
				else throw ExecutorImplementationException("only simple function epxressions");
				imp->stack.push(std::make_shared<NodeFunction>(plist, fimpl));
				break;
			}
			case InstructionType::FUNCTIONEND: {
				auto fimpl = imp->stack.top();
				imp->stack.pop();
				std::stack<std::string> pstack;
				while (true) {
					auto top = imp->stack.top();
					imp->stack.pop();
					if (top->IsAtom()) {
						auto& atom = reinterpret_cast<NodeAtom&>(*top);
						pstack.push(atom.AtomText);
					} else if (top->IsCommand()) {
						auto& cmd = reinterpret_cast<NodeCommand&>(*top);
						if (cmd.CommandType == InstructionType::FUNCTIONBEGIN) {
							std::vector<std::string> plist;
							while (!pstack.empty()) {
								plist.push_back(pstack.top());
								pstack.pop();
							}
							imp->stack.push(std::make_shared<NodeFunction>(plist, fimpl));
							break;
						} else throw ExecutorImplementationException("invalid");
					} else throw ExecutorImplementationException("invalid");
				}
			}
				imp->SymbolTable.Pop();
				break;
			case InstructionType::FUNCTIONBEGIN:
				imp->SymbolTable.Push();
				imp->stack.push(node);
				break;
			case InstructionType::ARRAYBEGIN:
			case InstructionType::OBJECTBEGIN:
			case InstructionType::RETURN0:
			case InstructionType::BREAK:
			case InstructionType::CONTINUE:
				imp->stack.push(node);
				break;
			case InstructionType::ARRAYEND: {
				auto arrayNode = std::make_shared<NodeArray>();
				auto& vector = arrayNode->Vector;
				bool finished = false;
				while(!finished) {
					auto& top = imp->stack.top();
					if (top->IsCommand() && top->GetCommandType() == InstructionType::ARRAYBEGIN)
					{
						imp->stack.pop();
						std::reverse(vector.begin(), vector.end());
						imp->stack.push(arrayNode);
						finished = true;
					} else {
						vector.push_back(top);
						imp->stack.pop();
					}
				}
			}
				break;
			case InstructionType::OBJECTEND: {
				std::vector<std::shared_ptr<Node>> collector;
				bool finished = false;
				while(!finished)
				{
					auto& top = imp->stack.top();
					if (top->IsCommand() && top->GetCommandType() == InstructionType::OBJECTBEGIN)
					{
						imp->stack.pop();
						auto objectNode = std::make_shared<NodeObject>();
						for (int i=collector.size(); i>0; i-=2)
						{
							auto& key = reinterpret_cast<NodeAtom&>(*collector[i - 1]);
							if (key.GetNodeType() == NodeType::String)
							{
								auto& str = reinterpret_cast<NodeString&>(key);
								// map[str.Value] = collector[i - 2];
								objectNode->SetAttributeValue(str.Value, collector[i - 2]);
							} else
							{
								objectNode->SetAttributeValue(key.AtomText, collector[i - 2]);
							}
						}
						imp->stack.push(objectNode);
						finished = true;
					}
					else
					{
						collector.push_back(top);
						imp->stack.pop();
						collector.push_back(imp->stack.top());
						imp->stack.pop();
					}
				}

			}
				break;
			case InstructionType::RETURN1:
			case InstructionType::LOCAL:
			case InstructionType::CALLBEGIN:
			case InstructionType::POSITIVE:
			case InstructionType::NEGATIVE:
				// prefix operators
				node->Children.resize(1);
				node->Children[0] = imp->stack.top();
				imp->stack.pop();
				imp->stack.push(imp->OptimizeIfPossible(node));
				break;
			case InstructionType::CALLEND: {
				//compile the call list
				std::stack<std::shared_ptr<Node>> callstack;
				bool finished = false;
				do {
					auto top = imp->stack.top();
					if (top->IsCommand()) {
						auto call = reinterpret_cast<NodeCommand*>(&*top);
						if (call->CommandType == InstructionType::CALLBEGIN) {
							call->Children.reserve(callstack.size() + 1);
							while (!callstack.empty()) {
								call->Children.push_back(imp->ReplaceIdIfPossible(callstack.top()));
								callstack.pop();
							}
							// callbegin and callend are discarded
							// and replaced with a single CALL command
							call->CommandType = InstructionType::CALL;
							finished = true;
						}
					}
					if (!finished) {
						callstack.push(top);
						imp->stack.pop();
					}
				} while (!finished);
			}
				break;
			case InstructionType::BLOCKBEGIN:
				node->DoesPushStack = true;
				imp->SymbolTable.Push();
				imp->stack.push(node);
				break;
			case InstructionType::BLOCKEND: {
				//compile the call list
				std::stack<std::shared_ptr<Node>> blockstack;
				bool finished = false;
				do {
					auto top = imp->stack.top();
					if (top->IsCommand()) {
						auto call = reinterpret_cast<NodeCommand*>(&*top);
						if (call->CommandType == InstructionType::BLOCKBEGIN) {
							call->Children.reserve(blockstack.size());
							while (!blockstack.empty()) {
								call->Children.push_back(imp->ReplaceIdIfPossible(blockstack.top()));
								blockstack.pop();
							}
							// callbegin and callend are discarded
							// and replaced with a single CALL command
							call->CommandType = InstructionType::BLOCK;
							finished = true;
						}
					}
					if (!finished) {
						blockstack.push(top);
						imp->stack.pop();
					}
				} while (!finished);
				imp->SymbolTable.Pop();
				if (imp->stack.size() != 1) {
					break;
				}
			}
				//passthrough
			case InstructionType::END_STATEMENT:
			end_statement:
				if (imp->SymbolTable.GetLevel() == 1 && imp->ControlLevel == 0) {
					if (imp->stack.empty() == true)
					{
						// if stack is empty we have nothing left to do
						break; // exit early
					}
					if (imp->VERBOSE_TREE) {
						recursive_print(*(imp->stack.top()));
					}
					//save the statement
					this->imp->StatementList.push_back(imp->stack.top());
					//clear the stack
					while (!imp->stack.empty()) imp->stack.pop();
					if (imp->ShowPrompt) {
						//if console mode, execute
						std::vector<std::shared_ptr<Node>> args;

						std::chrono::steady_clock::time_point t1;
						// if we need to know perf, we need to know time before starting execution
						if (imp->VERBOSE_PERFORMANCE == true) t1 = std::chrono::high_resolution_clock::now(); 
						// execute recorded statements
						auto result = imp->ExecuteStatementList();
						args.push_back(result);
						args.push_back(std::make_shared<NodeString>(std::string(":") + GetTypeText(result->GetNodeType())));
						if (imp->VERBOSE_PERFORMANCE == true)
						{
							// if perf is enabled, check time now and print delta
							std::chrono::steady_clock::time_point t2 = std::chrono::high_resolution_clock::now();
							auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count(); 
							double ms = ns / 1.0e6;
							std::ostringstream strs;
							strs << "(" << ms << "ms" << ")";
							std::string str = strs.str();
							args.push_back(std::make_shared<NodeString>(strs.str()));
						}
						
						native::view(args);
						imp->ClearStatementList();
						printf(">> ");
					}
				}
				
				break;
			default: throw ExecutorImplementationException("Unhandled command.");
		}

		//operations   
		//if (imp->ControlLevel!=imp->SymbolTable.GetLevel())
		//{
		/*if (cmd == InstructionType::ASSIGN)                     
    {                                                   
      auto symbol = node->Children[0];                   
      auto value = node->Children[1];                    
      if (symbol->IsAtom())           
      {                                                 
        auto atom = reinterpret_cast<NodeAtom*>(&*symbol);
        if (atom->GetAtomType() == InstructionType::ID)          
        {
          imp->SymbolTable.Local(atom->AtomText) = value;
        }
      }
    }*/

	};

	std::shared_ptr<Node> ExecutorImp::ReplaceIdIfPossible(std::shared_ptr<Node> node) {
		/*if (node->IsAtom())
    {
      auto atom = reinterpret_cast<NodeAtom*>(&*node);
      if (atom->AtomType == InstructionType::ID)
      {
        auto lookup = SymbolTable.Local(atom->AtomText);
        if (lookup != nullptr) return lookup;
      }
    }*/

		return node; //unmodified
	}

	const char* Executor::getLastAtomText() {
		return imp->stack.top()->GetText();
	}

	std::shared_ptr<Node> ExecutorImp::OptimizeIfPossible(std::shared_ptr<Node> node) {
		if (node->IsCommand()) {
			auto cmd = reinterpret_cast<NodeCommand*>(&*node);
			if (cmd->Children.size() == 1) {
				auto cmdtype = cmd->GetCommandType();
				if (cmdtype == InstructionType::POSITIVE) {
					if (cmd->Children[0]->GetNodeType() == NodeType::Integer ||
						cmd->Children[0]->GetNodeType() == NodeType::Float)
						return cmd->Children[0];
				} else if (cmdtype == InstructionType::NEGATIVE) {
					if (cmd->Children[0]->GetNodeType() == NodeType::Integer)
						return std::make_shared<NodeInteger>(- reinterpret_cast<NodeInteger*>(&*cmd->Children[0])->Value);
					if (cmd->Children[0]->GetNodeType() == NodeType::Float)
						return std::make_shared<NodeFloat>(- reinterpret_cast<NodeFloat*>(&*cmd->Children[0])->Value);
				}

			} else if (cmd->Children.size() == 2) {
				auto cmdtype = cmd->GetCommandType();

				//try reduce arithmetic nodes
				if (cmdtype == InstructionType::ADD || cmdtype == InstructionType::SUBTRACT ||
					cmdtype == InstructionType::MULTIPLY || cmdtype == InstructionType::DIVIDE) {
					//reduce integer arithmetic
					if (cmd->Children[0]->GetNodeType() == NodeType::Integer &&
						cmd->Children[1]->GetNodeType() == NodeType::Integer) {
						auto a = reinterpret_cast<NodeInteger*>(&*cmd->Children[0]);
						auto b = reinterpret_cast<NodeInteger*>(&*cmd->Children[1]);
						switch (cmdtype) {
							case InstructionType::ADD: return std::make_shared<NodeInteger>(a->Value + b->Value);
							case InstructionType::SUBTRACT: return std::make_shared<NodeInteger>(a->Value - b->Value);
							case InstructionType::MULTIPLY: return std::make_shared<NodeInteger>(a->Value * b->Value);
							case InstructionType::DIVIDE:
								if (b->Value == 0) {
									throw ExecutorRuntimeException("Division by zero.");
									return node;
								};
								return std::make_shared<NodeInteger>(a->Value / b->Value);
						}
					}

					//reduce float arithmetic
					if (cmd->Children[0]->GetNodeType() == NodeType::Float &&
						cmd->Children[1]->GetNodeType() == NodeType::Float) {
						auto a = reinterpret_cast<NodeFloat*>(&*cmd->Children[0]);
						auto b = reinterpret_cast<NodeFloat*>(&*cmd->Children[1]);
						switch (cmdtype) {
							case InstructionType::ADD: return std::make_shared<NodeFloat>(a->Value + b->Value);
							case InstructionType::SUBTRACT: return std::make_shared<NodeFloat>(a->Value - b->Value);
							case InstructionType::MULTIPLY: return std::make_shared<NodeFloat>(a->Value * b->Value);
							case InstructionType::DIVIDE:
								return std::make_shared<NodeFloat>(a->Value / b->Value);
						}
					}
				}
			}
		}
		return node;
	}

	static char integerTextBuffer[256];

	const char* NodeInteger::GetText() {
		sprintf_s<sizeof(integerTextBuffer)>(integerTextBuffer, "%lld", this->Value);
		return integerTextBuffer;
	}

	const char* NodeFloat::GetText() {
		if (text == nullptr) {
			char buffer[256];
			sprintf_s<sizeof(buffer)>(buffer, "%g", this->Value);
			text = std::make_shared<std::string>(buffer);
		}
		return text->c_str();
	}

	const char* NodeString::GetText() {
		return Value.c_str();
	}

	NodeString::NodeString(const std::string str) : Node(NodeType::String), Value(str) { }

	bool Executor::GetInteractiveMode() {
		return this->imp->ShowPrompt;
	}

	void Executor::SetInteractiveMode(bool interactive) {
		this->imp->ShowPrompt = interactive;
		if (interactive)
			printf(">> ");
	}

	void Executor::ClearStatement() {
		while (!this->imp->stack.empty())
			this->imp->stack.pop();
		imp->ClearStatementList();
	}

	std::shared_ptr<Node> Executor::Execute() {
		auto node = imp->ExecuteStatementList();
		imp->ClearStatementList();
		return node;
	}

	int SymbolTableStack::GetLevel() {
		return table.size();
	}

	SymbolTableStack::SymbolTableStack() {
		this->LocalMode = false;
		this->Push();
	};

	void SymbolTableStack::Push() {
		table.push_back(std::unordered_map<std::string, std::shared_ptr<Node>>());
	}

	void SymbolTableStack::Pop() {
		table.pop_back();
	}

	std::shared_ptr<Node>& SymbolTableStack::operator[](const std::string& key) {
		if (!LocalMode)
			for (auto i = table.rbegin(); i != table.rend(); i++) {
				if (i->find(key) != i->end())
					return (*i)[key];
			}
		return table.back()[key];
	}

	std::shared_ptr<Node>& SymbolTableStack::Global(const std::string& key) {
		return table.front()[key];
	}

	std::vector<std::string> SymbolTableStack::GlobalKeys() {
		std::vector<std::string> result;
		auto& global = table.front();
		for (auto i = global.begin(); i != global.end(); i++) {
			result.push_back(i->first);
		}
		return result;
	}

	std::shared_ptr<Node>& SymbolTableStack::Local(const std::string& key) {
		return table.back()[key];
	}

	NodeFunction::NodeFunction(native_function_ptr fptr, bool pure)
		: Node(NodeType::Function), Native(true), nativeptr(fptr), Pure(pure), InternalNative(false) { }

	NodeFunction::NodeFunction(std::vector<std::string> parameterList, std::shared_ptr<Node> impl)
		:Node(NodeType::Function), Native(false), nativeptr(nullptr), ParameterList(parameterList), Implementation(impl), InternalNative(false), Pure(false) {}

	const char* NodeFunction::GetText() {
		if (Native) return "native function";
		else return "function";
	}

	NodeBit::NodeBit(bool value) : Value(value), Node(NodeType::Bit) {};

	const char* NodeBit::GetText() {
		return Value ? "1" : "0";
	};

	NodeInteger::NodeInteger(long long value) : Node(NodeType::Integer), Value(value) { }

	NodeFloat::NodeFloat(double value) : Node(NodeType::Float), Value(value), text(nullptr) { }


	// a native function receives a vector of nodes and retursn another node
	typedef std::shared_ptr<Node>(*internal_native_function_ptr)(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node);

	void ExecutorImp::RegisterNativeFunction(const char* name, native_function_ptr fptr, bool pure) {
		auto newfunction = std::make_shared<NodeFunction>(fptr, pure);
		newfunction->InternalNative = false;
		auto& symbol = SymbolTable.Global(name);
		symbol = newfunction;
	}

	void ExecutorImp::RegisterInternalNativeFunction(const char* name, internal_native_function_ptr fptr, bool pure) {
		auto newfunction = std::make_shared<NodeFunction>(reinterpret_cast<native_function_ptr>(fptr), pure);
		newfunction->InternalNative = true;
		auto& symbol = SymbolTable.Global(name);
		symbol = newfunction;
	}

	void ExecutorImp::ClearStatementList() {
		this->StatementList.clear();
	}

	std::shared_ptr<Node> ExecutorImp::ExecuteStatementList() {
		std::shared_ptr<Node> node;
		for (auto i = StatementList.begin(); i != StatementList.end(); ++i) {
			node = ExecuteStatement(*i);
			if (node->GetNodeType() == NodeType::Error) {
				if (!ShowPrompt) {
					fprintf(stderr, "Execution halted to prevent unknown sidefects.\n");
				} else { }
			}
		}
		return node;
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecutePrefixArithmetic(NodeCommand& node) {
		if (node.Children.size() == 1) {
			auto executed = ExecuteStatement(node.Children[0]);
			switch (executed->GetNodeType()) {
				case NodeType::Integer: {
					auto& integer = reinterpret_cast<NodeInteger&>(*executed);
					switch (node.CommandType) {
						case InstructionType::POSITIVE: return executed;
						case InstructionType::NEGATIVE: return std::make_shared<NodeInteger>(-integer.Value);
						default: throw ExecutorImplementationException("unexpected commandtype");
					}
				}
				case NodeType::Float: {
					auto& fval = reinterpret_cast<NodeFloat&>(*executed);
					switch (node.CommandType) {
						case InstructionType::POSITIVE: return executed;
						case InstructionType::NEGATIVE: std::make_shared<NodeFloat>(-fval.Value);
						default: throw ExecutorImplementationException("unexpected commandtype");
					}
				}
				case NodeType::Bit: {
					auto& bval = reinterpret_cast<NodeBit&>(*executed);
					switch (node.CommandType) {
						case InstructionType::POSITIVE: return executed;
						case InstructionType::NEGATIVE: return std::make_shared<NodeBit>(!bval.Value);
					}
				}
				default:
					throw ExecutorRuntimeException("prefix operators + or - only work on numeric values");
			}
		} else throw ExecutorImplementationException("Prefix op can only have 1 child.");
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteInfixArithmetic(NodeCommand& node) {
		if (node.Children.size() >= 2) {
			std::vector<std::shared_ptr<Node>> executed;
			for (auto i = node.Children.begin(); i != node.Children.end(); i++) {
				executed.push_back(ExecuteStatement(*i));
			}
			// swap none to always be first child
			if (node.Children.size() == 2 &&
				(node.CommandType == InstructionType::COMP_EQ || node.CommandType == InstructionType::COMP_NE) &&
				executed[1]->GetNodeType() == NodeType::None) {
				std::swap(node.Children[0], node.Children[1]); //optimize, next run wont enter this branch
				std::swap(executed[0], executed[1]);
			}
		reexecute:
			switch (executed[0]->GetNodeType()) {
				case NodeType::Integer: {
					if (node.CommandType == InstructionType::MULTIPLY &&
						(executed[1]->GetNodeType() == NodeType::String ||
						executed[1]->GetNodeType() == NodeType::Bits ||
						executed[1]->GetNodeType() == NodeType::DynamicArray)) {
						std::swap(node.Children[0], node.Children[1]); //optimize, next run wont enter this branch
						std::swap(executed[0], executed[1]);
						goto reexecute;
					}
					if (node.CommandType == InstructionType::ADD ||
						node.CommandType == InstructionType::SUBTRACT ||
						node.CommandType == InstructionType::MULTIPLY ||
						node.CommandType == InstructionType::DIVIDE) {
						auto i = executed.begin();
						auto acc = std::make_shared<NodeInteger>(reinterpret_cast<NodeInteger*>(&**i)->Value);
						for (i++; i != executed.end(); i++) {
							if ((*i)->GetNodeType() != NodeType::Integer) throw ExecutorRuntimeException("unexpected type in an arithmetic integer expression");
							auto val = reinterpret_cast<NodeInteger*>(&**i)->Value;
							switch (node.CommandType) {
								case InstructionType::ADD: acc->Value += val;
									break;
								case InstructionType::SUBTRACT: acc->Value -= val;
									break;
								case InstructionType::MULTIPLY: acc->Value *= val;
									break;
								case InstructionType::DIVIDE: if (val == 0) throw ExecutorRuntimeException("integer division by 0"); else acc->Value /= val;
									break;
							}
						}
						return acc;
					} else if (executed[0]->GetNodeType() == executed[1]->GetNodeType()) {
						auto& a = reinterpret_cast<NodeInteger&>(*executed[0]);
						auto& b = reinterpret_cast<NodeInteger&>(*executed[1]);
						switch (node.CommandType) {
							case InstructionType::COMP_EQ: return std::make_shared<NodeBit>(a.Value == b.Value);
							case InstructionType::COMP_NE: return std::make_shared<NodeBit>(a.Value != b.Value);
							case InstructionType::COMP_LE: return std::make_shared<NodeBit>(a.Value <= b.Value);
							case InstructionType::COMP_GE: return std::make_shared<NodeBit>(a.Value >= b.Value);
							case InstructionType::COMP_LT: return std::make_shared<NodeBit>(a.Value < b.Value);
							case InstructionType::COMP_GT: return std::make_shared<NodeBit>(a.Value > b.Value);
						}
					} else throw ExecutorRuntimeException("error in expression");

				}
				case NodeType::Float: {

					if (node.CommandType == InstructionType::ADD ||
						node.CommandType == InstructionType::SUBTRACT ||
						node.CommandType == InstructionType::MULTIPLY ||
						node.CommandType == InstructionType::DIVIDE) {
						auto i = executed.begin();
						auto acc = std::make_shared<NodeFloat>(reinterpret_cast<NodeFloat*>(&**i)->Value);
						for (i++; i != executed.end(); i++) {
							if ((*i)->GetNodeType() != NodeType::Float) throw ExecutorRuntimeException("unexpected type in an arithmetic float expression");
							auto val = reinterpret_cast<NodeFloat*>(&**i)->Value;
							switch (node.CommandType) {
								case InstructionType::ADD: acc->Value += val;
									break;
								case InstructionType::SUBTRACT: acc->Value -= val;
									break;
								case InstructionType::MULTIPLY: acc->Value *= val;
									break;
								case InstructionType::DIVIDE: acc->Value /= val;
									break;
							}
						}
						return acc;
					} else if (executed[0]->GetNodeType() == executed[1]->GetNodeType()) {
						auto& a = reinterpret_cast<NodeFloat&>(*executed[0]);
						auto& b = reinterpret_cast<NodeFloat&>(*executed[1]);
						switch (node.CommandType) {
							case InstructionType::COMP_EQ: return std::make_shared<NodeBit>(a.Value == b.Value);
							case InstructionType::COMP_NE: return std::make_shared<NodeBit>(a.Value != b.Value);
							case InstructionType::COMP_LE: return std::make_shared<NodeBit>(a.Value <= b.Value);
							case InstructionType::COMP_GE: return std::make_shared<NodeBit>(a.Value >= b.Value);
							case InstructionType::COMP_LT: return std::make_shared<NodeBit>(a.Value < b.Value);
							case InstructionType::COMP_GT: return std::make_shared<NodeBit>(a.Value > b.Value);
						}
					}
				}
				case NodeType::String: {
					//if (executed[1]->GetNodeType()!=NodeType::String) throw ExecutorRuntimeException("non string value in string expression");
					auto& left = reinterpret_cast<NodeString&>(*executed[0]);
					NodeString* right = nullptr;

					if (node.CommandType != InstructionType::MULTIPLY) {
						if (executed[1]->GetNodeType() != NodeType::String) {
							throw ExecutorRuntimeException("non string value in string expression");
						} else {
							right = reinterpret_cast<NodeString*>(&*executed[1]);
						}
					}

					switch (node.CommandType) {
						case InstructionType::ADD: {
							std::string acc = left.Value;
							for (unsigned i = 1; i < executed.size(); i++) {
								if (executed[i]->GetNodeType() == NodeType::String) {
									acc += reinterpret_cast<NodeString&>(*executed[i]).Value;
								} else throw ExecutorRuntimeException("non string value in string concatenation");
							}
							return std::make_shared<NodeString>(acc);
						}
						case InstructionType::MULTIPLY:
							if (executed.size() == 2) {
								if (executed[1]->GetNodeType() == NodeType::Integer) {
									auto ival = reinterpret_cast<NodeInteger&>(*executed[1]).Value;
									std::string result;
									result.reserve(left.Value.size() * ival);
									while (ival--) result += left.Value;
									return std::make_shared<NodeString>(result);
								} else throw ExecutorRuntimeException("multiplication is not defined between the given arguments (try string * int)");
							} else throw ExecutorRuntimeException("multiplication when left side is string is only valid with an integer");
							break;
						case InstructionType::COMP_EQ: return std::make_shared<NodeBit>(left.Value == right->Value);
						case InstructionType::COMP_NE: return std::make_shared<NodeBit>(left.Value != right->Value);
						case InstructionType::COMP_GE: return std::make_shared<NodeBit>(left.Value >= right->Value);
						case InstructionType::COMP_LE: return std::make_shared<NodeBit>(left.Value <= right->Value);
						case InstructionType::COMP_GT: return std::make_shared<NodeBit>(left.Value > right->Value);
						case InstructionType::COMP_LT: return std::make_shared<NodeBit>(left.Value < right->Value);
						default: throw ExecutorRuntimeException("string doesn't support the requested command");
					}
				}
				case NodeType::Bits: {
					//if (executed[1]->GetNodeType()!=NodeType::String) throw ExecutorRuntimeException("non string value in string expression");
					auto& left = reinterpret_cast<NodeBits&>(*executed[0]);
					NodeBits* right = nullptr;

					if (node.CommandType != InstructionType::MULTIPLY) {
						if (executed[1]->GetNodeType() != NodeType::Bits) {
							throw ExecutorRuntimeException("non binseq value in string expression");
						} else {
							right = reinterpret_cast<NodeBits*>(&*executed[1]);
						}
					}

					switch (node.CommandType) {
						case InstructionType::ADD: {
							binseq::bit_sequence acc = left.Value;
							for (unsigned i = 1; i < executed.size(); i++) {
								if (executed[i]->GetNodeType() == NodeType::Bits) {
									acc = acc + reinterpret_cast<NodeBits&>(*executed[i]).Value;
								} else throw ExecutorRuntimeException("non binseq value in binseq concatenation");
							}
							return std::make_shared<NodeBits>(std::move(acc));
						}
						case InstructionType::MULTIPLY:
							if (executed.size() == 2) {
								if (executed[1]->GetNodeType() == NodeType::Integer) {
									auto ival = reinterpret_cast<NodeInteger&>(*executed[1]).Value;
									binseq::bit_sequence result;
									while (ival--) result = result + left.Value;
									return std::make_shared<NodeBits>(std::move(result));
								} else throw ExecutorRuntimeException("multiplication is not defined between the given arguments (try binseq * int)");
							} else throw ExecutorRuntimeException("multiplication when left side is binseq is only valid with an integer");
							break;
						case InstructionType::COMP_EQ: return std::make_shared<NodeBit>(left.Value == right->Value);
						case InstructionType::COMP_NE: return std::make_shared<NodeBit>(left.Value != right->Value);
						case InstructionType::COMP_GE: return std::make_shared<NodeBit>(left.Value >= right->Value);
						case InstructionType::COMP_LE: return std::make_shared<NodeBit>(left.Value <= right->Value);
						case InstructionType::COMP_GT: return std::make_shared<NodeBit>(left.Value > right->Value);
						case InstructionType::COMP_LT: return std::make_shared<NodeBit>(left.Value < right->Value);
						default: throw ExecutorRuntimeException("binseq doesn't support the requested command");
					}
				}
				case NodeType::DynamicArray: {
					auto& left = reinterpret_cast<NodeArray&>(*executed[0]);
					NodeArray* right = nullptr;

					if (node.CommandType != InstructionType::MULTIPLY) {
						if (executed[1]->GetNodeType() != NodeType::DynamicArray) {
							throw ExecutorRuntimeException("non array value in string expression");
						} else {
							right = reinterpret_cast<NodeArray*>(&*executed[1]);
						}
					}

					switch (node.CommandType) {
						case InstructionType::ADD: {
							auto acc = std::make_shared<NodeArray>();
							for (unsigned i = 1; i < executed.size(); i++) {
								acc->Vector = left.Vector; //copy
								if (executed[i]->GetNodeType() == NodeType::DynamicArray) {
									auto& other = reinterpret_cast<NodeArray&>(*executed[i]).Vector;
									for (auto i = other.begin(); i != other.end(); i++) {
										acc->Vector.push_back(*i);
									}
								} else throw ExecutorRuntimeException("non array value in array concatenation");
							}
							return acc;
						}
						case InstructionType::MULTIPLY:
							if (executed.size() == 2) {
								if (executed[1]->GetNodeType() == NodeType::DynamicArray) {
									auto ival = reinterpret_cast<NodeInteger&>(*executed[1]).Value;
									auto acc = std::make_shared<NodeArray>();
									while (ival--) {
										auto& other = left.Vector;
										for (auto i = other.begin(); i != other.end(); i++) {
											acc->Vector.push_back(*i);
										}
									};
									return acc;
								} else throw ExecutorRuntimeException("multiplication is not defined between the given arguments (try array * int)");
							} else throw ExecutorRuntimeException("multiplication when left side is array is only valid with an integer");
							break;
						default: throw ExecutorRuntimeException("binseq doesn't support the requested command");
					}
				}
				case NodeType::Bit: {
					if (node.CommandType == InstructionType::ADD ||
						node.CommandType == InstructionType::MULTIPLY) {
						auto i = executed.begin();
						auto acc = std::make_shared<NodeBit>(reinterpret_cast<NodeBit*>(&**i)->Value);
						for (i++; i != executed.end(); i++) {
							if ((*i)->GetNodeType() != NodeType::Bit) throw ExecutorRuntimeException("unexpected type in a bit expression");
							auto val = reinterpret_cast<NodeBit*>(&**i)->Value;
							switch (node.CommandType) {
								case InstructionType::ADD: acc->Value = acc->Value || val;
									break;
								case InstructionType::MULTIPLY: acc->Value = acc->Value && val;
									break;
							}
						}
						return acc;
					} else if (executed[0]->GetNodeType() == executed[1]->GetNodeType()) {
						auto& a = reinterpret_cast<NodeBit&>(*executed[0]);
						auto& b = reinterpret_cast<NodeBit&>(*executed[1]);
						switch (node.CommandType) {
							case InstructionType::COMP_EQ: return std::make_shared<NodeBit>(a.Value == b.Value);
							case InstructionType::COMP_NE: return std::make_shared<NodeBit>(a.Value != b.Value);
							default: throw ExecutorRuntimeException("invalid expression");
						}
					} else throw ExecutorRuntimeException("error in expression");

				}
				case NodeType::None: {
					if (executed.size() != 2) {
						throw ExecutorRuntimeException("expression with void are only valid with 2 operands");
					}
					switch (node.CommandType) {
						case InstructionType::COMP_EQ: return std::make_shared<NodeBit>(executed[1]->GetNodeType() == NodeType::None);
						case InstructionType::COMP_NE: return std::make_shared<NodeBit>(executed[1]->GetNodeType() != NodeType::None);
						default: throw ExecutorRuntimeException("cannot perform requested operation on void type");
					}
				}
				default:
					throw ExecutorRuntimeException(std::string("arithmetic operators not implemented for ") + (GetTypeText(executed[0]->GetNodeType())));
			}
		} else throw ExecutorImplementationException("arithmetic operators need to have at least 2 arguments");

	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteMemberOperator(NodeCommand& node) {
		if (node.Children.size() == 2) {
			auto lvalue = ExecuteStatement(node.Children[0]);
			auto rvalue = reinterpret_cast<Node*>(&*node.Children[1]);
			if (rvalue->GetNodeType() == NodeType::Atom) {
				auto atom = reinterpret_cast<NodeAtom*>(rvalue);
				if (atom->GetAtomType() == InstructionType::ID) {
					auto& idx = atom->AtomText;
					if (lvalue->GetNodeType() == NodeType::DynamicObject) {
						auto& object = reinterpret_cast<NodeObject&>(*lvalue);
						auto result = object.GetAttributeValue(idx);
						if (result != nullptr)
							return result;
						else
							return std::make_shared<Node>(NodeType::None);
					}
					else throw ExecutorRuntimeException("left side of an object member operator must be an object");
				}
				else throw ExecutorRuntimeException(atom->AtomText + " is not a valid indentifier");
			}
			else throw ExecutorRuntimeException("right side of an object member operator must be an identifier");
		}
		else throw ExecutorImplementationException("Member operator requires 2 parameters.");
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteAssignment(NodeCommand& node) {
		if (node.Children.size() == 2) {
			Node* lvalue = &*node.Children[0];

			auto savedLocalState = SymbolTable.LocalMode;
			SymbolTable.LocalMode = false;
			auto rvalue = ExecuteStatement(node.Children[1]);
			SymbolTable.LocalMode = savedLocalState;

			bool isLocalAssign = false;
			if (lvalue->IsCommand() && lvalue->GetCommandType() == InstructionType::LOCAL) {
				lvalue = &*(reinterpret_cast<NodeCommand*>(lvalue))->Children[0];
				isLocalAssign = true;
			}

			if (lvalue->GetNodeType() == NodeType::Atom) {
				auto atom = reinterpret_cast<NodeAtom*>(lvalue);
				if (atom->GetAtomType() == InstructionType::ID) {
					if (isLocalAssign) {
						return this->SymbolTable.Local(atom->AtomText) = rvalue;
					}
					else {
						return this->SymbolTable[atom->AtomText] = rvalue;
					}
				} else throw ExecutorRuntimeException(atom->AtomText + " is not a valid indentifier");
			} 
			else if (lvalue->GetCommandType() == InstructionType::MEMBER)
			{
				auto& member = reinterpret_cast<NodeCommand&>(*lvalue);
				auto container = ExecuteStatement(member.Children[0]);
				if (container->GetNodeType() != NodeType::DynamicObject)
					throw ExecutorRuntimeException("left side of member operator is not an object");
				auto& object = reinterpret_cast<NodeObject&>(*container);
				auto& index = member.Children[1];
				if (index->GetAtomType() != InstructionType::ID)
					throw ExecutorRuntimeException("right side of member oeprator is not an identifier");
				auto& idx = reinterpret_cast<NodeAtom&>(*index).AtomText;
				object.SetAttributeValue(idx, rvalue);
				return rvalue;
			}
			throw ExecutorRuntimeException("left side of an assignment must be an identifier");
		} else throw ExecutorImplementationException("Assignment requires 2 parameters.");
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteBlock(NodeCommand& node) {
		std::shared_ptr<Node> result;
		for (auto i = node.Children.begin(); i != node.Children.end(); i++) {
			result = ExecuteStatement(*i);
			switch (result->GetNodeType()) {
				case NodeType::Error: return result;
				case NodeType::Return: return result;
				case NodeType::Break: return result;
				case NodeType::Continue: return result;
				default: continue;
			}
		}
		return result;
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteConditional(NodeCommand& node) {
		std::shared_ptr<Node> result;
		auto cond = ExecuteStatement(node.Children[0]);
		if (cond->GetNodeType() == NodeType::Bit) {
			if (reinterpret_cast<NodeBit&>(*cond).Value) {
				return ExecuteStatement(node.Children[1]);
			} else if (node.Children.size() == 3) {
				return ExecuteStatement(node.Children[2]);
			} else {
				return std::make_shared<Node>(NodeType::None);
			}
		} else throw ExecutorRuntimeException("condition is not a bit");
		return std::make_shared<Node>(NodeType::None);
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteLoop(NodeCommand& node) {
		std::shared_ptr<Node> result, init = nullptr, cond = nullptr, iterate = nullptr, body;
		switch (node.Children.size()) {
			case 1:
				cond = std::make_shared<NodeBit>(true);
				body = node.Children[0];
				break;
			case 2:
				cond = node.Children[0];
				body = node.Children[1];
				break;
			case 3:
				cond = node.Children[0];
				iterate = node.Children[1];
				body = node.Children[2];
				break;
			case 4:
				init = node.Children[0];
				cond = node.Children[1];
				iterate = node.Children[2];
				body = node.Children[3];
				break;
			default: throw ExecutorImplementationException("loop expects 1 or 2 or 3 or 4 children");
		}

		if (init != nullptr) ExecuteStatement(init);

		while (true) {
			auto condResult = ExecuteStatement(cond);
			if (condResult->GetNodeType() == NodeType::Bit) {
				if (reinterpret_cast<NodeBit&>(*condResult).Value) {
					auto result = ExecuteStatement(body);
					if (result == nullptr || result->GetNodeType() == NodeType::Break)
						return std::make_shared<Node>(NodeType::None);
					if (result->GetNodeType() == NodeType::Return)
						return result;
					if (iterate != nullptr) ExecuteStatement(iterate);
				} else {
					break;
				}
			} else throw ExecutorRuntimeException("condition did not evaluate to a bit");
		}
		return std::make_shared<Node>(NodeType::None);
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteCommand(NodeCommand& node) {
		std::shared_ptr<Node> result;
		const char* imp_error = nullptr;
		bool pushpop = node.DoesPushStack;

		if (pushpop)
			SymbolTable.Push();

		switch (node.CommandType) {
				//arithmetic
			case InstructionType::ADD:
			case InstructionType::SUBTRACT:
			case InstructionType::MULTIPLY:
			case InstructionType::DIVIDE:
			case InstructionType::COMP_EQ:
			case InstructionType::COMP_NE:
			case InstructionType::COMP_LE:
			case InstructionType::COMP_GE:
			case InstructionType::COMP_LT:
			case InstructionType::COMP_GT:
				result = ExecuteInfixArithmetic(node);
				break;
				//arithmetic prefix
			case InstructionType::NEGATIVE:
			case InstructionType::POSITIVE:
				result = ExecutePrefixArithmetic(node);
				break;
				//assignment :)
			case InstructionType::ASSIGN:
				result = ExecuteAssignment(node);
				break;
				//object member operator
			case InstructionType::MEMBER:
				result = ExecuteMemberOperator(node);
				break;
				//function calls
			case InstructionType::CALL:
				result = ExecuteCall(node);
				break;
			case InstructionType::BLOCK:
				result = ExecuteBlock(node);
				break;
			case InstructionType::LOCAL: {
				auto state = SymbolTable.LocalMode;
				SymbolTable.LocalMode = true;
				result = ExecuteBlock(node);
				SymbolTable.LocalMode = state;
			}
				break;
			case InstructionType::LOOP0:
			case InstructionType::LOOP1:
			case InstructionType::LOOP2:
			case InstructionType::LOOP3:
				result = ExecuteLoop(node);
				break;
			case InstructionType::IF:
			case InstructionType::IFELSE:
				result = ExecuteConditional(node);
				break;
			case InstructionType::BREAK:
				result = std::make_shared<Node>(NodeType::Break);
				break;
			case InstructionType::CONTINUE:
				result = std::make_shared<Node>(NodeType::Continue);
				break;
			case InstructionType::RETURN0:
				result = std::make_shared<NodeReturn>(std::make_shared<Node>(NodeType::None));
				break;
			case InstructionType::RETURN1:
				result = std::make_shared<NodeReturn>(ExecuteBlock(node));
				break;
			case InstructionType::BLOCKBEGIN:
			case InstructionType::BLOCKEND:
			case InstructionType::CALLBEGIN:
			case InstructionType::CALLEND:
			case InstructionType::END_STATEMENT: throw ExecutorImplementationException("should have been processed");
			default: throw ExecutorImplementationException("unhandled case");
		}

		if (pushpop)
			SymbolTable.Pop();

		return result;
	}

	std::shared_ptr<Node> ExecutorImp::ExecuteStatement(std::shared_ptr<Node> node) {
		switch (node->GetNodeType()) {
			case NodeType::Command:
				//pass by simple reference, we're already holding the shared ptr at least 2 times
				return ExecuteCommand(reinterpret_cast<NodeCommand&>(*node));
			case NodeType::Atom: {
				auto id = reinterpret_cast<NodeAtom&>(*node);
				if (id.GetAtomType() == InstructionType::ID) {
					auto ptr = SymbolTable[id.AtomText];
					if (ptr != nullptr)
						return ptr;
					else
						throw ExecutorRuntimeException(id.AtomText + " is undefined");
				} else return node;
			}
			default:
				return node;
		}
	}

	inline std::shared_ptr<Node> ExecutorImp::Error(std::string message) {
		fprintf(stderr, "Runtime Error: %s.\n", message.c_str());
		return std::make_shared<Node>(NodeType::Error);
	}

	/* native function implementation */

	namespace native {

		static void view_primitive(Node& node, const char* sep = 0) {
			if (sep == 0) sep = " ";
			switch (node.GetNodeType()) {
				case NodeType::Integer: printf("%lld%s", reinterpret_cast<NodeInteger&>(node).Value, sep);
					break;
				case NodeType::Float: printf("%g%s", reinterpret_cast<NodeFloat&>(node).Value, sep);
					break;
				case NodeType::String: printf("%s%s", reinterpret_cast<NodeString&>(node).Value.c_str(), sep);
					break;
				case NodeType::DynamicArray: printf("array(%d)%s", reinterpret_cast<NodeArray&>(node).Vector.size(), sep);
					break;
				case NodeType::Bits: printf("binseq(%d)%s", reinterpret_cast<NodeArray&>(node).Vector.size(), sep);
					break;
				case NodeType::DynamicObject: printf("object%s", sep);
					break;
				case NodeType::Function: printf("function%s", sep);
					break;
				case NodeType::Bit: printf("%d%s", reinterpret_cast<NodeBit&>(node).Value, sep);
					break;
				default: printf("?%s", sep);
			}
		}

		static std::shared_ptr<Node> print(std::vector<std::shared_ptr<Node>>& node) {
			for (auto i = node.begin(); i != node.end(); i++) {
				view_primitive(**i,"");
			}
			return std::make_shared<Node>(NodeType::None);
		}

		static std::shared_ptr<Node> view(std::vector<std::shared_ptr<Node>>& node) {
			for (auto i = node.begin(); i != node.end(); i++) {
				switch ((*i)->GetNodeType()) {
					case NodeType::DynamicArray: {
						auto& n = reinterpret_cast<NodeArray&>(**i);
						auto size = n.Vector.size();
						printf("array(%d): (", size);
						for (unsigned i = 0; i < size; i++) {
							auto sep = i < size - 1 ? ", " : ") ";
							view_primitive(*n.Vector[i], sep);
						}
					}
						break;
					case NodeType::DynamicObject: {
						auto& n = reinterpret_cast<NodeObject&>(**i);
						printf("object: {");
						auto keys = n.GetAttributeKeys();
						for (auto& key : keys) {

							printf("%s: ", key.c_str());
							view_primitive(*n.GetAttributeValue(key), ", ");
						}
						printf("} ");
						break;
					}
					case NodeType::Bits: {
						auto& n = reinterpret_cast<NodeBits&>(**i);
						auto size = n.Value.size();
						auto i = size;
						char buffer[65];
						char bcount = 0;
						for (i = 0; i < size; i++) {
							if (n.Value[i]) buffer[bcount++] = '1'; else buffer[bcount++] = '0';
							if (bcount == 64) {
								buffer[bcount] = 0;
								printf("%s", buffer);
								bcount = 0;
							}
						}
						if (bcount > 0) {
							buffer[bcount] = 0;
							printf("%s", buffer);
							bcount = 0;
						}
						printf(" ");
					}
						break;
					default: view_primitive(**i, " ");
				}
			}
			printf("\n");
			return std::make_shared<Node>(NodeType::None);
		}

		static std::shared_ptr<Node> system(std::vector<std::shared_ptr<Node>>& node) {
			auto result = 0;
			for (auto i = node.begin(); i != node.end(); i++) {
				if ((*i)->GetNodeType() == NodeType::String) {
					auto result = std::system(reinterpret_cast<NodeString&>(*node[0]).Value.c_str());
					if (result != 0) return std::make_shared<NodeInteger>(result);
				} else throw Carbon::ExecutorRuntimeException("parameter of system must be a string");
			}
			return std::make_shared<NodeInteger>(result);
		}

		static std::shared_ptr<Node> parallel(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node)
		{
			//throw Carbon::ExecutorRuntimeException("unfortunately parallel is not fully implemented");
			// get hanle to thread pool
			auto& threadPool = *(GetThreadPool());
			// required parameter
			if (node.size()<1) throw Carbon::ExecutorRuntimeException("parallel needs at least one parameter");
			auto& functionParam = *(node[0]); 
			// optional parameters
			Node* argumentParam = nullptr;
			Node* degreeOfParallelismParam = nullptr;
			if (node.size() >= 2) argumentParam = &*node[1];
			if (node.size() >= 3) degreeOfParallelismParam = &*node[2];
			int degreeOfParallelism = 0;
			if (degreeOfParallelismParam!=nullptr && degreeOfParallelismParam->GetNodeType() == NodeType::Integer)
			{
				degreeOfParallelism = reinterpret_cast<NodeInteger*>(degreeOfParallelismParam)->Value;
			}

			// check if more parameters
			if (node.size()>3) throw Carbon::ExecutorRuntimeException("parallel does not accept more than 3 paramters");
			// execution vastly depends on the type of first paramter
			switch (functionParam.GetNodeType())
			{
			case NodeType::Function:
				if (argumentParam == nullptr)
				{
					NodeCommand cmd(InstructionType::CALL);
					cmd.Children.push_back(node[0]);
					return ex->ExecuteCall(cmd);
				}
				else if (argumentParam->GetNodeType() == NodeType::DynamicArray)
				{
					auto& vec = reinterpret_cast<NodeArray*>(argumentParam)->Vector;
					auto results = std::make_shared<NodeArray>(vec.size());
					auto& rvec = results->Vector;
					std::vector<std::function<void(void)>> tasks(vec.size());
					for (int i=0; i<vec.size(); i++)
					{
						tasks[i] = [i, &rvec, &node, &vec, ex]()
						{
							NodeCommand cmd(InstructionType::CALL);
							cmd.Children.push_back(node[0]);
							cmd.Children.push_back(vec[i]);
							rvec[i] = ex->ExecuteCall(cmd);
						};
					}
					auto task = threadPool.SubmitForExecution(tasks, degreeOfParallelism);
					task->WaitUntilDone();
					return results;
				}
				else if (argumentParam->GetNodeType() == NodeType::DynamicObject)
				{
					auto argument = reinterpret_cast<NodeObject*>(argumentParam);
					auto results = std::make_shared<NodeObject>();
					auto keys = argument->GetAttributeKeys();
					std::vector<std::function<void(void)>> tasks(keys.size());
					std::vector<std::pair<std::string, std::shared_ptr<Node>>> resultsVector(keys.size());
					int i = 0;
					for (auto key : keys)
					{
						resultsVector[i].first = key;
						auto param = argument->GetAttributeValue(key);
						tasks[i] = [i, &resultsVector, &node, param, ex]()
						{
							NodeCommand cmd(InstructionType::CALL);
							cmd.Children.push_back(node[0]);
							cmd.Children.push_back(param);
							resultsVector[i].second = ex->ExecuteCall(cmd);
						};
						i++;
					}
					auto task = threadPool.SubmitForExecution(tasks, degreeOfParallelism);
					task->WaitUntilDone();
					for (auto& element : resultsVector)
					{
						results->SetAttributeValue(element.first, element.second);
					}
					return results;
				}
				else
				{
					NodeCommand cmd(InstructionType::CALL);
					cmd.Children.push_back(node[0]);
					cmd.Children.push_back(node[1]);
					return ex->ExecuteCall(cmd);
				}
				break;
			case NodeType::DynamicArray:
				if (argumentParam == nullptr)
				{
					auto& fnVec = reinterpret_cast<NodeArray&>(functionParam).Vector;
					std::vector<std::function<void(void)>> tasks(fnVec.size());
					auto results = std::make_shared<NodeArray>(fnVec.size());
					auto& rvec = results->Vector;
					int i = 0;
					for (auto function:fnVec)
					{
						tasks[i] = [function, i, &rvec, &node, ex]()
						{
							NodeCommand cmd(InstructionType::CALL);
							cmd.Children.push_back(function);
							rvec[i] = ex->ExecuteCall(cmd);
						};
						i++;
					}
					auto task = threadPool.SubmitForExecution(tasks, degreeOfParallelism);
					task->WaitUntilDone();
					return results;
				}
				else
				{
					auto& fnVec = reinterpret_cast<NodeArray&>(functionParam).Vector;
					std::vector<std::function<void(void)>> tasks(fnVec.size());
					auto results = std::make_shared<NodeArray>(fnVec.size());
					auto& rvec = results->Vector;
					int i = 0;
					for (auto function : fnVec)
					{
						tasks[i] = [function, i, &rvec, &node, ex]()
						{
							NodeCommand cmd(InstructionType::CALL);
							cmd.Children.push_back(function);
							cmd.Children.push_back(node[1]);
							rvec[i] = ex->ExecuteCall(cmd);
						};
						i++;
					}
					auto task = threadPool.SubmitForExecution(tasks, degreeOfParallelism);
					task->WaitUntilDone();
					return results;
				}
				break;
			case NodeType::DynamicObject:
				if (argumentParam == nullptr)
				{
					auto argument = reinterpret_cast<NodeObject*>(argumentParam);
					auto argumentKeys = argument->GetAttributeKeys();
					std::vector<std::function<void(void)>> tasks(argumentKeys.size());
					auto results = std::make_shared<NodeObject>();
					std::vector<std::pair<std::string, std::shared_ptr<Node>>> resultsVector(argumentKeys.size());
					int i = 0;
					for (auto key : argumentKeys)
					{
						resultsVector[i].first = key;
						auto param = argument->GetAttributeValue(key);
						tasks[i] = [param, i, &resultsVector, &node, ex]()
						{
							NodeCommand cmd(InstructionType::CALL);
							cmd.Children.push_back(param);
							resultsVector[i].second = ex->ExecuteCall(cmd);
						};
						i++;
					}
					auto task = threadPool.SubmitForExecution(tasks, degreeOfParallelism);
					task->WaitUntilDone();
					for (auto& element : resultsVector)
					{
						results->SetAttributeValue(element.first, element.second);
					}
					return results;
				}
				else
				{
					auto argument = reinterpret_cast<NodeObject*>(argumentParam);
					auto argumentKeys = argument->GetAttributeKeys();
					std::vector<std::function<void(void)>> tasks(argumentKeys.size());
					auto results = std::make_shared<NodeObject>();
					std::vector<std::pair<std::string, std::shared_ptr<Node>>> resultsVector(argumentKeys.size());
					int i = 0;
					for (auto key : argumentKeys)
					{
						resultsVector[i].first = key;
						auto param = argument->GetAttributeValue(key);
						tasks[i] = [param, i, &resultsVector, &node, ex]()
						{
							NodeCommand cmd(InstructionType::CALL);
							cmd.Children.push_back(param);
							cmd.Children.push_back(node[1]);
							resultsVector[i].second = ex->ExecuteCall(cmd);
						};
						i++;
					}
					auto task = threadPool.SubmitForExecution(tasks, degreeOfParallelism);
					task->WaitUntilDone();
					for (auto& element : resultsVector)
					{
						results->SetAttributeValue(element.first, element.second);
					}
					return results;
				}
				break;
			default:
				throw Carbon::ExecutorRuntimeException("first parameter should be a function or container of functions");
				break;
			}
		}

		static std::shared_ptr<Node> clock(std::vector<std::shared_ptr<Node>>& node) {
			long long result = 0;
			auto timepoint = std::chrono::high_resolution_clock::now();
			auto epoch = timepoint.time_since_epoch();
			// check parameter
			if (node.size() >= 1) {
				if (node[0]->GetNodeType() == NodeType::String) {
					auto str = reinterpret_cast<NodeString&>(*node[0]).Value;
					if		(str == "ns" || str == "nanoseconds")	result = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
					else if (str == "us" || str == "microseconds")	result = std::chrono::duration_cast<std::chrono::microseconds>(epoch).count();
					else if (str == "ms" || str == "milliseconds")	result = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
					else if (str == "s" || str == "seconds")	result = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
					else if (str == "m" || str == "minutes")	result = std::chrono::duration_cast<std::chrono::minutes>(epoch).count();
					else if (str == "h" || str == "hours")	result = std::chrono::duration_cast<std::chrono::hours>(epoch).count();
					else throw Carbon::ExecutorRuntimeException("invalid time unit, accepted values are nanoseconds, ns, microseconds, us, milliseconds, ms, seconds, s, minutes, m, hours, h");
				}
			} 
			else
			{  
				// if no parameter return nanoseconds
				result = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
			}
			return std::make_shared<NodeInteger>(result);
		}

		static std::shared_ptr<Node> exit(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 1) {
				if (node[0]->GetNodeType() == NodeType::Integer) {
					std::exit((int)reinterpret_cast<NodeInteger&>(*node[0]).Value);
				} else throw Carbon::ExecutorRuntimeException("parameter of exit must be an integer");
			} else if (node.size() == 0) {
				std::exit(0);
			} else throw Carbon::ExecutorRuntimeException("exit requires 0 or 1 paramter");
		}

		static std::shared_ptr<Node> del(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			int released = 0;
			for (auto i = node.begin(); i != node.end(); i++) {
				if ((*i)->GetNodeType() == NodeType::String) {
					auto str = reinterpret_cast<NodeString&>(**i);
					auto& ptr = ex->SymbolTable[str.Value];
					if (ptr != nullptr) {
						ptr = nullptr;
						released++;
					}
				} else throw Carbon::ExecutorRuntimeException("delete requires strings as a parameters");
			}
			return std::make_shared<NodeInteger>(released);
		}

		static std::shared_ptr<Node> file_read(std::vector<std::shared_ptr<Node>>& node) {
			//read(16,32,"x.bin");
			if (node.size() <= 0 || node.size() > 3) throw Carbon::ExecutorRuntimeException("incorrect number of parameters at file read call");
			binseq::u64 read_offset = 0, read_size = -1;
			const char* fname = nullptr;
			if (node[0]->GetNodeType() == NodeType::Integer) {
				read_size = reinterpret_cast<NodeInteger&>(*node[0]).Value;
				if (node[1]->GetNodeType() == NodeType::Integer) {
					read_offset = reinterpret_cast<NodeInteger&>(*node[1]).Value;
					if (node[2]->GetNodeType() == NodeType::String) {
						fname = reinterpret_cast<NodeString&>(*node[2]).Value.c_str();
					} else throw Carbon::ExecutorRuntimeException("unexpected type, parameter 3 of file read");
				} else if (node[1]->GetNodeType() == NodeType::String) {
					fname = reinterpret_cast<NodeString&>(*node[1]).Value.c_str();
				} else throw Carbon::ExecutorRuntimeException("unexpected type, parameter 2 of file read");
			} else if (node[0]->GetNodeType() == NodeType::String) {
				fname = reinterpret_cast<NodeString&>(*node[0]).Value.c_str();
			} else throw Carbon::ExecutorRuntimeException("unexpected type, parameter 1 of file read");

			auto f = fopen(fname, "rb");
			if (f == nullptr) throw Carbon::ExecutorRuntimeException(std::string("failed to open file ") + fname);
			fseek(f, 0,SEEK_END);
			auto file_size = ftell(f) * 8;
			if (read_size > file_size - read_offset) read_size = file_size - read_offset;
			auto read_byte_offset = read_offset >> 3;
			auto read_misalign = read_offset - (read_byte_offset << 3);
			auto read_byte_size = (read_size + 7) >> 3;
			binseq::bit_sequence bits;
			bits.reallocate(read_byte_size << 3);
			fseek(f, read_byte_offset,SEEK_SET);
			fread(bits.address(), read_byte_size, 1, f);
			fclose(f);
			return std::make_shared<NodeBits>(binseq::subseq(bits, read_misalign, read_size));
		}

		static std::shared_ptr<Node> file_write(std::vector<std::shared_ptr<Node>>& node) {
			//write(b"0010",16,"x.bin");
			if (node.size() == 2) {
				if (node[0]->GetNodeType() != NodeType::Bits) throw Carbon::ExecutorRuntimeException("first parameter of file write must be a binseq");
				if (node[1]->GetNodeType() != NodeType::String) throw Carbon::ExecutorRuntimeException("second parameter of file write must be a string");
				auto& seq = reinterpret_cast<NodeBits&>(*node[0]).Value;
				auto fname = reinterpret_cast<NodeString&>(*node[1]).Value.c_str();
				auto f = fopen(fname, "wb");
				if (f == nullptr) throw Carbon::ExecutorRuntimeException(std::string("could not open ") + fname + " for writing");
				fwrite(seq.address(), (seq.size() + 7) >> 3, 1, f);
				fclose(f);
			} else if (node.size() == 3) {
				if (node[0]->GetNodeType() != NodeType::Bits) throw Carbon::ExecutorRuntimeException("first parameter of file write must be a binseq");
				if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("second parameter of file write must be an integer");
				if (node[2]->GetNodeType() != NodeType::String) throw Carbon::ExecutorRuntimeException("third parameter of file write must be a string");
				auto seq = &reinterpret_cast<NodeBits&>(*node[0]).Value;
				auto localseq = binseq::bit_sequence();
				auto write_offset = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				auto fname = reinterpret_cast<NodeString&>(*node[2]).Value.c_str();
				auto f = fopen(fname, "rwb");
				if (write_offset & 3) {
					fseek(f, write_offset >> 3,SEEK_SET);
					unsigned char c = 0;
					fread(&c, 1, 1, f);
					auto misalign = write_offset & 3;
					localseq = binseq::head(binseq::bit_sequence(c), misalign) + *seq;
					seq = &localseq;
				}
				auto write_size = seq->size();
				if (write_size & 3) {
					unsigned char c;
					fseek(f, (write_offset + write_size) >> 3,SEEK_SET);
					fread(&c, 1, 1, f);
					localseq = *seq + binseq::tail(binseq::bit_sequence(c), 8 - ((write_offset + write_size) & 3));
					write_size = localseq.size();
					seq = &localseq;
				}
				fseek(f, write_offset >> 3,SEEK_SET);
				fwrite(seq->address(), write_size, 1, f);
				fclose(f);
			}
			return std::make_shared<Node>(NodeType::None);
		}

		static std::shared_ptr<Node> cast_integer(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeInteger>(0);
			} else if (node.size() == 1) {
				long long ival = 0;
				switch (node[0]->GetNodeType()) {
					case NodeType::Integer: return node[0];
					case NodeType::Float: ival = (long long)reinterpret_cast<NodeFloat&>(*node[0]).Value;
						break;
					case NodeType::Bit: ival = (long long)reinterpret_cast<NodeBit&>(*node[0]).Value;
						break;
					case NodeType::String: ival = (long long)atol(reinterpret_cast<NodeString&>(*node[0]).Value.c_str());
						break;
					case NodeType::Bits: ival = *((long long*) reinterpret_cast<NodeBits&>(*node[0]).Value.address());
						break;
					default: throw Carbon::ExecutorRuntimeException("cannot convert parameter to integer");
				}
				return std::make_shared<NodeInteger>(ival);
			}
			throw Carbon::ExecutorRuntimeException("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_float(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeFloat>(.0);
			} else if (node.size() == 1) {
				double val = 0;
				switch (node[0]->GetNodeType()) {
					case NodeType::Float: return node[0];
					case NodeType::Integer: val = (double)reinterpret_cast<NodeInteger&>(*node[0]).Value;
						break;
					case NodeType::Bit: val = (double)reinterpret_cast<NodeBit&>(*node[0]).Value;
						break;
					case NodeType::String: val = (double)atof(reinterpret_cast<NodeString&>(*node[0]).Value.c_str());
					case NodeType::Bits: val = *((double*) reinterpret_cast<NodeBits&>(*node[0]).Value.address());
						break;
					default: throw Carbon::ExecutorRuntimeException("cannot convert parameter to float");
				}
				return std::make_shared<NodeFloat>(val);
			}
			throw Carbon::ExecutorRuntimeException("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_bit(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeFloat>(.0);
			} else if (node.size() == 1) {
				bool val = 0;
				switch (node[0]->GetNodeType()) {
					case NodeType::Bit: return node[0];
					case NodeType::Float: val = (bool) reinterpret_cast<NodeFloat&>(*node[0]).Value;
						break;
					case NodeType::Integer: val = (bool) reinterpret_cast<NodeInteger&>(*node[0]).Value;
						break;
					case NodeType::String: val = !reinterpret_cast<NodeString&>(*node[0]).Value.compare("0") == 0;
						break;
					case NodeType::Bits: val = ((bool) reinterpret_cast<NodeBits&>(*node[0]).Value[0]);
						break;
					default: throw Carbon::ExecutorRuntimeException("cannot convert parameter to bit");
				}
				return std::make_shared<NodeBit>(val);
			}
			throw Carbon::ExecutorRuntimeException("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_string(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeFloat>(.0);
			} else if (node.size() == 1) {
				char buffer[256];
				switch (node[0]->GetNodeType()) {
					case NodeType::Bit: sprintf_s(buffer, "%d", reinterpret_cast<NodeBit&>(*node[0]).Value);
						break;
					case NodeType::Float: sprintf_s(buffer, "%g", reinterpret_cast<NodeFloat&>(*node[0]).Value);
						break;
					case NodeType::Integer: sprintf_s(buffer, "%lld", reinterpret_cast<NodeInteger&>(*node[0]).Value);
						break;
					case NodeType::String: return node[0];
					case NodeType::Bits: {
						auto newnode = std::make_shared<NodeString>("");
						auto& bitsnode = reinterpret_cast<NodeBits&>(*node[0]);
						auto bytestream = (binseq::u8*) bitsnode.Value.address();
						auto count = (bitsnode.Value.size() + 7) >> 3;
						while (count--) {
							newnode->Value.push_back(*bytestream++);
						}
						return newnode;
					}
						break;
					default: throw Carbon::ExecutorRuntimeException("cannot convert parameter to float");
				}
				return std::make_shared<NodeString>(buffer);
			}
			throw Carbon::ExecutorRuntimeException("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_bits(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeBits>();
			} else if (node.size() == 1) {
				switch (node[0]->GetNodeType()) {
					case NodeType::Bit:
						return std::make_shared<NodeBits>(
							binseq::bit_sequence(reinterpret_cast<NodeBit&>(*node[0]).Value));
					case NodeType::Integer:
						return std::make_shared<NodeBits>(
							binseq::bit_sequence((binseq::u64)reinterpret_cast<NodeInteger&>(*node[0]).Value));
					case NodeType::Float:
						return std::make_shared<NodeBits>(
							binseq::bit_sequence(*((binseq::u64*)&reinterpret_cast<NodeFloat&>(*node[0]).Value)));
					case NodeType::Bits: return node[0];
					case NodeType::String:
						return std::make_shared<NodeBits>(
							binseq::bit_sequence(reinterpret_cast<NodeString&>(*node[0]).Value.c_str()));
						break;
					default: throw Carbon::ExecutorRuntimeException("cannot convert parameter to binseq");
				}
			}
			throw Carbon::ExecutorRuntimeException("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_array(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeArray>();
			} else if (node.size() == 1) {
				if (node[0]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("array can only receive integer as a parameter");
				auto& size = reinterpret_cast<NodeInteger&>(*node[0]).Value;
				auto newnode = std::make_shared<NodeArray>(size);
				auto nothing = std::make_shared<Node>(NodeType::None);
				auto& vec = newnode->Vector;
				for (auto i = vec.begin(); i != vec.end(); i++) {
					*i = nothing;
				}
				return newnode;
			} else {
				throw Carbon::ExecutorRuntimeException("array can't have more than 1 parameter");
			}
		}

		static std::shared_ptr<Node> cast_object(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeObject>();
			} else throw Carbon::ExecutorRuntimeException("object constructor accepts no parameters");
		}

		static std::shared_ptr<Node> type(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeString>("void");
			} else if (node.size() == 1) {
				const char* r = "unknown";
				switch (node[0]->GetNodeType()) {
					case NodeType::Atom:
						switch (node[0]->GetAtomType()) {
							case InstructionType::ID: r = "identifier";
								break;
							default: r = "unprocessed atom";
								break;
						}
						break;
					case NodeType::Bit: r = "bit";
						break;
					case NodeType::Bits: r = "binseq";
						break;
					case NodeType::Command: r = "command";
						break;
					case NodeType::Float: r = "float";
						break;
					case NodeType::Function: r = "function";
						break;
					case NodeType::Integer: r = "integer";
						break;
					case NodeType::Error: r = "error";
						break;
					case NodeType::None: r = "void";
						break;
					case NodeType::String: r = "string";
						break;
					case NodeType::DynamicArray: r = "array";
						break;
					case NodeType::DynamicObject: r = "object";
						break;
				}
				return std::make_shared<NodeString>(r);
			}
			throw Carbon::ExecutorRuntimeException("type function expects no more than one parameter");
		}


		static std::shared_ptr<Node> get(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 2) {
				switch (node[0]->GetNodeType()) {
					case NodeType::String: {
						if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("string can only be indexed by integer");
						auto& container = reinterpret_cast<NodeString&>(*node[0]).Value;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						if (idx < 0 || idx >= container.size()) throw Carbon::ExecutorRuntimeException("string index out of bounds");
						return std::make_shared<NodeInteger>((unsigned char)container[idx]);
					}
						break;

					case NodeType::DynamicArray: {
						if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("array can only be indexed by integer");
						auto& container = reinterpret_cast<NodeArray&>(*node[0]).Vector;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						if (idx < 0 || idx >= container.size()) throw Carbon::ExecutorRuntimeException("string index out of bounds");
						return container[idx];
					}
						break;

					case NodeType::Bits: {
						if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("string can only be indexed by integer");
						auto& container = reinterpret_cast<NodeBits&>(*node[0]).Value;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						if (idx < 0 || idx >= container.size()) throw Carbon::ExecutorRuntimeException("string index out of bounds");
						return std::make_shared<NodeBit>((bool)container[idx]);
						return node[2];
					}
						break;

					case NodeType::DynamicObject: {
						if (node[1]->GetNodeType() != NodeType::String) throw Carbon::ExecutorRuntimeException("object can only be indexed by string");
						auto& object = reinterpret_cast<NodeObject&>(*node[0]);
						auto& idx = reinterpret_cast<NodeString&>(*node[1]).Value;
						auto result = object.GetAttributeValue(idx);
						if (result != nullptr)
							return result;
						else
							return std::make_shared<Node>(NodeType::None);
					}
						break;

					default: throw Carbon::ExecutorRuntimeException("get requires the first parameter to be a container");
				}
			} else throw Carbon::ExecutorRuntimeException("get requires one container and one index");
		}


		static std::shared_ptr<Node> set(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 3) {
				switch (node[0]->GetNodeType()) {
					case NodeType::String: {
						if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("string can only be indexed by integer");
						if (node[2]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("value must be an integer");
						auto& container = reinterpret_cast<NodeString&>(*node[0]).Value;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						auto& val = reinterpret_cast<NodeInteger&>(*node[2]).Value;
						if (idx < 0 || idx >= container.size()) throw Carbon::ExecutorRuntimeException("index out of bounds when trying to set char code in string");
						container[idx] = val;
						return node[2];
					}
						break;

					case NodeType::DynamicArray: {
						if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("array can only be indexed by integer");
						auto& container = reinterpret_cast<NodeArray&>(*node[0]).Vector;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						auto& val = node[2];
						if (idx < 0 || idx >= container.size()) throw Carbon::ExecutorRuntimeException("index out of bounds when trying to set element in dynamic array");
						container[idx] = val;
						return node[2];
					}
						break;

					case NodeType::Bits: {
						if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("binary sequence can only be indexed by integer");
						if (node[2]->GetNodeType() != NodeType::Bit) throw Carbon::ExecutorRuntimeException("value must be a bit");
						auto& container = reinterpret_cast<NodeBits&>(*node[0]).Value;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						auto& val = reinterpret_cast<NodeBit&>(*node[2]).Value;
						if (idx < 0 || idx >= container.size()) throw Carbon::ExecutorRuntimeException("index out of bounds when trying to set bit in binary seruence");
						container[idx] = val;
						return node[2];
					}
						break;

					case NodeType::DynamicObject: {
						if (node[1]->GetNodeType() != NodeType::String) throw Carbon::ExecutorRuntimeException("object can only be indexed by string");
						auto& object = reinterpret_cast<NodeObject&>(*node[0]);
						auto& idx = reinterpret_cast<NodeString&>(*node[1]).Value;
						auto& val = node[2];
						object.SetAttributeValue(idx, val);
						return node[2];
					}
						break;

					default: throw Carbon::ExecutorRuntimeException("set requires the first parameter to be a container");
				}
			} else throw Carbon::ExecutorRuntimeException("set requires container, index and value");
		}

		static std::shared_ptr<Node> length(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 1) {
				long long val = 0;
				switch (node[0]->GetNodeType()) {
					case NodeType::String: val = reinterpret_cast<NodeString&>(*node[0]).Value.size();
						break;
					case NodeType::DynamicArray: val = reinterpret_cast<NodeArray&>(*node[0]).Vector.size();
						break;
					case NodeType::Bits: val = reinterpret_cast<NodeBits&>(*node[0]).Value.size();
						break;
					default: throw Carbon::ExecutorRuntimeException("parameter has no length, only array, string and binseq has length");
				}
				return std::make_shared<NodeInteger>(val);
			} else throw Carbon::ExecutorRuntimeException("length requires one parameter");
		}

		template <class T>
		static T vec_head(T& vec, long long count) {
			T result;
			for (auto i = vec.begin(); i != vec.end() && count > 0; i++ , count--) {
				result.push_back(*i);
			}
			return result;
		}

		template <class T>
		static T vec_tail(T& vec, long long count) {
			T result;
			for (auto i = vec.begin() + vec.size() - count; i != vec.end() && count > 0; i++ , count--) {
				result.push_back(*i);
			}
			return result;
		}

		template <class T>
		static T vec_subseq(T& vec, long long count, long long offset) {
			T result;
			for (auto i = vec.begin() + offset; i != vec.end() && count > 0; ++i , count--) {
				result.push_back(*i);
			}
			return result;
		}

		template <class T>
		static T vec_repeat(T& vec, long long count) {
			T result;
			auto i = vec.begin();
			while (count) {
				if (i == vec.end()) i = vec.begin();
				result.push_back(*i);
				++i;
				count--;
			}
			return result;
		}

		static std::shared_ptr<Node> sel_head(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 2) {
				if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("second parameter of head must be an integer");
				auto count = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				switch (node[0]->GetNodeType()) {
					case NodeType::Bits: return std::make_shared<NodeBits>(binseq::head(reinterpret_cast<NodeBits&>(*node[0]).Value, count));
					case NodeType::String: return std::make_shared<NodeString>(vec_head(reinterpret_cast<NodeString&>(*node[0]).Value, count));
					case NodeType::DynamicArray: return std::make_shared<NodeArray>(vec_head(reinterpret_cast<NodeArray&>(*node[0]).Vector, count));
					default: throw Carbon::ExecutorRuntimeException("head only works on sequences");
				}
			} else throw Carbon::ExecutorRuntimeException("head needs 2 parameters, a sequence and an integer");
		}

		static std::shared_ptr<Node> sel_tail(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 2) {
				if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("second parameter of tail must be an integer");
				auto count = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				switch (node[0]->GetNodeType()) {
					case NodeType::Bits: return std::make_shared<NodeBits>(binseq::tail(reinterpret_cast<NodeBits&>(*node[0]).Value, count));
					case NodeType::String: return std::make_shared<NodeString>(vec_tail(reinterpret_cast<NodeString&>(*node[0]).Value, count));
					case NodeType::DynamicArray: return std::make_shared<NodeArray>(vec_tail(reinterpret_cast<NodeArray&>(*node[0]).Vector, count));
					default: throw Carbon::ExecutorRuntimeException("tail only works on sequences");
				}
			} else throw Carbon::ExecutorRuntimeException("tail needs 2 parameters, a sequence and an integer");
		}

		static std::shared_ptr<Node> sel_subseq(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 3) {
				if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("second parameter of subseq must be an integer");
				if (node[2]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("third parameter of subseq must be an integer");
				auto offset = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				auto count = reinterpret_cast<NodeInteger&>(*node[2]).Value;
				switch (node[0]->GetNodeType()) {
					case NodeType::Bits: return std::make_shared<NodeBits>(binseq::subseq(reinterpret_cast<NodeBits&>(*node[0]).Value, offset, count));
					case NodeType::String: return std::make_shared<NodeString>(vec_subseq(reinterpret_cast<NodeString&>(*node[0]).Value, offset, count));
					case NodeType::DynamicArray: return std::make_shared<NodeArray>(vec_subseq(reinterpret_cast<NodeArray&>(*node[0]).Vector, offset, count));
					default: throw Carbon::ExecutorRuntimeException("subseq only works on sequences");
				}
			} else throw Carbon::ExecutorRuntimeException("subseq needs 3 parameters, a sequence and two integers");
		}

		static std::shared_ptr<Node> seq_repeat(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 2) {
				if (node[1]->GetNodeType() != NodeType::Integer) throw Carbon::ExecutorRuntimeException("second parameter of repeat must be an integer");
				auto count = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				switch (node[0]->GetNodeType()) {
					case NodeType::Bits: return std::make_shared<NodeBits>(binseq::repeat(reinterpret_cast<NodeBits&>(*node[0]).Value, count));
					case NodeType::String: return std::make_shared<NodeString>(vec_repeat(reinterpret_cast<NodeString&>(*node[0]).Value, count));
					case NodeType::DynamicArray: return std::make_shared<NodeArray>(vec_repeat(reinterpret_cast<NodeArray&>(*node[0]).Vector, count));
					default: throw Carbon::ExecutorRuntimeException("repeat only works on sequences");
				}
			} else throw Carbon::ExecutorRuntimeException("repeat needs 2 parameters, a sequence and an integer");
		}

		static std::shared_ptr<Node> popcount(std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 1) {
				if (node[0]->GetNodeType() != NodeType::Bits) throw Carbon::ExecutorRuntimeException("the parameter of popcount must be binseq");
				auto& seq = reinterpret_cast<NodeBits&>(*node[0]).Value;
				auto count = binseq::popcount(seq);
				return std::make_shared<NodeInteger>((long long)count);
			} else throw Carbon::ExecutorRuntimeException("popcount only accepts one parameter");
		}

		static std::shared_ptr<Node> verbose(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() > 1) {
				throw Carbon::ExecutorRuntimeException("verbose accepts only one or zero parameters of string which should contain the words tree, none, submit");
			}
			if (node.size() == 1) {
				if (node[0]->GetNodeType() == NodeType::String) {
					auto& node0 = reinterpret_cast<NodeString&>(*node[0]);
					auto& str = node0.Value;
					if (str.find("submit") != std::string::npos) ex->VERBOSE_SUBMIT = true;
					if (str.find("tree") != std::string::npos) ex->VERBOSE_TREE = true;
					if (str.find("performance") != std::string::npos) ex->VERBOSE_PERFORMANCE = true;
					if (str.find("none") != std::string::npos) ex->VERBOSE_SUBMIT = ex->VERBOSE_TREE = ex->VERBOSE_PERFORMANCE = false;
				} else throw Carbon::ExecutorRuntimeException("parameter is not a string");
			}
			std::string acc = "";
			if (ex->VERBOSE_SUBMIT) acc += "submit";
			if (ex->VERBOSE_TREE) {
				if (acc.size() > 0) {
					acc += " ";
				}
				acc += "tree";
			}
			return std::make_shared<NodeString>(acc);
		}
		
		static std::shared_ptr<Node> properties(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				auto keys = ex->SymbolTable.GlobalKeys();
				auto arr = std::make_shared<NodeArray>(keys.size());
				for (auto i = 0; i < keys.size(); i++) {
					arr->Vector[i] = std::make_shared<NodeString>(keys[i]);
				}
				return arr;
			} else if (node.size() == 1) {
				if (node[0]->GetNodeType() == NodeType::DynamicObject) {
					auto& obj = reinterpret_cast<NodeObject&>(*node[0]);
					auto arr = std::make_shared<NodeArray>();
					std::vector<std::string> propList;
					for (auto key : obj.GetAttributeKeys()) {
						arr->Vector.push_back(std::make_shared<NodeString>(key));
					}
					return arr;
				} else return ex->Error("first parameter of properties must be an object");

			} else return ex->Error("properties must be invoked with an object or no paramteres");
		}

		namespace op {


			static std::shared_ptr<Node> not(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 1) {
					if (node[0]->GetNodeType() == NodeType::Bit) {
						auto& v = reinterpret_cast<NodeBit&>(*node[0]);
						return std::make_shared<NodeBit>(!v.Value);
					} else if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& v = reinterpret_cast<NodeBits&>(*node[0]);
						return std::make_shared<NodeBits>(binseq::not(v.Value));
					} else throw Carbon::ExecutorRuntimeException("not operator only works on binseq or bit");
				} else throw Carbon::ExecutorRuntimeException("not operator requires 1 parameter");
			}

			static std::shared_ptr<Node> and(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bit) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>((l.Value && r.Value));
					} else if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::and(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise operator only works on binseq or bit");
				} else throw Carbon::ExecutorRuntimeException("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> or(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bit) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>((l.Value || r.Value));
					} else if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::or(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise operator only works on binseq or bit");
				} else throw Carbon::ExecutorRuntimeException("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> xor(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bit) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>(l.Value != r.Value);
					} else if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::xor(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise operator only works on binseq or bit");
				} else throw Carbon::ExecutorRuntimeException("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nand(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bit) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>(!(l.Value && r.Value));
					} else if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nand(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise operator only works on binseq or bit");
				} else throw Carbon::ExecutorRuntimeException("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nor(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bit) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>(!(l.Value || r.Value));
					} else if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nor(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise operator only works on binseq or bit");
				} else throw Carbon::ExecutorRuntimeException("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nxor(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bit) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>(l.Value == r.Value);
					} else if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nxor(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise operator only works on binseq or bit");
				} else throw Carbon::ExecutorRuntimeException("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> andc(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::andc(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise cut operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> orc(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::orc(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise cut operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> xorc(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::xorc(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise cut operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nandc(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nandc(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise cut operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> norc(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::norc(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise cut operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nxorc(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nxorc(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise cut operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> andr(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::andr(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> orr(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::orr(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> xorr(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::xorr(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nandr(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nandr(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> norr(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::norr(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nxorr(std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NodeType::Bits) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nxorr(l.Value, r.Value));
					} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator only works on binseq");
				} else throw Carbon::ExecutorRuntimeException("bitwise repeat operator requires 2 parameters");
			}

		};

	};

	/* native function resitration */

	ExecutorImp::ExecutorImp() {
		this->ControlLevel = 0;
		this->SymbolTable.Global("void") = std::make_shared<Node>(NodeType::None);
		this->SymbolTable.Global("error") = std::make_shared<Node>(NodeType::Error);
		//system commands
		RegisterNativeFunction("system", native::system, false);
		RegisterNativeFunction("exit", native::exit, false);
		RegisterInternalNativeFunction("delete", native::del, false);
		RegisterInternalNativeFunction("verbose", native::verbose, false);

		// paralellization
		RegisterInternalNativeFunction("parallel", native::parallel, false);
		

		//time
		RegisterNativeFunction("clock", native::clock, false);

		//printing
		RegisterNativeFunction("view", native::view, false);
		RegisterNativeFunction("print", native::print, false);

		//type information
		RegisterNativeFunction("type", native::type, true);

		//type casting / new instance
		RegisterNativeFunction("integer", native::cast_integer, true);
		RegisterNativeFunction("float", native::cast_float, true);
		RegisterNativeFunction("string", native::cast_string, true);
		RegisterNativeFunction("binseq", native::cast_bits, true);
		RegisterNativeFunction("bit", native::cast_bit, true);
		RegisterNativeFunction("array", native::cast_array, true);
		RegisterNativeFunction("object", native::cast_object, true);

		//container operations
		RegisterNativeFunction("get", native::get, true);
		RegisterNativeFunction("set", native::set, true);
		RegisterNativeFunction("length", native::length, true);
		RegisterInternalNativeFunction("properties", native::properties, false);

		//selectors
		RegisterNativeFunction("head", native::sel_head, true);
		RegisterNativeFunction("tail", native::sel_tail, true);
		RegisterNativeFunction("subseq", native::sel_subseq, true);

		//sequence operators
		RegisterNativeFunction("repeat", native::seq_repeat, true);

		//io
		RegisterNativeFunction("read", native::file_read, false);
		RegisterNativeFunction("write", native::file_write, false);

		//bits operators                                       
		RegisterNativeFunction("not", native::op::not, true);
		RegisterNativeFunction("and", native::op::and, true);
		RegisterNativeFunction("or", native::op::or, true);
		RegisterNativeFunction("xor", native::op::xor, true);
		RegisterNativeFunction("nand", native::op::nand, true);
		RegisterNativeFunction("nor", native::op::nor, true);
		RegisterNativeFunction("nxor", native::op::nxor, true);

		RegisterNativeFunction("andc", native::op::andc, true);
		RegisterNativeFunction("orc", native::op::orc, true);
		RegisterNativeFunction("xorc", native::op::xorc, true);
		RegisterNativeFunction("nandc", native::op::nandc, true);
		RegisterNativeFunction("norc", native::op::norc, true);
		RegisterNativeFunction("nxorc", native::op::nxorc, true);

		RegisterNativeFunction("andr", native::op::andr, true);
		RegisterNativeFunction("orr", native::op::orr, true);
		RegisterNativeFunction("xorr", native::op::xorr, true);
		RegisterNativeFunction("nandr", native::op::nandr, true);
		RegisterNativeFunction("norr", native::op::norr, true);
		RegisterNativeFunction("nxorr", native::op::nxorr, true);

		//testing

		RegisterNativeFunction("popcount", native::popcount, true);

	}

	/* function call dispatch */

	inline std::shared_ptr<Node> ExecutorImp::ExecuteCall(NodeCommand& node) {
		// function node
		NodeFunction* function = nullptr;
		auto fnnodeptr = node.Children[0];
		if (fnnodeptr->GetNodeType() == NodeType::Atom)
		{
			// we ereceived function name, perform lookup
			auto fname = reinterpret_cast<NodeAtom*>(&*node.Children[0])->AtomText;
			std::shared_ptr<Node> nodeptr = SymbolTable[fname];
			if (nodeptr != nullptr && nodeptr->GetNodeType() == NodeType::Function) {
				function = reinterpret_cast<NodeFunction*>(&*nodeptr);
			}
			else throw ExecutorRuntimeException(fname + " is not a function");
		}
		else if(fnnodeptr->GetNodeType() == NodeType::Function)
		{
			// we received function directly
			function = reinterpret_cast<NodeFunction*>(&*fnnodeptr);
		}
		if (function != nullptr) {
			std::vector<std::shared_ptr<Node>> paramlist;
			for (auto i = ++node.Children.begin(); i != node.Children.end(); ++i) {
				paramlist.push_back(ExecuteStatement(*i));
			}
			if (function->Native) {
				if (function->InternalNative)
				{
					return reinterpret_cast<internal_native_function_ptr>(function->nativeptr)(this, paramlist);
				}
				else
				{
					return function->nativeptr(paramlist);
				}
			} else {
				SymbolTable.Push();
				unsigned functionParameterCount = function->ParameterList.size();
				unsigned callParamCount = paramlist.size();
				unsigned parametersToPass = functionParameterCount > callParamCount ? callParamCount : functionParameterCount;
				for (unsigned i = 0; i < parametersToPass; i++)
					SymbolTable.Local(function->ParameterList[i]) = paramlist[i];
				for (unsigned i = parametersToPass; i < functionParameterCount; i++)
					SymbolTable.Local(function->ParameterList[i]) = std::make_shared<Node>(NodeType::None);
				auto result = ExecuteStatement(function->Implementation);
				SymbolTable.Pop();
				while (result->GetNodeType() == NodeType::Return) {
					result = reinterpret_cast<NodeReturn&>(*result).Value;
				}
				return result;
			}
		}
		else throw ExecutorRuntimeException("parameter is not a function");
	}

}
