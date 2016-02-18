#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm-c/IRReader.h"
#include "llvm-c/Target.h"
#include "llvm-c/Linker.h"

#include <iostream>
#include <iostream>
#include <cstdlib>
#include <vector>

LLVMModuleRef GenerateModule() {
	const char * moduleName = "test generate ir";
	const char * moduleFile = "dumped_generate_load_exec.ll";
	const char * printMsg = "This is dynamically created function";

	LLVMContextRef cxt;
	LLVMModuleRef module;
	LLVMBasicBlockRef block;
	LLVMBuilderRef builder;
	char * error = NULL;

	LLVMInitializeNativeTarget();
	LLVMLinkInJIT();

	cxt = LLVMGetGlobalContext();

	/**
	 * create a module
	 */
	module = LLVMModuleCreateWithName(moduleName);

	/**
	 * create types
	 */
	LLVMTypeRef charType = LLVMInt8TypeInContext(cxt);
	LLVMTypeRef int32Type = LLVMInt32TypeInContext(cxt);
	LLVMTypeRef charPointType = LLVMPointerType(charType, 0);
	LLVMTypeRef voidType = LLVMVoidTypeInContext(cxt);

	std::vector<LLVMTypeRef> inputs;
	LLVMTypeRef funcTypeGetMsg = LLVMFunctionType(charPointType, &inputs[0],
			inputs.size(), 0);

	/**
	 * create a function
	 */
	LLVMValueRef funcGetMsg = LLVMAddFunction(module, "GetMsg", funcTypeGetMsg);

	/**
	 * create a basic block in the function GetMsg
	 */
	block = LLVMAppendBasicBlock(funcGetMsg, "");

	/**
	 * create block builder for the basic block
	 */
	builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(builder, block);

	/**
	 * define function GetMsg
	 */
	LLVMValueRef strVal = LLVMBuildGlobalStringPtr(builder, printMsg, "");
	LLVMBuildRet(builder, strVal);
	LLVMDisposeBuilder(builder);

	/**
	 * dump module to file
	 */
	if (LLVMPrintModuleToFile(module, moduleFile, &error)) {
		std::cerr << "cannot dump module to file " << (error ? error : "")
				<< std::endl;
		exit(1);
	}

	return module;
}

int main() {
	const char * path = "demo.bt";
	const char * funcname = "PrintMsg";
	LLVMContextRef cxt;
	LLVMMemoryBufferRef buffer;
	LLVMModuleRef precompiledModule;
	char * error = NULL;

	LLVMInitializeNativeTarget();
	LLVMLinkInJIT();

	cxt = LLVMGetGlobalContext();

	if (LLVMCreateMemoryBufferWithContentsOfFile(path, &buffer, &error)) {
		std::cerr << "cannot read IR bytecode from file " << path << " "
				<< (error ? error : "") << std::endl;
		return -1;
	}

	if (LLVMParseIRInContext(cxt, buffer, &precompiledModule, &error)) {
		std::cerr << "cannot parse IR bytecode " << (error ? error : "")
				<< std::endl;
		return -1;
	}

	/**
	 * remove default GetMsg
	 */
	LLVMValueRef func = LLVMGetNamedFunction(precompiledModule, "GetMsg");
	if (func) {
		LLVMDeleteFunction(func);
	}

	LLVMModuleRef dynamicallyGeneratedModule = GenerateModule();

	/**
	 * Link two modules
	 */
	if (LLVMLinkModules(precompiledModule, dynamicallyGeneratedModule,
			LLVMLinkerPreserveSource, &error)) {
		std::cerr << "cannot link two module " << (error ? error : "")
				<< std::endl;
		exit(1);
	}

	/**
	 * dump module to file
	 */
	if (LLVMPrintModuleToFile(precompiledModule,
			"dumped_generate_load_exec_link.ll", &error)) {
		std::cerr << "cannot dump module to file " << (error ? error : "")
				<< std::endl;
		exit(1);
	}

	LLVMExecutionEngineRef engine;

	if (LLVMCreateJITCompilerForModule(&engine, precompiledModule, 0, &error)) {
		std::cerr << "cannot create execution engine " << (error ? error : "")
				<< std::endl;
		return -1;
	}

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
