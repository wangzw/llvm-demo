/**
 * Demonstrate how to load IR bytecode and execute it.
 */

#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm-c/IRReader.h"
#include "llvm-c/Target.h"

#include <iostream>

int main() {
	const char * path = "demo.bt";
	const char * funcname = "PrintMsg";
	LLVMContextRef cxt;
	LLVMMemoryBufferRef buffer;
	LLVMModuleRef module;
	char * error = NULL;

	aa;

	LLVMInitializeNativeTarget();
	LLVMLinkInJIT();

	cxt = LLVMGetGlobalContext();

	if (LLVMCreateMemoryBufferWithContentsOfFile(path, &buffer, &error)) {
		std::cerr << "cannot read IR bytecode from file " << path << " "
				<< (error ? error : "") << std::endl;
		return -1;
	}

	if (LLVMParseIRInContext(cxt, buffer, &module, &error)) {
		std::cerr << "cannot parse IR bytecode " << (error ? error : "")
				<< std::endl;
		return -1;
	}

	LLVMExecutionEngineRef engine;

	if (LLVMCreateJITCompilerForModule(&engine, module, 0, &error)) {
		std::cerr << "cannot create execution engine " << (error ? error : "")
				<< std::endl;
		return -1;
	}

	LLVMValueRef func;
	if (LLVMFindFunction(engine, funcname, &func)) {
		std::cerr << "cannot find function " << funcname << " "
				<< (error ? error : "") << std::endl;
		return -1;
	}

	if (NULL == LLVMRunFunction(engine, func, 0, NULL)) {
		std::cerr << "cannot execure function " << funcname << std::endl;
	}

	LLVMContextDispose(cxt);

	return 0;
}
