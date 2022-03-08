#include<iostream>
#include<fstream>
#include<string>
using namespace std;

#include "Compiler.h"

Compiler :: Compiler(string inPath, string outPath1, string outPath2, string outPath3, string outPath4, string outPath5) {
    this->curWord = "";  
    this->curSym = 0;  

    this->curLine = "";
    this->curChar = 0;    
    this->curLineNum = 0;    
    this->curLineLength = 0;
    this->curIndex = 0;
    this->curExpr = "";
    this->curFunctionName = "";
    this->curFuncSize = 0;

    this->firstWord = "";
    this->firstSym = 0;
    this->firstLineNum = 0;
    this->secondWord = "";
    this->thirdWord = "";

    this->curError = 0;
    this->curLev = 0;

    this->hasReturn = 0;

    this->isParam = 0;
    this->isLoop = 0;

    this->curStrNum = 0;

    this->sp_top = -1;
    this->global_top = 0;

    this->reg_num = 16;
    initialize_regs();
    this->and_num = 0;
    this->or_num = 0;
    this->if_num = -1;
    this->while_num = -1;
    this->tmpVarIdx = 0;

    infile.open(inPath);
    outfile1.open(outPath1);
    outfile2.open(outPath2);
    outfile3.open(outPath3);
    outfile4.open(outPath4);
    outfile5.open(outPath5);

    indexTab.push_back(0);
    levStack.push(0);
}

void Compiler :: Process() {
    /*
    while(true) {
        if (getSym(0) == -1) {
            break;
        }
    }
    */
    
    //getSym();
    while (!infile.eof()) {
        getline(infile, curLine);
        curLine += '\n';
        code.insert(code.end(), curLine);
    }

    curLineNum = 1;
    if (code.size() != 0) {
        curLine = code[0];
        curLineLength = curLine.length();
    }

    CompUnit();

    //输出中间代码
    for (int i = 0; i < midCodes.size(); ++i) {
        vector<string> tmp = midCodes[i].toString();
        for (int j = 0; j < tmp.size(); ++j) {
            outfile3 << tmp[j] << endl;
        }
    }

    //生成mips
    gen_MIPS();

    infile.close();
    outfile1.close();
    outfile2.close();
    outfile3.close();
    outfile4.close();
    outfile5.close();
}

void Compiler::initialize_regs() {
    while (!regs.empty()) {
        regs.pop();
    }
    for (int i = 0; i <= 9; ++i) {
        regs.push("t" + to_string(i));
    }
    for (int i = 2; i <= 7; ++i) {
        regs.push("s" + to_string(i));
    }
}