#pragma once
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/CFG.h"

// include STL
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <string>

#define NO_ALLOC 0
#define ALLOCATED 1
#define FREED 2

#define UNANALYZED 1
#define IN_PROGRESS 2
#define ANALYZED 3
// #define ALLOC 1
// #define FREE 2

using namespace llvm;
using namespace std;
