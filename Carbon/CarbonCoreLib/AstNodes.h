#pragma once
#include "../CarbonCommonLib/InstructionType.h"
#include <memory>
#include <vector>
#include "../BinseqLib/bit_sequence.hpp"
#include <unordered_map>
#include "ExecutorException.h"


namespace Carbon
{
	enum class NodeType {
		// basic commands 
		None,
		Error,
		Continue,
		Break,
		Return,
		Atom,
		Command,

		// builtin types
		Integer,
		Float,
		String,
		Bit,
		Bits,
		Function,
		DynamicArray,
		DynamicObject,
	};
	
	// get a displayable type text for a NodeType
	const char* GetTypeText(NodeType nodetype);

	class Node {
	public:
		bool IsAtom() const;
		bool IsCommand() const;
		NodeType GetNodeType() const;

		virtual const char* GetText(); //get a description to help debug
		virtual InstructionType GetCommandType();
		virtual InstructionType GetAtomType();
		virtual const char* GetAtomText(); //get parsed text    
		Node(const NodeType);
	protected:
		NodeType Type;
	};

	// a native function receives a vector of nodes and retursn another node
	typedef std::shared_ptr<Node>(*native_function_ptr)(std::vector<std::shared_ptr<Node>>& node);

	class NodeAtom : public Node {
	public:
		InstructionType AtomType;
		std::string AtomText;
		NodeAtom(const InstructionType type, const char* text);
		virtual const char* GetText() override;
		virtual InstructionType GetAtomType() override;
		virtual  const char* GetAtomText() override; //get parsed text
	};

	class NodeString : public Node {
	public:
		std::string Value;
		virtual const char* GetText() override;
		NodeString(const std::string str);
	};

	class NodeFunction : public Node {
	public:
		bool Native;
		bool Pure;
		NodeFunction(native_function_ptr nativeptr, bool pure);
		NodeFunction(std::vector<std::string> parameterList, std::shared_ptr<Node> impl);
		native_function_ptr nativeptr;
		bool InternalNative;
		std::vector<std::string> ParameterList;
		std::shared_ptr<Node> Implementation;
		virtual const char* GetText() override;
	};

	class NodeInteger : public Node {
	public:
		long long Value;
		virtual const char* GetText() override;
		NodeInteger(long long value);
		std::shared_ptr<std::string> text;
	};

	class NodeBits : public Node {
	public:
		binseq::bit_sequence Value;
		virtual const char* GetText() override;
		NodeBits();
		NodeBits(const binseq::bit_sequence& b);
		NodeBits(binseq::bit_sequence&& b);
	};
	class NodeArray : public Node {
	public:
		std::vector<std::shared_ptr<Node>> Vector;
		virtual const char* GetText() override;
		NodeArray();
		NodeArray(int size);
		NodeArray(const std::vector<std::shared_ptr<Node>>& vec);
		NodeArray(std::vector<std::shared_ptr<Node>>&& vec);
	};

	class NodeObject : public Node {
	public:
		std::unordered_map<std::string, std::shared_ptr<Node>> Map;
		virtual const char* GetText() override;
		NodeObject();
	};

	class NodeBit : public Node {
	public:
		bool Value;
		virtual const char* GetText() override;
		NodeBit(bool value);
	};

	class NodeFloat : public Node {
	public:
		double Value;
		virtual const char* GetText() override;
		NodeFloat(double value);
		std::shared_ptr<std::string> text;
	};

	class NodeCommand : public Node {
	public:
		InstructionType CommandType;
		std::vector<std::shared_ptr<Node>> Children;
		NodeCommand(const InstructionType);
		virtual const char* GetText() override;
		virtual InstructionType GetCommandType() override;
		bool DoesPushStack;
		bool IsPure;
	};

	class NodeReturn : public Node {
	public:
		std::shared_ptr<Node> Value;
		NodeReturn(std::shared_ptr<Node> return_value);
		virtual const char* GetText() override;
	};


}

