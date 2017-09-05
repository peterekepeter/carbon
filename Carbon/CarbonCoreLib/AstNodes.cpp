#include "AstNodes.h"

namespace Carbon
{
	NodeType Node::GetNodeType() const
	{
		return Type;
	}

	const char* Node::GetText() {
		throw ExecutorImplementationException("Unspecified node type.");
	}

	InstructionType Node::GetCommandType() {
		throw ExecutorImplementationException("Unspecified node type.");
	}

	InstructionType Node::GetAtomType() {
		throw ExecutorImplementationException("Unspecified node type.");
	}

	const char* Node::GetAtomText() {
		throw ExecutorImplementationException("Unspecified node type.");
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

	const char* GetTypeText(NodeType nodetype)
	{
		switch (nodetype)
		{
		case NodeType::Atom: return "identifier";
		case NodeType::Bit: return "bit";
		case NodeType::Bits: return "bitseq";
		case NodeType::Command: return "command";
		case NodeType::Float: return "float";
		case NodeType::Function: return "function";
		case NodeType::Integer: return "integer";
		case NodeType::Error: return "error";
		case NodeType::None: return "void";
		case NodeType::String: return "string";
		case NodeType::DynamicArray: return "array";
		case NodeType::DynamicObject: return "object";
		default: throw ExecutorImplementationException("Unhandled nodetype.");
		}
	}

	bool Node::IsAtom() const
	{
		return Type == NodeType::Atom;
	}

	bool Node::IsCommand() const
	{
		return Type == NodeType::Command;
	}


	NodeReturn::NodeReturn(std::shared_ptr<Node> value) : Value(value), Node(NodeType::Return) {}

	const char* NodeReturn::GetText() {
		return "return";
	}


	NodeAtom::NodeAtom(const InstructionType type, const char* text) : Node(NodeType::Atom) {
		this->AtomType = type;
		this->AtomText = text;
	}

	Node::Node(const NodeType ntype) {
		this->Type = ntype;
	}

	NodeCommand::NodeCommand(const InstructionType cmd) : Node(NodeType::Command) {
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
		default: throw ExecutorImplementationException("Unhandled commandtype.");
		}
	}





	const char* NodeBits::GetText() {
		return "binseq";
	}

	NodeBits::NodeBits() :Node(NodeType::Bits) {}

	NodeBits::NodeBits(const binseq::bit_sequence& b) : Node(NodeType::Bits) {
		Value = b;
	}

	NodeBits::NodeBits(binseq::bit_sequence&& b) : Node(NodeType::Bits) {
		Value = b;
	}


	const char* NodeObject::GetText() {
		return "object";
	}

	const char* NodeArray::GetText() {
		return "array";
	}

	NodeObject::NodeObject() :Node(NodeType::DynamicObject) {};



	NodeArray::NodeArray(int size) :Node(NodeType::DynamicArray) {
		Vector.resize(size);
	}

	NodeArray::NodeArray() : Node(NodeType::DynamicArray) { }

	NodeArray::NodeArray(const std::vector<std::shared_ptr<Node>>& vec) : Node(NodeType::DynamicArray) {
		Vector = vec;
	};

	NodeArray::NodeArray(std::vector<std::shared_ptr<Node>>&& vec) :Node(NodeType::DynamicArray) {
		Vector = vec;
	};

}
