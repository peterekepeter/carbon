#include "exec.hpp"

#include <cstdio>
#include <string>
#include <stack>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>

#include "../binseq_lib/binseq.hpp"

#pragma warning(disable:4482)

#define override virtual

//#define VERBOSE_SUBMIT
//#define VERBOSE_TREE

namespace exec {
	enum NODETYPE {
		//`
		NODE_NONE,
		NODE_ERROR,
		NODE_CONTINUE,
		NODE_BREAK,
		NODE_RETURN,
		NODE_ATOM,
		NODE_COMMAND,
		//objects
		NODE_INTEGER,
		NODE_FLOAT,
		NODE_STRING,
		NODE_BIT,
		NODE_BITS,
		NODE_FUNCTION,
		NODE_ARRAY,
		NODE_OBJECT,
	};

	const char* GetTypeText(NODETYPE nodetype);

	class Node {
	public:
		inline bool IsAtom() {
			return NodeType == NODETYPE::NODE_ATOM;
		}

		inline bool IsCommand() {
			return NodeType == NODETYPE::NODE_COMMAND;
		}

		inline NODETYPE GetNodeType() {
			return NodeType;
		}

		virtual const char* GetText(); //get a description to help debug
		virtual InstructionType GetCommandType();
		virtual InstructionType GetAtomType();
		virtual const char* GetAtomText(); //get parsed text    
		Node(const NODETYPE);
	protected:
		NODETYPE NodeType;
	};

	class NodeAtom : public Node {
	public:
		InstructionType AtomType;
		std::string AtomText;
		NodeAtom(const InstructionType type, const char* text);
		override const char* GetText();
		override InstructionType GetAtomType();
		override const char* GetAtomText(); //get parsed text
	};

	class NodeString : public Node {
	public:
		std::string Value;
		override const char* GetText();
		NodeString(const std::string str);
	};

	typedef std::shared_ptr<Node> (*native_function_ptr)(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node);

	class NodeFunction : public Node {
	public:
		bool Native;
		bool Pure;
		NodeFunction(native_function_ptr nativeptr, bool pure);
		NodeFunction(std::vector<std::string> parameterList, std::shared_ptr<Node> impl);
		native_function_ptr nativeptr;
		std::vector<std::string> ParameterList;
		std::shared_ptr<Node> Implementation;
		override const char* GetText();
	};

	class NodeInteger : public Node {
	public:
		long long Value;
		override const char* GetText();
		NodeInteger(long long value);
		std::shared_ptr<std::string> text;
	};

	class NodeBits : public Node {
	public:
		binseq::bit_sequence Value;
		override const char* GetText();
		NodeBits();
		NodeBits(const binseq::bit_sequence& b);
		NodeBits(binseq::bit_sequence&& b);
	};

	const char* NodeBits::GetText() {
		return "binseq";
	}

	NodeBits::NodeBits():Node(NODETYPE::NODE_BITS) {}

	NodeBits::NodeBits(const binseq::bit_sequence& b):Node(NODETYPE::NODE_BITS) {
		Value = b;
	}

	NodeBits::NodeBits(binseq::bit_sequence&& b):Node(NODETYPE::NODE_BITS) {
		Value = b;
	}

	class NodeArray : public Node {
	public:
		std::vector<std::shared_ptr<Node>> Vector;
		override const char* GetText();
		NodeArray();
		NodeArray(int size);
		NodeArray(const std::vector<std::shared_ptr<Node>>& vec);
		NodeArray(std::vector<std::shared_ptr<Node>>&& vec);
	};

	const char* NodeArray::GetText() {
		return "array";
	}

	NodeArray::NodeArray(int size):Node(NODETYPE::NODE_ARRAY) {
		Vector.resize(size);
	}

	NodeArray::NodeArray():Node(NODETYPE::NODE_ARRAY) { }

	NodeArray::NodeArray(const std::vector<std::shared_ptr<Node>>& vec):Node(NODETYPE::NODE_ARRAY) {
		Vector = vec;
	};

	NodeArray::NodeArray(std::vector<std::shared_ptr<Node>>&& vec):Node(NODETYPE::NODE_ARRAY) {
		Vector = vec;
	};

	class NodeObject : public Node {
	public:
		std::unordered_map<std::string, std::shared_ptr<Node>> Map;
		override const char* GetText();
		NodeObject();
	};

	const char* NodeObject::GetText() {
		return "object";
	}

	NodeObject::NodeObject():Node(NODETYPE::NODE_OBJECT) {};

	class NodeBit : public Node {
	public:
		bool Value;
		override const char* GetText();
		NodeBit(bool value);
	};

	class NodeFloat : public Node {
	public:
		double Value;
		override const char* GetText();
		NodeFloat(double value);
		std::shared_ptr<std::string> text;
	};

	class NodeCommand : public Node {
	public:
		InstructionType CommandType;
		std::vector<std::shared_ptr<Node>> Children;
		NodeCommand(const InstructionType);
		override const char* GetText();
		override InstructionType GetCommandType();
		bool DoesPushStack;
		bool IsPure;
	};

	class NodeReturn : public Node {
	public:
		std::shared_ptr<Node> Value;
		NodeReturn(std::shared_ptr<Node> return_value);
		override const char* GetText();
	};

	NodeReturn::NodeReturn(std::shared_ptr<Node> value) : Value(value), Node(NODETYPE::NODE_RETURN) {}

	const char* NodeReturn::GetText() {
		return "return";
	}

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
		bool ShowPrompt;
		SymbolTableStack SymbolTable;
		std::stack<std::shared_ptr<Node>> stack;
		std::vector<std::shared_ptr<Node>> StatementList;
		std::shared_ptr<Node> ReplaceIdIfPossible(std::shared_ptr<Node>);
		std::shared_ptr<Node> OptimizeIfPossible(std::shared_ptr<Node>);
		void RegisterNativeFunction(const char* name, native_function_ptr, bool pure);
		ExecutorImp::ExecutorImp();
		std::shared_ptr<Node> ExecuteStatement(std::shared_ptr<Node>);
		void ExecuteStatementList();
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

	// generic execution context exception, this should be the base for all execution exceptions
	class Exception {
	public:
		const char* Message;
		Exception(const char* message) : Message(message) {};
	};

	// exception due to incomplete implementation of the executor
	class ImplementationException : public Exception {
	public:
		ImplementationException(const char* message) : Exception(message) {};
	};

	// exception occured during the runtime of the interpreted language
	class RuntimeException : public Exception	{
	public:
		RuntimeException(const char* message) : Exception(message) {};
	};

	Executor::Executor() {
		this->imp = new ExecutorImp;
		this->SetInteractiveMode(false);
		this->imp->VERBOSE_SUBMIT = false;
		this->imp->VERBOSE_TREE = false;
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
						} else throw ImplementationException("invalid");
					} else throw ImplementationException("invalid");
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
						auto& map = objectNode->Map;
						for (int i=collector.size(); i>0; i-=2)
						{
							auto& key = reinterpret_cast<NodeAtom&>(*collector[i - 1]);
							map[key.AtomText] = collector[i - 2];
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
			}
				//passthrough
			case InstructionType::END_STATEMENT:
			end_statement:
				if (imp->SymbolTable.GetLevel() == 1 && imp->ControlLevel == 0) {
					if (imp->VERBOSE_TREE) {
						recursive_print(*(imp->stack.top()));
					}
					//save the statement
					this->imp->StatementList.push_back(imp->stack.top());
					//clear the stack
					while (!imp->stack.empty()) imp->stack.pop();
					if (imp->ShowPrompt) {
						//if console mode, execute
						imp->ExecuteStatementList();
						imp->ClearStatementList();
						printf(">");
					}
				}

				break;
			default: throw ImplementationException("Unhandled command.");
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

	NodeAtom::NodeAtom(const InstructionType type, const char* text) : Node(NODETYPE::NODE_ATOM) {
		this->AtomType = type;
		this->AtomText = text;
	}

	Node::Node(const NODETYPE ntype) {
		this->NodeType = ntype;
	}

	NodeCommand::NodeCommand(const InstructionType cmd) : Node(NODETYPE::NODE_COMMAND) {
		this->DoesPushStack = false;
		this->IsPure = true;
		this->CommandType = cmd;
	}

	const char* NodeAtom::GetText() {
		return this->AtomText.c_str();
	}

	const char* NodeCommand::GetText() {
		switch (this->CommandType) {
			case InstructionType::END_STATEMENT: return "ENDST";
			case InstructionType::ASSIGN: return "ASSIGN";
			case InstructionType::CALL: return "CALL";
			case InstructionType::CALLBEGIN: return "CALLBEGIN";
			case InstructionType::CALLEND: return "CALLEND";
			case InstructionType::ADD: return "ADD";
			case InstructionType::SUBTRACT: return "SUB";
			case InstructionType::MULTIPLY: return "MUL";
			case InstructionType::DIVIDE: return "DIV";
			case InstructionType::POSITIVE: return "POS";
			case InstructionType::NEGATIVE: return "NEG";
			case InstructionType::COMP_EQ: return "COMP_EQ";
			case InstructionType::COMP_NE: return "COMP_NE";
			case InstructionType::COMP_GE: return "COMP_GE";
			case InstructionType::COMP_LE: return "COMP_LE";
			case InstructionType::COMP_GT: return "COMP_GT";
			case InstructionType::COMP_LT: return "COMP_LT";
			case InstructionType::BLOCK: return "BLOCK";
			case InstructionType::BLOCKBEGIN: return "BLOCKBEGIN";
			case InstructionType::BLOCKEND: return "BLOCKEND";
			case InstructionType::FUNCTIONBEGIN: return "FUNCTIONBEGIN";
			case InstructionType::FUNCTIONEND: return "FUNCTIONEND";
			case InstructionType::LOCAL: return "LOCAL";
			case InstructionType::CONTROL: return "CONTROL";
			case InstructionType::IF: return "IF";
			case InstructionType::IFELSE: return "IFELSE";
			case InstructionType::LOOP0: return "LOOP0";
			case InstructionType::LOOP1: return "LOOP1";
			case InstructionType::LOOP2: return "LOOP2";
			case InstructionType::LOOP3: return "LOOP3";
			case InstructionType::MEMBER: return "MEMBER";
			default: throw ImplementationException("Unhandled commandtype.");
		}
	}

	const char* GetTypeText(NODETYPE nodetype) {
		switch (nodetype) {
			case NODETYPE::NODE_ATOM: return"identifier";
			case NODETYPE::NODE_BIT: return "bit";
			case NODETYPE::NODE_BITS: return "bitseq";
			case NODETYPE::NODE_COMMAND: return "command";
			case NODETYPE::NODE_FLOAT: return "float";
			case NODETYPE::NODE_FUNCTION: return "function";
			case NODETYPE::NODE_INTEGER: return "integer";
			case NODETYPE::NODE_ERROR: return "error";
			case NODETYPE::NODE_NONE: return "void";
			case NODETYPE::NODE_STRING: return "string";
			default: throw ImplementationException("Unhandled nodetype.");
		}
	}


	const char* Node::GetText() {
		throw ImplementationException("Unspecified node type.");
	}

	InstructionType Node::GetCommandType() {
		throw ImplementationException("Unspecified node type.");
	}

	InstructionType Node::GetAtomType() {
		throw ImplementationException("Unspecified node type.");
	}

	const char* Node::GetAtomText() {
		throw ImplementationException("Unspecified node type.");
	}

	InstructionType NodeAtom::GetAtomType() {
		return this->AtomType;
	}

	const char* NodeAtom::GetAtomText() {
		return this->AtomText.c_str();
	}

	InstructionType NodeCommand::GetCommandType() {
		return this->CommandType;
	}

	std::shared_ptr<Node> ExecutorImp::OptimizeIfPossible(std::shared_ptr<Node> node) {
		if (node->IsCommand()) {
			auto cmd = reinterpret_cast<NodeCommand*>(&*node);
			if (cmd->Children.size() == 1) {
				auto cmdtype = cmd->GetCommandType();
				if (cmdtype == InstructionType::POSITIVE) {
					if (cmd->Children[0]->GetNodeType() == NODETYPE::NODE_INTEGER ||
						cmd->Children[0]->GetNodeType() == NODETYPE::NODE_FLOAT)
						return cmd->Children[0];
				} else if (cmdtype == InstructionType::NEGATIVE) {
					if (cmd->Children[0]->GetNodeType() == NODETYPE::NODE_INTEGER)
						return std::make_shared<NodeInteger>(- reinterpret_cast<NodeInteger*>(&*cmd->Children[0])->Value);
					if (cmd->Children[0]->GetNodeType() == NODETYPE::NODE_FLOAT)
						return std::make_shared<NodeFloat>(- reinterpret_cast<NodeFloat*>(&*cmd->Children[0])->Value);
				}

			} else if (cmd->Children.size() == 2) {
				auto cmdtype = cmd->GetCommandType();

				//try reduce arithmetic nodes
				if (cmdtype == InstructionType::ADD || cmdtype == InstructionType::SUBTRACT ||
					cmdtype == InstructionType::MULTIPLY || cmdtype == InstructionType::DIVIDE) {
					//reduce integer arithmetic
					if (cmd->Children[0]->GetNodeType() == NODETYPE::NODE_INTEGER &&
						cmd->Children[1]->GetNodeType() == NODETYPE::NODE_INTEGER) {
						auto a = reinterpret_cast<NodeInteger*>(&*cmd->Children[0]);
						auto b = reinterpret_cast<NodeInteger*>(&*cmd->Children[1]);
						switch (cmdtype) {
							case InstructionType::ADD: return std::make_shared<NodeInteger>(a->Value + b->Value);
							case InstructionType::SUBTRACT: return std::make_shared<NodeInteger>(a->Value - b->Value);
							case InstructionType::MULTIPLY: return std::make_shared<NodeInteger>(a->Value * b->Value);
							case InstructionType::DIVIDE:
								if (b->Value == 0) {
									throw RuntimeException("Division by zero.");
									return node;
								};
								return std::make_shared<NodeInteger>(a->Value / b->Value);
						}
					}

					//reduce float arithmetic
					if (cmd->Children[0]->GetNodeType() == NODETYPE::NODE_FLOAT &&
						cmd->Children[1]->GetNodeType() == NODETYPE::NODE_FLOAT) {
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


	const char* NodeInteger::GetText() {
		if (text == nullptr) {
			char buffer[256];
			sprintf_s<sizeof(buffer)>(buffer, "%lld", this->Value);
			text = std::make_shared<std::string>(buffer);
		}
		return text->c_str();
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

	NodeString::NodeString(const std::string str) : Node(NODETYPE::NODE_STRING), Value(str) { }

	bool Executor::GetInteractiveMode() {
		return this->imp->ShowPrompt;
	}

	void Executor::SetInteractiveMode(bool interactive) {
		this->imp->ShowPrompt = interactive;
		if (interactive)
			printf(">");
	}

	void Executor::ClearStatement() {
		while (!this->imp->stack.empty())
			this->imp->stack.pop();
	}

	void Executor::Execute() {
		imp->ExecuteStatementList();
		imp->ClearStatementList();
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
		: Node(NODETYPE::NODE_FUNCTION), Native(true), nativeptr(fptr), Pure(pure) { }

	NodeFunction::NodeFunction(std::vector<std::string> parameterList, std::shared_ptr<Node> impl)
		:Node(NODETYPE::NODE_FUNCTION), Native(false), nativeptr(nullptr), ParameterList(parameterList), Implementation(impl) {}

	const char* NodeFunction::GetText() {
		if (Native) return "native function";
		else return "function";
	}

	NodeBit::NodeBit(bool value) : Value(value), Node(NODETYPE::NODE_BIT) {};

	const char* NodeBit::GetText() {
		return Value ? "1" : "0";
	};

	NodeInteger::NodeInteger(long long value) : Node(NODETYPE::NODE_INTEGER), Value(value), text(nullptr) { }

	NodeFloat::NodeFloat(double value) : Node(NODETYPE::NODE_FLOAT), Value(value), text(nullptr) { }

	void ExecutorImp::RegisterNativeFunction(const char* name, native_function_ptr fptr, bool pure) {
		auto newfunction = std::make_shared<NodeFunction>(fptr, pure);
		auto& symbol = SymbolTable.Global(name);
		symbol = newfunction;
	}

	void ExecutorImp::ClearStatementList() {
		this->StatementList.clear();
	}

	void ExecutorImp::ExecuteStatementList() {
		for (auto i = StatementList.begin(); i != StatementList.end(); i++) {
			auto node = ExecuteStatement(*i);
			if (node->GetNodeType() == NODETYPE::NODE_ERROR) {
				if (!ShowPrompt) {
					fprintf(stderr, "Execution halted to prevent unknown sidefects.\n");
				} else { }
			}
		}
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecutePrefixArithmetic(NodeCommand& node) {
		if (node.Children.size() == 1) {
			auto executed = ExecuteStatement(node.Children[0]);
			switch (executed->GetNodeType()) {
				case NODETYPE::NODE_INTEGER: {
					auto& integer = reinterpret_cast<NodeInteger&>(*executed);
					switch (node.CommandType) {
						case InstructionType::POSITIVE: return executed;
						case InstructionType::NEGATIVE: return std::make_shared<NodeInteger>(-integer.Value);
						default: throw ImplementationException("unexpected commandtype");
					}
				}
				case NODETYPE::NODE_FLOAT: {
					auto& fval = reinterpret_cast<NodeFloat&>(*executed);
					switch (node.CommandType) {
						case InstructionType::POSITIVE: return executed;
						case InstructionType::NEGATIVE: std::make_shared<NodeFloat>(-fval.Value);
						default: throw ImplementationException("unexpected commandtype");
					}
				}
				case NODETYPE::NODE_BIT: {
					auto& bval = reinterpret_cast<NodeBit&>(*executed);
					switch (node.CommandType) {
						case InstructionType::POSITIVE: return executed;
						case InstructionType::NEGATIVE: return std::make_shared<NodeBit>(!bval.Value);
					}
				}
				default:
					return Error("prefix operators + or - only work on numeric values");
			}
		} else throw ImplementationException("Prefix op can only have 1 child.");
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteInfixArithmetic(NodeCommand& node) {
		if (node.Children.size() >= 2) {
			std::vector<std::shared_ptr<Node>> executed;
			for (auto i = node.Children.begin(); i != node.Children.end(); i++) {
				executed.push_back(ExecuteStatement(*i));
			}
		reexecute:
			switch (executed[0]->GetNodeType()) {
				case NODETYPE::NODE_INTEGER: {
					if (node.CommandType == InstructionType::MULTIPLY &&
						executed[1]->GetNodeType() == NODETYPE::NODE_STRING ||
						executed[1]->GetNodeType() == NODETYPE::NODE_BITS ||
						executed[1]->GetNodeType() == NODETYPE::NODE_ARRAY) {
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
							if ((*i)->GetNodeType() != NODETYPE::NODE_INTEGER) return Error("unexpected type in an arithmetic integer expression");
							auto val = reinterpret_cast<NodeInteger*>(&**i)->Value;
							switch (node.CommandType) {
								case InstructionType::ADD: acc->Value += val;
									break;
								case InstructionType::SUBTRACT: acc->Value -= val;
									break;
								case InstructionType::MULTIPLY: acc->Value *= val;
									break;
								case InstructionType::DIVIDE: if (val == 0) return Error("integer division by 0"); else acc->Value /= val;
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
					} else return Error("error in expression");

				}
				case NODETYPE::NODE_FLOAT: {

					if (node.CommandType == InstructionType::ADD ||
						node.CommandType == InstructionType::SUBTRACT ||
						node.CommandType == InstructionType::MULTIPLY ||
						node.CommandType == InstructionType::DIVIDE) {
						auto i = executed.begin();
						auto acc = std::make_shared<NodeFloat>(reinterpret_cast<NodeFloat*>(&**i)->Value);
						for (i++; i != executed.end(); i++) {
							if ((*i)->GetNodeType() != NODETYPE::NODE_FLOAT) return Error("unexpected type in an arithmetic float expression");
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
				case NODETYPE::NODE_STRING: {
					//if (executed[1]->GetNodeType()!=NODETYPE::NODE_STRING) return Error("non string value in string expression");
					auto& left = reinterpret_cast<NodeString&>(*executed[0]);
					NodeString* right = nullptr;

					if (node.CommandType != InstructionType::MULTIPLY) {
						if (executed[1]->GetNodeType() != NODETYPE::NODE_STRING) {
							return Error("non string value in string expression");
						} else {
							right = reinterpret_cast<NodeString*>(&*executed[1]);
						}
					}

					switch (node.CommandType) {
						case InstructionType::ADD: {
							std::string acc = left.Value;
							for (unsigned i = 1; i < executed.size(); i++) {
								if (executed[i]->GetNodeType() == NODETYPE::NODE_STRING) {
									acc += reinterpret_cast<NodeString&>(*executed[i]).Value;
								} else return Error("non string value in string concatenation");
							}
							return std::make_shared<NodeString>(acc);
						}
						case InstructionType::MULTIPLY:
							if (executed.size() == 2) {
								if (executed[1]->GetNodeType() == NODETYPE::NODE_INTEGER) {
									int ival = reinterpret_cast<NodeInteger&>(*executed[1]).Value;
									std::string result;
									result.reserve(left.Value.size() * ival);
									while (ival--) result += left.Value;
									return std::make_shared<NodeString>(result);
								} else return Error("multiplication is not defined between the given arguments (try string * int)");
							} else return Error("multiplication when left side is string is only valid with an integer");
							break;
						case InstructionType::COMP_EQ: return std::make_shared<NodeBit>(left.Value == right->Value);
						case InstructionType::COMP_NE: return std::make_shared<NodeBit>(left.Value != right->Value);
						case InstructionType::COMP_GE: return std::make_shared<NodeBit>(left.Value >= right->Value);
						case InstructionType::COMP_LE: return std::make_shared<NodeBit>(left.Value <= right->Value);
						case InstructionType::COMP_GT: return std::make_shared<NodeBit>(left.Value > right->Value);
						case InstructionType::COMP_LT: return std::make_shared<NodeBit>(left.Value < right->Value);
						default: return Error("string doesn't support the requested command");
					}
				}
				case NODETYPE::NODE_BITS: {
					//if (executed[1]->GetNodeType()!=NODETYPE::NODE_STRING) return Error("non string value in string expression");
					auto& left = reinterpret_cast<NodeBits&>(*executed[0]);
					NodeBits* right = nullptr;

					if (node.CommandType != InstructionType::MULTIPLY) {
						if (executed[1]->GetNodeType() != NODETYPE::NODE_BITS) {
							return Error("non binseq value in string expression");
						} else {
							right = reinterpret_cast<NodeBits*>(&*executed[1]);
						}
					}

					switch (node.CommandType) {
						case InstructionType::ADD: {
							binseq::bit_sequence acc = left.Value;
							for (unsigned i = 1; i < executed.size(); i++) {
								if (executed[i]->GetNodeType() == NODETYPE::NODE_BITS) {
									acc = acc + reinterpret_cast<NodeBits&>(*executed[i]).Value;
								} else return Error("non binseq value in binseq concatenation");
							}
							return std::make_shared<NodeBits>(std::move(acc));
						}
						case InstructionType::MULTIPLY:
							if (executed.size() == 2) {
								if (executed[1]->GetNodeType() == NODETYPE::NODE_INTEGER) {
									int ival = reinterpret_cast<NodeInteger&>(*executed[1]).Value;
									binseq::bit_sequence result;
									while (ival--) result = result + left.Value;
									return std::make_shared<NodeBits>(std::move(result));
								} else return Error("multiplication is not defined between the given arguments (try binseq * int)");
							} else return Error("multiplication when left side is binseq is only valid with an integer");
							break;
						case InstructionType::COMP_EQ: return std::make_shared<NodeBit>(left.Value == right->Value);
						case InstructionType::COMP_NE: return std::make_shared<NodeBit>(left.Value != right->Value);
						case InstructionType::COMP_GE: return std::make_shared<NodeBit>(left.Value >= right->Value);
						case InstructionType::COMP_LE: return std::make_shared<NodeBit>(left.Value <= right->Value);
						case InstructionType::COMP_GT: return std::make_shared<NodeBit>(left.Value > right->Value);
						case InstructionType::COMP_LT: return std::make_shared<NodeBit>(left.Value < right->Value);
						default: return Error("binseq doesn't support the requested command");
					}
				}
				case NODETYPE::NODE_ARRAY: {
					auto& left = reinterpret_cast<NodeArray&>(*executed[0]);
					NodeArray* right = nullptr;

					if (node.CommandType != InstructionType::MULTIPLY) {
						if (executed[1]->GetNodeType() != NODETYPE::NODE_ARRAY) {
							return Error("non array value in string expression");
						} else {
							right = reinterpret_cast<NodeArray*>(&*executed[1]);
						}
					}

					switch (node.CommandType) {
						case InstructionType::ADD: {
							auto acc = std::make_shared<NodeArray>();
							for (unsigned i = 1; i < executed.size(); i++) {
								acc->Vector = left.Vector; //copy
								if (executed[i]->GetNodeType() == NODETYPE::NODE_ARRAY) {
									auto& other = reinterpret_cast<NodeArray&>(*executed[i]).Vector;
									for (auto i = other.begin(); i != other.end(); i++) {
										acc->Vector.push_back(*i);
									}
								} else return Error("non array value in array concatenation");
							}
							return acc;
						}
						case InstructionType::MULTIPLY:
							if (executed.size() == 2) {
								if (executed[1]->GetNodeType() == NODETYPE::NODE_ARRAY) {
									int ival = reinterpret_cast<NodeInteger&>(*executed[1]).Value;
									auto acc = std::make_shared<NodeArray>();
									while (ival--) {
										auto& other = left.Vector;
										for (auto i = other.begin(); i != other.end(); i++) {
											acc->Vector.push_back(*i);
										}
									};
									return acc;
								} else return Error("multiplication is not defined between the given arguments (try array * int)");
							} else return Error("multiplication when left side is array is only valid with an integer");
							break;
						default: return Error("binseq doesn't support the requested command");
					}
				}
				case NODETYPE::NODE_BIT: {
					if (node.CommandType == InstructionType::ADD ||
						node.CommandType == InstructionType::MULTIPLY) {
						auto i = executed.begin();
						auto acc = std::make_shared<NodeBit>(reinterpret_cast<NodeBit*>(&**i)->Value);
						for (i++; i != executed.end(); i++) {
							if ((*i)->GetNodeType() != NODETYPE::NODE_BIT) return Error("unexpected type in a bit expression");
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
							default: return Error("invalid expression");
						}
					} else return Error("error in expression");

				}

				default:
					return Error(std::string("arithmetic operators only works don't work on ") + (GetTypeText(executed[0]->GetNodeType())));
			}
		} else throw ImplementationException("arithmetic operators need to have at least 2 arguments");

	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteMemberOperator(NodeCommand& node) {
		if (node.Children.size() == 2) {
			auto lvalue = ExecuteStatement(node.Children[0]);
			auto rvalue = reinterpret_cast<Node*>(&*node.Children[1]);
			if (rvalue->GetNodeType() == NODETYPE::NODE_ATOM) {
				auto atom = reinterpret_cast<NodeAtom*>(rvalue);
				if (atom->GetAtomType() == InstructionType::ID) {
					auto& idx = atom->AtomText;
					if (lvalue->GetNodeType() == NODETYPE::NODE_OBJECT) {
						auto& object = reinterpret_cast<NodeObject&>(*lvalue);
						auto& container = object.Map;
						if (container.find(idx) != container.end())
							return container[idx];
						else
							return std::make_shared<Node>(NODETYPE::NODE_NONE);
					}
					else return Error("left side of an object member operator must be an object");
				}
				else return Error(atom->AtomText + " is not a valid indentifier");
			}
			else return Error("right side of an object member operator must be an identifier");
		}
		else throw ImplementationException("Member operator requires 2 parameters.");
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteAssignment(NodeCommand& node) {
		if (node.Children.size() == 2) {
			auto lvalue = reinterpret_cast<Node*>(&*node.Children[0]);
			auto rvalue = ExecuteStatement(node.Children[1]);
			if (lvalue->GetNodeType() == NODETYPE::NODE_ATOM) {
				auto atom = reinterpret_cast<NodeAtom*>(lvalue);
				if (atom->GetAtomType() == InstructionType::ID) {
					return this->SymbolTable[atom->AtomText] = rvalue;
				} else return Error(atom->AtomText + " is not a valid indentifier");
			} else if (lvalue->GetCommandType() == InstructionType::MEMBER)
			{
				auto& member = reinterpret_cast<NodeCommand&>(*lvalue);
				auto container = ExecuteStatement(member.Children[0]);
				if (container->GetNodeType() != NODE_OBJECT)
					return Error("left side of member operator is not an object");
				auto& object = reinterpret_cast<NodeObject&>(*container);
				auto& map = object.Map;
				auto& index = member.Children[1];
				if (index->GetAtomType() != InstructionType::ID)
					return Error("right side of member oeprator is not an identifier");
				auto& idx = reinterpret_cast<NodeAtom&>(*index).AtomText;
				map[idx] = rvalue;
				return rvalue;
			}
			return Error("left side of an assignment must be an identifier");
		} else throw ImplementationException("Assignment requires 2 parameters.");
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteBlock(NodeCommand& node) {
		std::shared_ptr<Node> result;
		for (auto i = node.Children.begin(); i != node.Children.end(); i++) {
			result = ExecuteStatement(*i);
			switch (result->GetNodeType()) {
				case NODETYPE::NODE_ERROR: return result;
				case NODETYPE::NODE_RETURN: return result;
				case NODETYPE::NODE_BREAK: return result;
				case NODETYPE::NODE_CONTINUE: return result;
				default: continue;
			}
		}
		return result;
	}

	inline std::shared_ptr<Node> ExecutorImp::ExecuteConditional(NodeCommand& node) {
		std::shared_ptr<Node> result;
		auto cond = ExecuteStatement(node.Children[0]);
		if (cond->GetNodeType() == NODETYPE::NODE_BIT) {
			if (reinterpret_cast<NodeBit&>(*cond).Value) {
				return ExecuteStatement(node.Children[1]);
			} else if (node.Children.size() == 3) {
				return ExecuteStatement(node.Children[2]);
			} else {
				return std::make_shared<Node>(NODETYPE::NODE_NONE);
			}
		} else return Error("condition is not a bit");
		return std::make_shared<Node>(NODETYPE::NODE_NONE);
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
			default: throw ImplementationException("loop expects 1 or 2 or 3 or 4 children");
		}

		if (init != nullptr) ExecuteStatement(init);

		while (true) {
			auto condResult = ExecuteStatement(cond);
			if (condResult->GetNodeType() == NODETYPE::NODE_BIT) {
				if (reinterpret_cast<NodeBit&>(*condResult).Value) {
					auto result = ExecuteStatement(body);
					if (result == nullptr || result->GetNodeType() == NODETYPE::NODE_BREAK)
						return std::make_shared<Node>(NODETYPE::NODE_NONE);
					if (result->GetNodeType() == NODETYPE::NODE_RETURN)
						return result;
					if (iterate != nullptr) ExecuteStatement(iterate);
				} else {
					break;
				}
			} else return Error("condition did not evaluate to a bit");
		}
		return std::make_shared<Node>(NODETYPE::NODE_NONE);
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
				result = std::make_shared<Node>(NODETYPE::NODE_BREAK);
				break;
			case InstructionType::CONTINUE:
				result = std::make_shared<Node>(NODETYPE::NODE_CONTINUE);
				break;
			case InstructionType::RETURN0:
				result = std::make_shared<NodeReturn>(nullptr);
				break;
			case InstructionType::RETURN1:
				result = std::make_shared<NodeReturn>(ExecuteBlock(node));
				break;
			case InstructionType::BLOCKBEGIN:
			case InstructionType::BLOCKEND:
			case InstructionType::CALLBEGIN:
			case InstructionType::CALLEND:
			case InstructionType::END_STATEMENT: throw ImplementationException("should have been processed");
			default: throw ImplementationException("unhandled case");
		}

		if (pushpop)
			SymbolTable.Pop();

		return result;
	}

	std::shared_ptr<Node> ExecutorImp::ExecuteStatement(std::shared_ptr<Node> node) {
		switch (node->GetNodeType()) {
			case NODETYPE::NODE_COMMAND:
				//pass by simple reference, we're already holding the shared ptr at least 2 times
				return ExecuteCommand(reinterpret_cast<NodeCommand&>(*node));
			case NODETYPE::NODE_ATOM: {
				auto id = reinterpret_cast<NodeAtom&>(*node);
				if (id.GetAtomType() == InstructionType::ID) {
					auto ptr = SymbolTable[id.AtomText];
					if (ptr != nullptr)
						return ptr;
					else
						return Error(id.AtomText + " is undefined");
				} else return node;
			}
			default:
				return node;
		}
	}

	inline std::shared_ptr<Node> ExecutorImp::Error(std::string message) {
		fprintf(stderr, "Runtime Error: %s.\n", message.c_str());
		return std::make_shared<Node>(NODETYPE::NODE_ERROR);
	}

	/* native function implementation */

	namespace native {

		static void view_primitive(Node& node, const char* sep = 0) {
			if (sep == 0) sep = " ";
			switch (node.GetNodeType()) {
				case NODETYPE::NODE_INTEGER: printf("%lld%s", reinterpret_cast<NodeInteger&>(node).Value, sep);
					break;
				case NODETYPE::NODE_FLOAT: printf("%g%s", reinterpret_cast<NodeFloat&>(node).Value, sep);
					break;
				case NODETYPE::NODE_STRING: printf("%s%s", reinterpret_cast<NodeString&>(node).Value.c_str(), sep);
					break;
				case NODETYPE::NODE_ARRAY: printf("array(%d)%s", reinterpret_cast<NodeArray&>(node).Vector.size(), sep);
					break;
				case NODETYPE::NODE_BITS: printf("binseq(%d)%s", reinterpret_cast<NodeArray&>(node).Vector.size(), sep);
					break;
				case NODETYPE::NODE_OBJECT: printf("object%s", sep);
					break;
				case NODETYPE::NODE_FUNCTION: printf("function%s", sep);
					break;
				case NODETYPE::NODE_BIT: printf("%d%s", reinterpret_cast<NodeBit&>(node).Value, sep);
					break;
				default: printf("?%s", sep);
			}
		}

		static std::shared_ptr<Node> view(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			for (auto i = node.begin(); i != node.end(); i++) {
				switch ((*i)->GetNodeType()) {
					case NODETYPE::NODE_ARRAY: {
						auto& n = reinterpret_cast<NodeArray&>(**i);
						auto size = n.Vector.size();
						printf("array(%d): (", size);
						for (unsigned i = 0; i < size; i++) {
							auto sep = i < size - 1 ? ", " : ") ";
							view_primitive(*n.Vector[i], sep);
						}
					}
						break;
					case NODETYPE::NODE_OBJECT: {
						auto& n = reinterpret_cast<NodeObject&>(**i);
						printf("object: (");
						auto i = n.Map.begin();
						if (i == n.Map.end())
						{
							printf(") ");
						}
						else
						{
							while (true) {
								auto j = i;
								j++;
								if (j == n.Map.end()) break;
								printf("%s: ", i->first.c_str());
								view_primitive(*(i->second), ", ");
								i = j;
							}
							printf("%s: ", i->first.c_str());
							view_primitive(*(i->second), ") ");
						}
					}
						break;
					case NODETYPE::NODE_BITS: {
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
			return std::make_shared<Node>(NODETYPE::NODE_NONE);
		}

		static std::shared_ptr<Node> system(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			auto result = 0;
			for (auto i = node.begin(); i != node.end(); i++) {
				if ((*i)->GetNodeType() == NODETYPE::NODE_STRING) {
					auto result = std::system(reinterpret_cast<NodeString&>(*node[0]).Value.c_str());
					if (result != 0) return std::make_shared<NodeInteger>(result);
				} else return ex->Error("parameter of system must be a string");
			}
			return std::make_shared<NodeInteger>(result);
		}

		static std::shared_ptr<Node> exit(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 1) {
				if (node[0]->GetNodeType() == NODETYPE::NODE_INTEGER) {
					std::exit((int)reinterpret_cast<NodeInteger&>(*node[0]).Value);
				} else return ex->Error("parameter of exit must be an integer");
			} else if (node.size() == 0) {
				std::exit(0);
			} else return ex->Error("exit requires 0 or 1 paramter");
		}

		static std::shared_ptr<Node> del(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			int released = 0;
			for (auto i = node.begin(); i != node.end(); i++) {
				if ((*i)->GetNodeType() == NODETYPE::NODE_STRING) {
					auto str = reinterpret_cast<NodeString&>(**i);
					auto& ptr = ex->SymbolTable[str.Value];
					if (ptr != nullptr) {
						ptr = nullptr;
						released++;
					}
				} else return ex->Error("delete requires strings as a parameters");
			}
			return std::make_shared<NodeInteger>(released);
		}

		static std::shared_ptr<Node> file_read(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			//read(16,32,"x.bin");
			if (node.size() <= 0 || node.size() > 3) return ex->Error("incorrect number of parameters at file read call");
			binseq::u64 read_offset = 0, read_size = -1;
			const char* fname = nullptr;
			if (node[0]->GetNodeType() == NODETYPE::NODE_INTEGER) {
				read_size = reinterpret_cast<NodeInteger&>(*node[0]).Value;
				if (node[1]->GetNodeType() == NODETYPE::NODE_INTEGER) {
					read_offset = reinterpret_cast<NodeInteger&>(*node[1]).Value;
					if (node[2]->GetNodeType() == NODETYPE::NODE_STRING) {
						fname = reinterpret_cast<NodeString&>(*node[2]).Value.c_str();
					} else return ex->Error("unexpected type, parameter 3 of file read");
				} else if (node[1]->GetNodeType() == NODETYPE::NODE_STRING) {
					fname = reinterpret_cast<NodeString&>(*node[1]).Value.c_str();
				} else return ex->Error("unexpected type, parameter 2 of file read");
			} else if (node[0]->GetNodeType() == NODETYPE::NODE_STRING) {
				fname = reinterpret_cast<NodeString&>(*node[0]).Value.c_str();
			} else return ex->Error("unexpected type, parameter 1 of file read");

			auto f = fopen(fname, "rb");
			if (f == nullptr) return ex->Error(std::string("failed to open file ") + fname);
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

		static std::shared_ptr<Node> file_write(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			//write(b"0010",16,"x.bin");
			if (node.size() == 2) {
				if (node[0]->GetNodeType() != NODETYPE::NODE_BITS) return ex->Error("first parameter of file write must be a binseq");
				if (node[1]->GetNodeType() != NODETYPE::NODE_STRING) return ex->Error("second parameter of file write must be a string");
				auto& seq = reinterpret_cast<NodeBits&>(*node[0]).Value;
				auto fname = reinterpret_cast<NodeString&>(*node[1]).Value.c_str();
				auto f = fopen(fname, "wb");
				if (f == nullptr) return ex->Error(std::string("could not open ") + fname + " for writing");
				fwrite(seq.address(), (seq.size() + 7) >> 3, 1, f);
				fclose(f);
			} else if (node.size() == 3) {
				if (node[0]->GetNodeType() != NODETYPE::NODE_BITS) return ex->Error("first parameter of file write must be a binseq");
				if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("second parameter of file write must be an integer");
				if (node[2]->GetNodeType() != NODETYPE::NODE_STRING) return ex->Error("third parameter of file write must be a string");
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
			return std::make_shared<Node>(NODETYPE::NODE_NONE);
		}

		static std::shared_ptr<Node> cast_integer(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeInteger>(0);
			} else if (node.size() == 1) {
				long long ival = 0;
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_INTEGER: return node[0];
					case NODETYPE::NODE_FLOAT: ival = (long long)reinterpret_cast<NodeFloat&>(*node[0]).Value;
						break;
					case NODETYPE::NODE_BIT: ival = (long long)reinterpret_cast<NodeBit&>(*node[0]).Value;
						break;
					case NODETYPE::NODE_STRING: ival = (long long)atol(reinterpret_cast<NodeString&>(*node[0]).Value.c_str());
						break;
					case NODETYPE::NODE_BITS: ival = *((long long*) reinterpret_cast<NodeBits&>(*node[0]).Value.address());
						break;
					default: return ex->Error("cannot convert parameter to integer");
				}
				return std::make_shared<NodeInteger>(ival);
			}
			return ex->Error("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_float(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeFloat>(.0);
			} else if (node.size() == 1) {
				double val = 0;
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_FLOAT: return node[0];
					case NODETYPE::NODE_INTEGER: val = (double)reinterpret_cast<NodeInteger&>(*node[0]).Value;
						break;
					case NODETYPE::NODE_BIT: val = (double)reinterpret_cast<NodeBit&>(*node[0]).Value;
						break;
					case NODETYPE::NODE_STRING: val = (double)atof(reinterpret_cast<NodeString&>(*node[0]).Value.c_str());
					case NODETYPE::NODE_BITS: val = *((double*) reinterpret_cast<NodeBits&>(*node[0]).Value.address());
						break;
					default: return ex->Error("cannot convert parameter to float");
				}
				return std::make_shared<NodeFloat>(val);
			}
			return ex->Error("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_bit(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeFloat>(.0);
			} else if (node.size() == 1) {
				bool val = 0;
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_BIT: return node[0];
					case NODETYPE::NODE_FLOAT: val = (bool) reinterpret_cast<NodeFloat&>(*node[0]).Value;
						break;
					case NODETYPE::NODE_INTEGER: val = (bool) reinterpret_cast<NodeInteger&>(*node[0]).Value;
						break;
					case NODETYPE::NODE_STRING: val = !reinterpret_cast<NodeString&>(*node[0]).Value.compare("0") == 0;
						break;
					case NODETYPE::NODE_BITS: val = ((bool) reinterpret_cast<NodeBits&>(*node[0]).Value[0]);
						break;
					default: return ex->Error("cannot convert parameter to bit");
				}
				return std::make_shared<NodeBit>(val);
			}
			return ex->Error("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_string(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeFloat>(.0);
			} else if (node.size() == 1) {
				char buffer[256];
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_BIT: sprintf_s(buffer, "%d", reinterpret_cast<NodeBit&>(*node[0]).Value);
						break;
					case NODETYPE::NODE_FLOAT: sprintf_s(buffer, "%g", reinterpret_cast<NodeFloat&>(*node[0]).Value);
						break;
					case NODETYPE::NODE_INTEGER: sprintf_s(buffer, "%lld", reinterpret_cast<NodeInteger&>(*node[0]).Value);
						break;
					case NODETYPE::NODE_STRING: return node[0];
					case NODETYPE::NODE_BITS: {
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
					default: return ex->Error("cannot convert parameter to float");
				}
				return std::make_shared<NodeString>(buffer);
			}
			return ex->Error("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_bits(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeBits>();
			} else if (node.size() == 1) {
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_BIT:
						return std::make_shared<NodeBits>(
							binseq::bit_sequence(reinterpret_cast<NodeBit&>(*node[0]).Value));
					case NODETYPE::NODE_INTEGER:
						return std::make_shared<NodeBits>(
							binseq::bit_sequence((binseq::u64)reinterpret_cast<NodeInteger&>(*node[0]).Value));
					case NODETYPE::NODE_FLOAT:
						return std::make_shared<NodeBits>(
							binseq::bit_sequence(*((binseq::u64*)&reinterpret_cast<NodeFloat&>(*node[0]).Value)));
					case NODETYPE::NODE_BITS: return node[0];
					case NODETYPE::NODE_STRING:
						return std::make_shared<NodeBits>(
							binseq::bit_sequence(reinterpret_cast<NodeString&>(*node[0]).Value.c_str()));
						break;
					default: return ex->Error("cannot convert parameter to binseq");
				}
			}
			return ex->Error("conversion function expects no more than one parameter");
		}

		static std::shared_ptr<Node> cast_array(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeArray>();
			} else if (node.size() == 1) {
				if (node[0]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("array can only receive integer as a parameter");
				auto& size = reinterpret_cast<NodeInteger&>(*node[0]).Value;
				auto newnode = std::make_shared<NodeArray>(size);
				auto nothing = std::make_shared<Node>(NODETYPE::NODE_NONE);
				auto& vec = newnode->Vector;
				for (auto i = vec.begin(); i != vec.end(); i++) {
					*i = nothing;
				}
				return newnode;
			} else {
				return ex->Error("array can't have more than 1 parameter");
			}
		}

		static std::shared_ptr<Node> cast_object(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeObject>();
			} else return ex->Error("object constructor accepts no parameters");
		}

		static std::shared_ptr<Node> type(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 0) {
				return std::make_shared<NodeString>("void");
			} else if (node.size() == 1) {
				const char* r = "unknown";
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_ATOM:
						switch (node[0]->GetAtomType()) {
							case InstructionType::ID: r = "identifier";
								break;
							default: r = "unprocessed atom";
								break;
						}
						break;
					case NODETYPE::NODE_BIT: r = "bit";
						break;
					case NODETYPE::NODE_BITS: r = "binseq";
						break;
					case NODETYPE::NODE_COMMAND: r = "command";
						break;
					case NODETYPE::NODE_FLOAT: r = "float";
						break;
					case NODETYPE::NODE_FUNCTION: r = "function";
						break;
					case NODETYPE::NODE_INTEGER: r = "integer";
						break;
					case NODETYPE::NODE_ERROR: r = "error";
						break;
					case NODETYPE::NODE_NONE: r = "void";
						break;
					case NODETYPE::NODE_STRING: r = "string";
						break;
					case NODETYPE::NODE_ARRAY: r = "array";
						break;
					case NODETYPE::NODE_OBJECT: r = "object";
						break;
				}
				return std::make_shared<NodeString>(r);
			}
			return ex->Error("type function expects no more than one parameter");
		}


		static std::shared_ptr<Node> get(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 2) {
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_STRING: {
						if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("string can only be indexed by integer");
						auto& container = reinterpret_cast<NodeString&>(*node[0]).Value;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						if (idx < 0 || idx >= container.size()) return ex->Error("string index out of bounds");
						return std::make_shared<NodeInteger>((unsigned char)container[idx]);
					}
						break;

					case NODETYPE::NODE_ARRAY: {
						if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("array can only be indexed by integer");
						auto& container = reinterpret_cast<NodeArray&>(*node[0]).Vector;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						if (idx < 0 || idx >= container.size()) return ex->Error("string index out of bounds");
						return container[idx];
					}
						break;

					case NODETYPE::NODE_BITS: {
						if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("string can only be indexed by integer");
						auto& container = reinterpret_cast<NodeBits&>(*node[0]).Value;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						if (idx < 0 || idx >= container.size()) return ex->Error("string index out of bounds");
						return std::make_shared<NodeBit>((bool)container[idx]);
						return node[2];
					}
						break;

					case NODETYPE::NODE_OBJECT: {
						if (node[1]->GetNodeType() != NODETYPE::NODE_STRING) return ex->Error("object can only be indexed by string");
						auto& container = reinterpret_cast<NodeObject&>(*node[0]).Map;
						auto& idx = reinterpret_cast<NodeString&>(*node[1]).Value;
						if (container.find(idx) != container.end())
							return container[idx];
						else
							return std::make_shared<Node>(NODETYPE::NODE_NONE);
					}
						break;

					default: return ex->Error("get requires the first parameter to be a container");
				}
			} else return ex->Error("get requires one container and one index");
		}


		static std::shared_ptr<Node> set(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 3) {
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_STRING: {
						if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("string can only be indexed by integer");
						if (node[2]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("value must be an integer");
						auto& container = reinterpret_cast<NodeString&>(*node[0]).Value;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						auto& val = reinterpret_cast<NodeInteger&>(*node[2]).Value;
						if (idx < 0 || idx > container.size()) return ex->Error("string index out of bounds");
						container[idx] = val;
						return node[2];
					}
						break;

					case NODETYPE::NODE_ARRAY: {
						if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("array can only be indexed by integer");
						auto& container = reinterpret_cast<NodeArray&>(*node[0]).Vector;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						auto& val = node[2];
						if (idx < 0 || idx > container.size()) return ex->Error("string index out of bounds");
						container[idx] = val;
						return node[2];
					}
						break;

					case NODETYPE::NODE_BITS: {
						if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("string can only be indexed by integer");
						if (node[2]->GetNodeType() != NODETYPE::NODE_BIT) return ex->Error("value must be a bit");
						auto& container = reinterpret_cast<NodeBits&>(*node[0]).Value;
						auto& idx = reinterpret_cast<NodeInteger&>(*node[1]).Value;
						auto& val = reinterpret_cast<NodeBit&>(*node[2]).Value;
						if (idx < 0 || idx > container.size()) return ex->Error("string index out of bounds");
						container[idx] = val;
						return node[2];
					}
						break;

					case NODETYPE::NODE_OBJECT: {
						if (node[1]->GetNodeType() != NODETYPE::NODE_STRING) return ex->Error("object can only be indexed by string");
						auto& container = reinterpret_cast<NodeObject&>(*node[0]).Map;
						auto& idx = reinterpret_cast<NodeString&>(*node[1]).Value;
						auto& val = node[2];
						container[idx] = val;
						return node[2];
					}
						break;

					default: return ex->Error("set requires the first parameter to be a container");
				}
			} else return ex->Error("set requires container, index and value");
		}

		static std::shared_ptr<Node> length(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 1) {
				long long val = 0;
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_STRING: val = reinterpret_cast<NodeString&>(*node[0]).Value.size();
						break;
					case NODETYPE::NODE_ARRAY: val = reinterpret_cast<NodeArray&>(*node[0]).Vector.size();
						break;
					case NODETYPE::NODE_BITS: val = reinterpret_cast<NodeBits&>(*node[0]).Value.size();
						break;
					default: return ex->Error("parameter has no length, only array, string and binseq has length");
				}
				return std::make_shared<NodeInteger>(val);
			} else return ex->Error("length requires one parameter");
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
			for (auto i = vec.begin() + offset; i != vec.end() && count > 0; i++ , count--) {
				result.push_back(*i);
			}
			return result;
		}

		template <class T>
		static T vec_repeat(T& vec, long long count) {
			T result;
			auto i = vec.begin();
			while (count) {
				if (i == vec.end());
				i = vec.begin();
				result.push_back(*i);
				i++;
				count--;
			}
			return result;
		}

		static std::shared_ptr<Node> sel_head(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 2) {
				if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("second parameter of head must be an integer");
				auto count = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_BITS: return std::make_shared<NodeBits>(binseq::head(reinterpret_cast<NodeBits&>(*node[0]).Value, count));
					case NODETYPE::NODE_STRING: return std::make_shared<NodeString>(vec_head(reinterpret_cast<NodeString&>(*node[0]).Value, count));
					case NODETYPE::NODE_ARRAY: return std::make_shared<NodeArray>(vec_head(reinterpret_cast<NodeArray&>(*node[0]).Vector, count));
					default: return ex->Error("head only works on sequences");
				}
			} else return ex->Error("head needs 2 parameters, a sequence and an integer");
		}

		static std::shared_ptr<Node> sel_tail(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 2) {
				if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("second parameter of tail must be an integer");
				auto count = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_BITS: return std::make_shared<NodeBits>(binseq::tail(reinterpret_cast<NodeBits&>(*node[0]).Value, count));
					case NODETYPE::NODE_STRING: return std::make_shared<NodeString>(vec_tail(reinterpret_cast<NodeString&>(*node[0]).Value, count));
					case NODETYPE::NODE_ARRAY: return std::make_shared<NodeArray>(vec_tail(reinterpret_cast<NodeArray&>(*node[0]).Vector, count));
					default: return ex->Error("tail only works on sequences");
				}
			} else return ex->Error("tail needs 2 parameters, a sequence and an integer");
		}

		static std::shared_ptr<Node> sel_subseq(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 3) {
				if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("second parameter of subseq must be an integer");
				if (node[2]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("third parameter of subseq must be an integer");
				auto offset = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				auto count = reinterpret_cast<NodeInteger&>(*node[2]).Value;
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_BITS: return std::make_shared<NodeBits>(binseq::subseq(reinterpret_cast<NodeBits&>(*node[0]).Value, offset, count));
					case NODETYPE::NODE_STRING: return std::make_shared<NodeString>(vec_subseq(reinterpret_cast<NodeString&>(*node[0]).Value, offset, count));
					case NODETYPE::NODE_ARRAY: return std::make_shared<NodeArray>(vec_subseq(reinterpret_cast<NodeArray&>(*node[0]).Vector, offset, count));
					default: return ex->Error("subseq only works on sequences");
				}
			} else return ex->Error("subseq needs 3 parameters, a sequence and two integers");
		}

		static std::shared_ptr<Node> seq_repeat(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 2) {
				if (node[1]->GetNodeType() != NODETYPE::NODE_INTEGER) return ex->Error("second parameter of repeat must be an integer");
				auto count = reinterpret_cast<NodeInteger&>(*node[1]).Value;
				switch (node[0]->GetNodeType()) {
					case NODETYPE::NODE_BITS: return std::make_shared<NodeBits>(binseq::repeat(reinterpret_cast<NodeBits&>(*node[0]).Value, count));
					case NODETYPE::NODE_STRING: return std::make_shared<NodeString>(vec_repeat(reinterpret_cast<NodeString&>(*node[0]).Value, count));
					case NODETYPE::NODE_ARRAY: return std::make_shared<NodeArray>(vec_repeat(reinterpret_cast<NodeArray&>(*node[0]).Vector, count));
					default: return ex->Error("repeat only works on sequences");
				}
			} else return ex->Error("repeat needs 2 parameters, a sequence and an integer");
		}

		static std::shared_ptr<Node> popcount(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() == 1) {
				if (node[0]->GetNodeType() != NODETYPE::NODE_BITS) return ex->Error("the parameter of popcount must be binseq");
				auto& seq = reinterpret_cast<NodeBits&>(*node[0]).Value;
				auto count = binseq::popcount(seq);
				return std::make_shared<NodeInteger>((long long)count);
			} else return ex->Error("popcount only accepts one parameter");
		}

		static std::shared_ptr<Node> verbose(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
			if (node.size() > 1) {
				return ex->Error("verbose accepts only one or zero parameters of string which should contain the words tree, none, submit");
			}
			if (node.size() == 1) {
				if (node[0]->GetNodeType() == NODETYPE::NODE_STRING) {
					auto& node0 = reinterpret_cast<NodeString&>(*node[0]);
					auto& str = node0.Value;
					if (str.find("submit") != std::string::npos) ex->VERBOSE_SUBMIT = true;
					if (str.find("tree") != std::string::npos) ex->VERBOSE_TREE = true;
					if (str.find("none") != std::string::npos) ex->VERBOSE_SUBMIT = ex->VERBOSE_TREE = false;
				} else return ex->Error("parameter is not a string");
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
				if (node[0]->GetNodeType() == NODETYPE::NODE_OBJECT) {
					auto& obj = reinterpret_cast<NodeObject&>(*node[0]);
					auto arr = std::make_shared<NodeArray>();
					std::vector<std::string> propList;
					for (auto i = obj.Map.begin(); i != obj.Map.end(); i++) {
						arr->Vector.push_back(std::make_shared<NodeString>(i->first));
					}
					return arr;
				} else return ex->Error("first parameter of properties must be an object");

			} else return ex->Error("properties must be invoked with an object or no paramteres");
		}

		namespace op {


			static std::shared_ptr<Node> not(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 1) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BIT) {
						auto& v = reinterpret_cast<NodeBit&>(*node[0]);
						return std::make_shared<NodeBit>(!v.Value);
					} else if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& v = reinterpret_cast<NodeBits&>(*node[0]);
						return std::make_shared<NodeBits>(binseq::not(v.Value));
					} else return ex->Error("not operator only works on binseq or bit");
				} else return ex->Error("not operator requires 1 parameter");
			}

			static std::shared_ptr<Node> and(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BIT) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>((l.Value && r.Value));
					} else if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::and(l.Value, r.Value));
					} else return ex->Error("bitwise operator only works on binseq or bit");
				} else return ex->Error("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> or(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BIT) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>((l.Value || r.Value));
					} else if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::or(l.Value, r.Value));
					} else return ex->Error("bitwise operator only works on binseq or bit");
				} else return ex->Error("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> xor(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BIT) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>(l.Value != r.Value);
					} else if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::xor(l.Value, r.Value));
					} else return ex->Error("bitwise operator only works on binseq or bit");
				} else return ex->Error("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nand(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BIT) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>(!(l.Value && r.Value));
					} else if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nand(l.Value, r.Value));
					} else return ex->Error("bitwise operator only works on binseq or bit");
				} else return ex->Error("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nor(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BIT) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>(!(l.Value || r.Value));
					} else if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nor(l.Value, r.Value));
					} else return ex->Error("bitwise operator only works on binseq or bit");
				} else return ex->Error("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nxor(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BIT) {
						auto& l = reinterpret_cast<NodeBit&>(*node[0]);
						auto& r = reinterpret_cast<NodeBit&>(*node[1]);
						return std::make_shared<NodeBit>(l.Value == r.Value);
					} else if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nxor(l.Value, r.Value));
					} else return ex->Error("bitwise operator only works on binseq or bit");
				} else return ex->Error("bitwise operator requires 2 parameters");
			}

			static std::shared_ptr<Node> andc(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::andc(l.Value, r.Value));
					} else return ex->Error("bitwise cut operator only works on binseq");
				} else return ex->Error("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> orc(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::orc(l.Value, r.Value));
					} else return ex->Error("bitwise cut operator only works on binseq");
				} else return ex->Error("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> xorc(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::xorc(l.Value, r.Value));
					} else return ex->Error("bitwise cut operator only works on binseq");
				} else return ex->Error("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nandc(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nandc(l.Value, r.Value));
					} else return ex->Error("bitwise cut operator only works on binseq");
				} else return ex->Error("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> norc(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::norc(l.Value, r.Value));
					} else return ex->Error("bitwise cut operator only works on binseq");
				} else return ex->Error("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nxorc(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nxorc(l.Value, r.Value));
					} else return ex->Error("bitwise cut operator only works on binseq");
				} else return ex->Error("bitwise cut operator requires 2 parameters");
			}

			static std::shared_ptr<Node> andr(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::andr(l.Value, r.Value));
					} else return ex->Error("bitwise repeat operator only works on binseq");
				} else return ex->Error("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> orr(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::orr(l.Value, r.Value));
					} else return ex->Error("bitwise repeat operator only works on binseq");
				} else return ex->Error("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> xorr(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::xorr(l.Value, r.Value));
					} else return ex->Error("bitwise repeat operator only works on binseq");
				} else return ex->Error("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nandr(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nandr(l.Value, r.Value));
					} else return ex->Error("bitwise repeat operator only works on binseq");
				} else return ex->Error("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> norr(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::norr(l.Value, r.Value));
					} else return ex->Error("bitwise repeat operator only works on binseq");
				} else return ex->Error("bitwise repeat operator requires 2 parameters");
			}

			static std::shared_ptr<Node> nxorr(ExecutorImp* ex, std::vector<std::shared_ptr<Node>>& node) {
				if (node.size() == 2) {
					if (node[0]->GetNodeType() == NODETYPE::NODE_BITS) {
						auto& l = reinterpret_cast<NodeBits&>(*node[0]);
						auto& r = reinterpret_cast<NodeBits&>(*node[1]);
						return std::make_shared<NodeBits>(binseq::nxorr(l.Value, r.Value));
					} else return ex->Error("bitwise repeat operator only works on binseq");
				} else return ex->Error("bitwise repeat operator requires 2 parameters");
			}

		};

	};

	/* native function resitration */

	ExecutorImp::ExecutorImp() {
		this->ControlLevel = 0;
		this->SymbolTable.Global("void") = std::make_shared<Node>(NODETYPE::NODE_NONE);
		this->SymbolTable.Global("error") = std::make_shared<Node>(NODETYPE::NODE_ERROR);
		//system commands
		RegisterNativeFunction("system", native::system, false);
		RegisterNativeFunction("exit", native::exit, false);
		RegisterNativeFunction("delete", native::del, false);
		RegisterNativeFunction("verbose", native::verbose, false);

		//printing
		RegisterNativeFunction("view", native::view, false);

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
		RegisterNativeFunction("properties", native::properties, false);

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
		auto fname = reinterpret_cast<NodeAtom*>(&*node.Children[0])->AtomText;
		auto nodeptr = SymbolTable[fname];
		if (nodeptr != nullptr && nodeptr->GetNodeType() == NODETYPE::NODE_FUNCTION) {
			auto function = reinterpret_cast<NodeFunction*>(&*nodeptr);
			std::vector<std::shared_ptr<Node>> paramlist;
			for (auto i = ++node.Children.begin(); i != node.Children.end(); i++) {
				paramlist.push_back(ExecuteStatement(*i));
			}
			if (function->Native) {
				return function->nativeptr(this, paramlist);
			} else {
				SymbolTable.Push();
				unsigned limit = function->ParameterList.size();
				if (limit > paramlist.size()) limit = paramlist.size();
				for (unsigned i = 0; i < limit; i++)
					SymbolTable.Local(function->ParameterList[i]) = paramlist[i];
				auto result = ExecuteStatement(function->Implementation);
				SymbolTable.Pop();
				while (result->GetNodeType() == NODETYPE::NODE_RETURN) {
					result = reinterpret_cast<NodeReturn&>(*result).Value;
				}
				return result;
			}
		} else return Error(fname + " is not a function");
	}

}
