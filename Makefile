LLVM_HOME=/opt/llvm
LLVM_CONF=$(LLVM_HOME)/bin/llvm-config
CXXFLAGS=-g -O0 -fno-rtti -std=c++0x -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I$(LLVM_HOME)/include
LDFLAGS=-L$(LLVM_HOME)/lib -lz -lpthread -lcurses -lm
LIBS=`$(LLVM_CONF) --libs`
CLANG_LIBS=-lclangAST -lclangAnalysis -lclangBasic -lclangCodeGen -lclangDriver -lclangEdit -lclangFrontend -lclangLex -lclangParse -lclangSema -lclangSerialization -lclangTooling
TARGET=demo.bt demo.ll demo.bt.cpp load_exec generate_c_exec generate_ir_exec generate_load_exec
CLANG=$(LLVM_HOME)/bin/clang
CLANGPP=clang++
LLVM_DIS=$(LLVM_HOME)/bin/llvm-dis
LLC=$(LLVM_HOME)/bin/llc

all:$(TARGET) 

demo.bt: demo.c
	$(CLANG) -c -emit-llvm -O0 -g $< -o $@
	
demo.ll: demo.bt
	$(LLVM_DIS) -o $@ $<

demo.bt.cpp: demo.bt
	$(LLC) -march=cpp $<

load_exec: load_exec.cpp demo.bt
	$(CLANGPP) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o $@ $<

generate_c_exec: generate_c_exec.cpp 
	$(CLANGPP) $(CXXFLAGS) $(LDFLAGS) $(LIBS) $(CLANG_LIBS) -o $@ $<

generate_ir_exec: generate_ir_exec.cpp
	$(CLANGPP) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o $@ $<

generate_load_exec: generate_load_exec.cpp demo.bt
	$(CLANGPP) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o $@ $<

clean:
	rm -rf *.o *.ll *.dSYM $(TARGET)
