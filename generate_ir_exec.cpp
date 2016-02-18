/**
 * Demonstrate how to generate IR bytecode dynamically and execute it.
 */

#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm-c/IRReader.h"
#include "llvm-c/Target.h"

#include <iostream>
#include <cstdlib>
#include <vector>

int main() {
	const char * funcname = "PrintMsg";
	const char * moduleName = "test generate ir";
	const char * moduleFile = "dumped_generate_ir_exec.ll";
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
	LLVMTypeRef funcTypePrintMsg = LLVMFunctionType(voidType, &inputs[0],
			inputs.size(), 0);
	LLVMTypeRef funcTypePuts = LLVMFunctionType(int32Type, &inputs[0],
			inputs.size(), 0);

	/**
	 * create a function
	 */
	LLVMValueRef funcGetMsg = LLVMAddFunction(module, "GetMsg", funcTypeGetMsg);
	LLVMValueRef funcPrintMsg = LLVMAddFunction(module, "PrintMsg",
			funcTypePrintMsg);
	LLVMValueRef funcPuts = LLVMAddFunction(module, "puts", funcTypePuts);

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
	 * create a basic block in the function PrintMsg
	 */
	block = LLVMAppendBasicBlock(funcPrintMsg, "");

	/**
	 * create block builder for the basic block
	 */
	builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(builder, block);

	std::vector<LLVMValueRef> args;
	LLVMValueRef msg = LLVMBuildCall(builder, funcGetMsg, &args[0], 0, "");
	args.push_back(msg);
	LLVMBuildCall(builder, funcPuts, &args[0], 1, "");
	LLVMBuildRetVoid(builder);
	LLVMDisposeBuilder(builder);

	/**
	 * dump module to file
	 */
	if (LLVMPrintModuleToFile(module, moduleFile, &error)) {
		std::cerr << "cannot dump module to file " << (error ? error : "")
				<< std::endl;
		return -1;
	}

	LLVMExecutionEngineRef engine;

	if (LLVMCreateJITCompilerForModule(&engine, module, 0, &error)) {
		std::cerr << "cannot create execution engine " << (error ? error : "")
				<< std::endl;
		return -1;
	}

	if (NULL == LLVMRunFunction(engine, funcPrintMsg, 0, NULL)) {
		std::cerr << "cannot execure function " << funcname << std::endl;
	}

	LLVMContextDispose(cxt);

	return 0;
}
