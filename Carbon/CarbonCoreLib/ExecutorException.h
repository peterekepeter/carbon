#pragma once

namespace Carbon
{
	// generic execution context exception, this should be the base for all execution exceptions
	class ExecutorExeption : public std::logic_error {
	public:
		const char* GetMessage() const;
		explicit ExecutorExeption(const char* message) : logic_error(message) {};
		explicit ExecutorExeption(const std::string& message) : logic_error(message) {};
	};

	inline const char* ExecutorExeption::GetMessage() const
	{
		return this->what();
	}

	// exception due to incomplete implementation of the executor
	class ExecutorImplementationException : public ExecutorExeption {
	public:
		explicit ExecutorImplementationException(const char* message) : ExecutorExeption(message) {};
		explicit ExecutorImplementationException(const std::string& message) : ExecutorExeption(message) {};
	};

	// exception occured during the runtime of the interpreted language
	class ExecutorRuntimeException : public ExecutorExeption {
	public:
		explicit ExecutorRuntimeException(const char* message) : ExecutorExeption(message) {};
		explicit ExecutorRuntimeException(const std::string& message) : ExecutorExeption(message) {};
	};

	
}