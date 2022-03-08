#include "Compiler.h"

int Compiler::checkFormatString() {
    int count = 0;
    for (int i = 0; i < curWord.size(); ++i) {
        char c = curWord.at(i);
        if (i == 0 || i == curWord.size() - 1) {
            if (c != '\"') {
                curError = ILLEGALSYMBOLERROR;
                errorOutput();
            }
        } else {
            if (c != 32 && c != 33 && !(c >= 40 && c <= 126)) {
                if (i < curWord.size() - 2 && c == '%' && curWord.at(i + 1) == 'd') {
                    ++count;
                } else {
                    curError = ILLEGALSYMBOLERROR;
                    errorOutput();
                }
            }
        }
    }
    return count;
}

int Compiler::checkRedefine(string name, int targetLev) {
    //返回-1代表发现重定义错误, 返回0代表没有重定义错误
    for (int i = indexTab[targetLev]; i < symbolTab.size(); i++) {
        if (symbolTab[i].name == name && symbolTab[i].isValid == 1) {
            return -1;
        }
    }
    return 0;
}

int Compiler::checkUndefine() {
    for (int i = symbolTab.size() - 1; i >= 0; i--) {
        if (symbolTab[i].name == curWord && symbolTab[i].isValid == 1) {
            return 0;   //找到定义
        }
    }
    return -1;  //未定义
}

//查找标识符并返回其类型
void Compiler::findIdentType(ParamType &pt) {
    int i = 0;
    pt.name = curExpr;
    pt.isConst = 0;
    string identName;
    for (int i = 0; i < symbolTab.size(); i++) {
        if (symbolTab[i].name == pt.name && symbolTab[i].type == FUNCTION) {
            pt.dimension = symbolTab[i].dimension;
            pt.type = FUNCTION;
            return;
        }
    }

    while (i < curExpr.size()) {
        char c = curExpr.at(i);
        if (isdigit(c)) {
            pt.type = INT;
            pt.dimension = 0;
            break;
        }
        else if (isLetter(c)) {
            while (isLetter(c) || isdigit(c)) {
                identName += c;
                if (i >= curExpr.size() - 1) {
                    break;
                }
                c = curExpr.at(++i);
            }
            for (int j = symbolTab.size() - 1; j >= 0; j--) {
                Symbol symbol = symbolTab[j];
                if (symbol.name == identName && symbol.isValid == 1) {
                    pt.isConst = symbol.isConst;
                    if (symbol.type == INT) {
                        pt.type = INT;
                        pt.dimension = symbol.dimension;
                        while (i < curExpr.size())
                        {
                            if (curExpr.at(i++) == '[') {
                                pt.dimension--;
                            }
                        }
                    }
                    else if (symbol.type == FUNCTION) {
                        pt.type = symbol.returnType;
                        pt.dimension = symbol.returnType == INT ? 0 : (symbol.returnType == VOID ? 0 : 0);
                    }
                    break;
                }
            }
            break;
        }
        ++i;
    }
    
}

int Compiler::findFunction(string name, Symbol &symbol) {
    //1代表找到, -1代表没找到
    Symbol s;
    for (int i = indexTab[curLev] - 1; i >= 0; i--) {
        s = symbolTab[i];
        if (s.name == name && s.type == FUNCTION) {
            symbol = s;
            return 1;
        }
    }
    return -1;
}

int Compiler::checkParamUnmatch(vector<Symbol> paramList, vector<ParamType> paramType) {
    //0代表成功匹配, -1代表不匹配
    int i = 0;
    while (i < paramList.size() && i < paramType.size()) {
        if (paramList[i].type != paramType[i].type || paramList[i].dimension != paramType[i].dimension) {
            return -1;
        }
        ++i;
    }
    if (i < paramList.size() || i < paramType.size()) {
        return -1;
    }
    return 0;
}

void Compiler::errorOutput() {
    auto iter = errors.find(curError);
    outfile2 << curLineNum << ' ' << (*iter).second << endl;
}

void Compiler::error() {
    outfile1 << "Syntax Error!" << endl;
}

