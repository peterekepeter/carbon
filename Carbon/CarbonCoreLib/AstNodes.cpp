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

	bool Node::IsNone() const
	{
		return Type == NodeType::None;
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
		case InstructionType::RETURN0: return "RETURN0";
		case InstructionType::RETURN1: return "RETURN1";
		case InstructionType::FUNCTION_OPERATOR: return "FUNCTION_OPERATOR";
		case InstructionType::COMMA: return "COMMA";
		case InstructionType::VOIDEXPR: return "VOIDEXPR";
		default: {
			throw ExecutorImplementationException("Unhandled commandtype.");
		}
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

	static int NameIdGenerator = 0;
	static std::unordered_map<std::string, size_t> NameIdMap;
	static std::unordered_map<size_t, std::string> IdNameMap;

	std::vector<std::string> NodeObject::GetAttributeKeys()
	{
		std::vector<std::string> result;
		for (auto id : this->Map) {
			result.push_back(IdNameMap[id.first]);
		}
		return result;
	}

	void NodeObject::SetAttributeValue(const std::string & byName, std::shared_ptr<Node> value)
	{
		SetAttributeValue(GetNameId(byName), value);
	}

	void NodeObject::SetAttributeValue(const size_t byNameId, std::shared_ptr<Node> value)
	{
		this->Map[byNameId] = value;
	}

	std::shared_ptr<Node> NodeObject::GetAttributeValue(const std::string & byName)
	{
		return GetAttributeValue(GetNameId(byName));
	}

	std::shared_ptr<Node> NodeObject::GetAttributeValue(const size_t byNameId)
	{
		return this->Map[byNameId];
	}

	size_t NodeObject::GetNameId(const std::string & name)
	{
		size_t id;
		auto search = NameIdMap.find(name);
		if (search == NameIdMap.end()) {
			id = ++NameIdGenerator;
			NameIdMap[name] = id;
			IdNameMap[id] = name;
		}
		else {
			id = search->second;
		}
		return id;
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

	NodeStructureFactory::NodeStructureFactory(): Node(NodeType::StrctureFactory) { }

	const char* NodeStructureFactory::GetText()
	{
		if (IsArrayFactory) return "structure_factory(array)";
		if (IsObjectFactory) return "structure_factory(object)";
		return "structure_factory";
	}

}
