#include<iostream>
#include<fstream>
#include<stdlib.h>

#include "Compiler.h"

using namespace std;

void Compiler::CompUnit() {
    while (true) {
        preRead(3);
        if (firstWord == "const") {
            ConstDecl();
        } else if (firstWord == "int" && thirdWord != "(") {
            VarDecl();
        } else {
            break;
        }
    }

    while (true) {
        preRead(2);
        if (firstWord == "int" && secondWord == "main") {
            break;
        }
        FuncDef();
    }

    MainFuncDef();

    outfile1 << "<CompUnit>" << endl;
}

/**
 * 常量声明
 * ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
 * 1.花括号内重复0次 2.花括号内重复多次
 */
void Compiler::ConstDecl() {
    getSym(0);   //读到const
    getSym(0);   //读到int
    if (curWord == "int") {
        ConstDef();
    } else {
        error();
    }
    while (true) {
        //无需预读
        preRead(1);
        if (firstWord == ",") {
            getSym(0);
            ConstDef();
        } else if (firstWord == ";") {
            getSym(0);
            break;
        } else {
            curError = LACKSEMICERROR;
            errorOutput();
            break;
        }
    }
    outfile1 << "<ConstDecl>" << endl;
}

/**
 * ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal
 * 包含普通变量、⼀维数组、⼆维数组共三种情况
 */
void Compiler::ConstDef() {
    Symbol symbol;
    int dimension = 0;
    getSym(0);   //得到常量名称Ident
    symbol.name = curWord;
    while (true) {
        if (preRead(1) == -1) {
            error();
        }
        if (firstWord == "[") {
            getSym(0);
            ++dimension;
            curExpr.clear();
            Operand len = ConstExp()[0];
            symbol.di_len.push_back(len.value);
            if (preRead(1) == -1) {
                error();
            }
            if (firstWord != "]") {
                //缺少右中括号
                curError = LACKRBRACKERROR;
                errorOutput();
            } else {
                getSym(0);
            }
        } else if (firstWord == "=") {
            break;
        } else {
            error();
        }
    }

    symbol.isConst = 1;
    symbol.type = INT;
    symbol.isPara = 0;
    symbol.dimension = dimension;
    symbol.lev = curLev;
    symbol.isValid = 1;
    if (dimension == 0) {
        ++curFuncSize;
    } else if (dimension == 1) {
        curFuncSize += symbol.di_len[0];
    } else if (dimension == 2) {
        curFuncSize += symbol.di_len[0] * symbol.di_len[1];
    }

    /**
     * ErrorHandle
     */
    if (checkRedefine(symbol.name, curLev) == -1) {
        curError = REDEFINEERROR;
        errorOutput();
    } else if (curLev != 0) {
        Symbol functionSymbol;
        findFunction(curFunctionName, functionSymbol);
        vector<Symbol> paramList = functionSymbol.paramList;
        for (int i = 0; i < paramList.size(); i++) {
            if (paramList[i].name == symbol.name) {
                curError = REDEFINEERROR;
                errorOutput();
            }
        }
    }
    //生成常量声明的中间代码
    midCodes.push_back(Quaternary("const", Operand(symbol)));

    //ConstInitVal包含普通变量、⼀维数组、⼆维数组共三种情况
    getSym(0);   //读等号
    vector<vector<Operand>> values;    //用来记录给常量赋的初值
    vector<Operand> tmp;
    ConstInitVal(values, tmp);

    //生成中间代码
    if (values.size() == 0) {
        if (tmp.size() == 0) {
            tmp.push_back(Operand(0));
        }
        values.push_back(tmp);
    }
    defMidCode(symbol, values, tmp);

    for (int i = 0; i < values.size(); i++) {
        vector<Operand> line = values[i];
        for (int j = 0; j < line.size(); j++) {
            cout << line[j].value << ' ';
        }
        cout << endl;
    }

    symbolTab.push_back(symbol);    //当如符号表
    outfile1 << "<ConstDef>" << endl;

    initialize_regs();
}

void Compiler::ConstInitVal(vector<vector<Operand>> &res, vector<Operand> &tmp) {
    if (preRead(1) == -1) {
        error();
    }

    if (firstWord == "{") {
        getSym(0);
        preRead(1);
        if (firstWord == "}") {
            getSym(0);
            res.push_back(tmp);
        } else {
            ConstInitVal(res, tmp);
            while (true) {
                preRead(1);
                if (firstWord == ",") {
                    getSym(0);
                    ConstInitVal(res, tmp);
                } else if (firstWord == "}") {
                    if (tmp.size() != 0) {
                        res.push_back(tmp);
                        tmp.clear();
                    }
                    getSym(0);
                    break;
                } else {
                    error();
                }
            }
        }

    } else {
        vector<Operand> op1_vec = ConstExp();
        ParamType pt;
        findIdentType(pt);
        tmp.insert(tmp.end(), op1_vec.begin(), op1_vec.end());
    }

    outfile1 << "<ConstInitVal>" << endl;
}

void Compiler::VarDecl() {
    getSym(0);  //读到int
    VarDef();
    while (true) {
        if (preRead(1) == -1) {
            error();
            break;
        }
        if (firstWord == ",") {
            getSym(0);
            VarDef();
        } else if (firstWord == ";") {
            getSym(0);
            break;
        } else {
            curError = LACKSEMICERROR;
            errorOutput();
            break;
        }
    }

    outfile1 << "<VarDecl>" << endl;
}

void Compiler::VarDef() {
    Symbol symbol;
    int dimension = 0;

    preRead(1);
    if (firstSym == IDENT) {
        getSym(0);
        symbol.name = curWord;
        while (true) {
            preRead(1);
            if (firstWord == "[") {
                getSym(0);
                ++dimension;
                vector<Operand> len = ConstExp();
                symbol.di_len.push_back(len[0].value);
                preRead(1);
                if (firstWord == "]") {
                    getSym(0);
                } else {
                    curError = LACKRBRACKERROR;
                    errorOutput();
                }
            } else {
                break;
            }
        }

        symbol.isConst = 0;
        symbol.type = INT;
        symbol.isPara = 0;
        symbol.dimension = dimension;
        symbol.lev = curLev;
        symbol.isValid = 1;
        if (dimension == 0) {
            ++curFuncSize;
        } else if (dimension == 1) {
            curFuncSize += symbol.di_len[0];
        } else if (dimension == 2) {
            curFuncSize += symbol.di_len[0] * symbol.di_len[1];
        }

        if (checkRedefine(symbol.name, curLev) == -1) {
            curError = REDEFINEERROR;
            errorOutput();
        } else if (curLev != 0) {
            Symbol functionSymbol;
            findFunction(curFunctionName, functionSymbol);
            vector<Symbol> paramList = functionSymbol.paramList;
            for (int i = 0; i < paramList.size(); i++) {
                if (paramList[i].name == symbol.name) {
                    curError = REDEFINEERROR;
                    errorOutput();
                }
            }
        }

        /**
         * MidCodes
         */
        midCodes.push_back(Quaternary("var", Operand(symbol)));

        if (firstWord == "=") {
            getSym(0);
            vector<vector<Operand>> values;    //用来记录给常量赋的初值
            vector<Operand> tmp;
            InitVal(values, tmp);

            if (values.size() == 0) {
                if (tmp.size() == 0) {
                    tmp.push_back(Operand(0));
                }
                values.push_back(tmp);
            }

            //生成中间代码
            defMidCode(symbol, values, tmp);
        } else if (curLev == 0) {
            //如果全局变量未赋初值，需要手动赋0
            if (symbol.dimension > 0) {
                vector<vector<Operand>> values(symbol.di_len[0]);    //用来记录给常量赋的初值
                defMidCode(symbol, values, vector<Operand>(1, Operand(0)));
            }
            else {
                symbol.value = 0;
                midCodes.push_back(Quaternary("=", Operand(0), Operand(symbol)));
            }
        }

    } else {
        error();
    }

    symbolTab.push_back(symbol);
    outfile1 << "<VarDef>" << endl;

    initialize_regs();
}

void Compiler::InitVal(vector<vector<Operand>> &res, vector<Operand> &tmp) {
    if (preRead(1) == -1) {
        error();
    }

    if (firstWord == "{") {
        getSym(0);
        preRead(1);
        if (firstWord == "}") {
            getSym(0);
            res.push_back(tmp);
        } else {
            InitVal(res, tmp);
            while (true) {
                preRead(1);
                if (firstWord == ",") {
                    getSym(0);
                    InitVal(res, tmp);
                } else if (firstWord == "}") {
                    if (tmp.size() != 0) {
                        res.push_back(tmp);
                        tmp.clear();
                    }
                    getSym(0);
                    break;
                } else {
                    error();
                }
            }
        }
    } else {
        vector<Operand> op1_vec = Exp();
        tmp.insert(tmp.end(), op1_vec.begin(), op1_vec.end());
    }
    outfile1 << "<InitVal>" << endl;
}

void Compiler::FuncDef() {
    curFuncSize = 0;
    hasReturn = 0;
    Symbol symbol;
    vector<Symbol> paramList;
    FuncType();
    string retType = curWord;
    symbol.returnType = curWord == "void" ? VOID : (curWord == "int" ? INT : 0);

    indexTab.push_back(symbolTab.size());

    preRead(1);
    if (firstSym == IDENT) {
        getSym(0);  //读Ident
        curFunctionName = curWord;
        symbol.name = curWord;
        symbol.lev = ++curLev;
        symbol.isConst = 0;
        symbol.type = FUNCTION;
        symbol.isPara = 0;
        symbol.dimension = 0;
        symbol.isValid = 1;
        symbol.value = 0;

        //检查重定义错误
        if (checkRedefine(symbol.name, 0) == -1) {
            curError = REDEFINEERROR;
            errorOutput();
        }

        getSym(0);  //读左括号
        if (curWord == "(") {
            preRead(1);
            if (firstWord == ")") {
                getSym(0);
            } else {
                FuncFParams(paramList);
                symbol.paramList = paramList;
                preRead(1);
                if (firstWord == ")") {
                    getSym(0);
                } else {
                    curError = LACKRPARENTERROR;
                    errorOutput();
                }
            }
            //将函数写入符号表
            int index = symbolTab.size() - paramList.size() + 1;
            symbolTab.insert(symbolTab.end() - paramList.size(), symbol);
            Quaternary funcDef(retType, Operand(symbol));
            midCodes.insert(midCodes.end() - paramList.size(), funcDef);
            levStack.push(curLev);

            Block();
            //curLev = tmpLev + 1;
            if (symbol.returnType != VOID && hasReturn == 0) {
                curError = LACKRETURNERROR;
                errorOutput();
            }

            //将当前层级的参数的isValid变为0
            if (paramList.size() > 0) {
                for (int i = 0; i < paramList.size(); i++) {
                    symbolTab[index + i].isValid = 0;
                }
            }
            //symbolTab.erase(symbolTab.begin() + indexTab[tmpLev], symbolTab.end());

        } else {
            error();
        }
    } else {
        error();
    }

    midCodes.push_back(Quaternary("end_func", symbol));
    outfile1 << "<FuncDef>" << endl;

    initialize_regs();
}

void Compiler::MainFuncDef() {
    hasReturn = 0;
    getSym(0);
    if (curWord != "int") {
        error();
    }
    getSym(0);
    if (curWord != "main") {
        error();
    }
    getSym(0);
    if (curWord != "(") {
        error();
    }
    preRead(1);
    if (firstWord == ")") {
        getSym(0);
    } else {
        curError = LACKRPARENTERROR;
        errorOutput();
    }
    curFunctionName = "main";
    midCodes.push_back(Quaternary("main"));
    Block();

    if (hasReturn == 0) {
        curError = LACKRETURNERROR;
        errorOutput();
    }
    outfile1 << "<MainFuncDef>" << endl;
}

void Compiler::FuncType() {
    getSym(0);
    if (curWord != "void" && curWord != "int") {
        error();
    }

    outfile1 << "<FuncType>" << endl;
}

void Compiler::FuncFParams(vector<Symbol> &paramList) {
    isParam = 1;
    Symbol symbol;
    FuncFParam(symbol);
    isParam = 0;
    if (checkRedefine(symbol.name, curLev)) {
        curError = REDEFINEERROR;
        errorOutput();
    }
    paramList.push_back(symbol);
    symbolTab.push_back(symbol);

    while (true) {
        preRead(1);
        if (firstWord == ",") {
            getSym(0);
            isParam = 1;
            FuncFParam(symbol);
            isParam = 0;
            if (checkRedefine(symbol.name, curLev)) {
                curError = REDEFINEERROR;
                errorOutput();
            }
            paramList.push_back(symbol);
            symbolTab.push_back(symbol);

        } else {
            break;
        }
    }

    outfile1 << "<FuncFParams>" << endl;
}

void Compiler::FuncFParam(Symbol &symbol) {
    symbol.di_len.clear();
    int dimension = 0;

    preRead(1);
    if (firstWord == "int") {
        getSym(0);
        symbol.type = INT;
        preRead(1);
        if (firstSym == IDENT) {
            getSym(0);
            symbol.name = curWord;
            preRead(1);
            if (firstWord == "[") {
                getSym(0);
                ++dimension;
                //第一维长度为空
                symbol.di_len.push_back(INT_MAX);
                preRead(1);  //预读右括号
                if (firstWord == "]") {
                    getSym(0);
                } else {
                    curError = LACKRBRACKERROR;
                    errorOutput();
                }
                while (true) {
                    preRead(1);
                    if (firstWord == "[") {
                        getSym(0);
                        ++dimension;
                        vector<Operand> operand = ConstExp();
                        symbol.di_len.push_back(operand[0].value);
                        preRead(1);  //预读右括号
                        if (firstWord == "]") {
                            getSym(0);
                        } else {
                            curError = LACKRBRACKERROR;
                            errorOutput();
                        }
                    } else {
                        break;
                    }
                }
            }
        } else {
            error();
        }
    } else {
        error();
    }

    symbol.type = INT;
    symbol.isPara = 1;
    symbol.dimension = dimension;
    symbol.lev = curLev + 1;
    symbol.isValid = 1;
    symbol.isConst = 0;
    symbol.value = 0;

    //生成中间代码
    midCodes.push_back(Quaternary("para", Operand(symbol)));

    outfile1 << "<FuncFParam>" << endl;
}

void Compiler::Block() {
    ++curLev;
    /*
    if (curLev > tmpLev) {
        tmpLev = curLev;
    }
    */
    //hasReturn = 0;
    indexTab.push_back(symbolTab.size());
    levStack.push(curLev);
    midCodes.push_back(Quaternary("Block"));

    getSym(0);
    if (curWord != "{") {
        error();
    }

    while (true) {
        preRead(1);
        if (firstWord == "}") {
            getSym(0);
            midCodes.push_back(Quaternary("EndBlock"));
            int tmpLev = levStack.top();
            levStack.pop();
            if (symbolTab.size() > indexTab[tmpLev]) {
                for (int i = symbolTab.size() - 1; i >= indexTab[tmpLev]; --i) {
                    symbolTab[i].isValid = 0;
                }
            }

            //--curLev;
            break;
        }
        BlockItem();
    }

    outfile1 << "<Block>" << endl;

    initialize_regs();
}

void Compiler::BlockItem() {
    preRead(1);
    if (firstWord == "const") {
        ConstDecl();
    } else if (firstWord == "int") {
        VarDecl();
    } else {
        Stmt();
    }
}

int Compiler::hasEqual() {
    preRead(1);
    int tmpLineNum = firstLineNum;
    string tmpLine = code[tmpLineNum - 1];

    int tmpIndex = 0;
    int flag = 0;
    while (true) {
        if (tmpIndex >= tmpLine.length()) {
            break;
        }
        if (tmpLine.at(tmpIndex) == ';') {
            break;
        }
        if (tmpLine.at(tmpIndex) == '=') {
            flag = 1;
            break;
        }
        ++tmpIndex;
    }
    return flag;

}

void Compiler::Stmt() {
    curExpr.clear();
    preRead(2);
    if (firstSym == IDENT && secondWord != "(" && hasEqual() == 1) {
        LVal();
        vector<Operand> op1_vec;
        vector<Operand> op3_vec;
        Quaternary q = midCodes[midCodes.size() - 1];
        if (q.opcode == "=") {
            op3_vec = q.op1_vec;
        } else if (q.opcode == "=[]") {
            op3_vec = q.op1_vec;
            op1_vec = q.op2_vec;
        }
        //如果变量是左值，没必要把变量的旧值赋给它
        midCodes.erase(midCodes.end() - 1);
        //--curTmpNum;

        //如果LVal是const类型，则不能给它赋值
        ParamType pt;
        findIdentType(pt);
        if (pt.isConst == 1) {
            curError = CHANGECONSTERROR;
            errorOutput();
        }
        curExpr.clear();
        getSym(0);
        if (curWord == "=") {
            preRead(1);
            if (firstWord == "getint") {
                getSym(0);
                preRead(1);
                if (firstWord == "(") {
                    getSym(0);
                } else {
                    error();
                }
                preRead(1);
                if (firstWord == ")") {
                    getSym(0);
                } else {
                    curError = LACKRPARENTERROR;
                    errorOutput();
                }

                //生成read的中间代码
                Operand right = getNewReg();
                Quaternary read("read", right);
                midCodes.push_back(read);
                if (q.opcode == "=") {
                    Quaternary assign("=", right, op3_vec[0]);
                    midCodes.push_back(assign);
                } else if (q.opcode == "=[]") {
                    Quaternary assign("[]=", op1_vec[0], right, op3_vec[0]);
                    midCodes.push_back(assign);
                }
            } else {
                vector<Operand> right = Exp();
                if (q.opcode == "=") {

                    Quaternary assign("=", right[0], op3_vec[0]);
                    midCodes.push_back(assign);
                } else if (q.opcode == "=[]") {

                    Quaternary assign("[]=", op1_vec, right, op3_vec);
                    midCodes.push_back(assign);
                }
                curExpr.clear();
            }
            preRead(1);
            if (firstWord == ";") {
                getSym(0);
            } else {
                curError = LACKSEMICERROR;
                errorOutput();
            }
        }
    } else if (firstWord == "{") {
        Block();
    } else if (firstWord == "if") {
        getSym(0);
        preRead(1);
        if (firstWord == "(") {
            getSym(0);
            vector<Operand> op1_vec = Cond();

            Quaternary branch("beqz", op1_vec[0], Operand("if_else_" + to_string(++if_num)));
            midCodes.push_back(branch);
            ifStack.push(if_num);

            preRead(1);
            if (firstWord == ")") {
                getSym(0);
                Stmt();
                preRead(1);

                Quaternary jump("jump", Operand("if_end_" + to_string(ifStack.top())));
                midCodes.push_back(jump);
                Quaternary else_label("label", Operand("if_else_" + to_string(ifStack.top())));
                midCodes.push_back(else_label);
                if (firstWord == "else") {
                    getSym(0);
                    Stmt();
                }
                Quaternary end_label("label", Operand("if_end_" + to_string(ifStack.top())));
                midCodes.push_back(end_label);
                ifStack.pop();
            } else {
                curError = LACKRPARENTERROR;
                errorOutput();
            }
        } else {
            error();
        }
    } else if (firstWord == "while") {
        isLoop = 1;
        getSym(0);
        preRead(1);
        if (firstWord == "(") {
            getSym(0);

            Quaternary begin_label("label", Operand("while_begin_" + to_string(++while_num)));
            midCodes.push_back(begin_label);
            whileStack.push(while_num);

            vector<Operand> op1_vec = Cond();
            Quaternary branch("beqz", op1_vec[0], Operand("while_end_" + to_string(while_num)));
            midCodes.push_back(branch);

            preRead(1);
            if (firstWord == ")") {
                getSym(0);
                Stmt();

                isLoop = 0;
                Quaternary jump("jump", Operand("while_begin_" + to_string(whileStack.top())));
                midCodes.push_back(jump);
                Quaternary else_label("label", Operand("while_end_" + to_string(whileStack.top())));
                midCodes.push_back(else_label);
                whileStack.pop();

            } else {
                curError = LACKRPARENTERROR;
                errorOutput();
            }
        } else {
            error();
        }
    } else if (firstWord == "break") {
        getSym(0);
        if (isLoop == 0) {
            curError = BREAKORCONTINUEERROR;
            errorOutput();
        }
        preRead(1);
        if (firstWord == ";") {
            getSym(0);
        } else {
            curError = LACKSEMICERROR;
            errorOutput();
        }

        Quaternary jump("jump", Operand("while_end_" + to_string(whileStack.top())));
        midCodes.push_back(jump);

    } else if (firstWord == "continue") {
        getSym(0);
        if (isLoop == 0) {
            curError = BREAKORCONTINUEERROR;
            errorOutput();
        }
        preRead(1);
        if (firstWord == ";") {
            getSym(0);
        } else {
            curError = LACKSEMICERROR;
            errorOutput();
        }

        Quaternary jump("jump", Operand("while_begin_" + to_string(whileStack.top())));
        midCodes.push_back(jump);

    } else if (firstWord == "return") {
        Symbol symbol;
        findFunction(curFunctionName, symbol);

        getSym(0);
        preRead(1);
        if (firstWord == ";") {
            getSym(0);
            midCodes.push_back(Quaternary("return"));
        } else {
            //说明有返回值，若该函数定义为void类型，则需报错
            if (symbol.returnType == VOID) {
                curError = UNMATCHRETURNERROR;
                errorOutput();
            }
            vector<Operand> op1_vec = Exp();
            midCodes.push_back(Quaternary("return", op1_vec[0]));
            curExpr.clear();
            preRead(1);
            if (firstWord == ";") {
                getSym(0);
            } else {
                curError = LACKSEMICERROR;
                errorOutput();
            }
        }
        hasReturn = 1;
    } else if (firstWord == "printf") {
        vector<Operand> nums;   //记录每个%d对应位置的Exp值
        vector<string> splits;      //splits用来存放用"%d"、"\n"切割完的字符串
        getSym(0);
        preRead(1);
        if (firstWord == "(") {
            getSym(0);
            preRead(1);
            if (firstSym == STRCON) {
                getSym(0);
                int d_count = checkFormatString();
                splitString(curWord, splits);

                int exp_count = 0;
                while (true) {
                    preRead(1);
                    if (firstWord == ",") {
                        getSym(0);
                        vector<Operand> num = Exp();
                        nums.push_back(num[0]);
                        curExpr.clear();
                        ++exp_count;
                    } else if (firstWord == ")") {
                        getSym(0);
                        break;
                    } else {
                        curError = LACKRPARENTERROR;
                        errorOutput();
                    }
                }
                if (d_count != exp_count) {
                    curError = UNMATCHEXPNUMERROR;
                    errorOutput();
                }
                preRead(1);
                if (firstWord == ";") {
                    getSym(0);
                } else {
                    curError = LACKSEMICERROR;
                    errorOutput();
                }
            } else {
                error();
            }
        } else {
            error();
        }
        long count = 0;
        for (int i = 0; i < splits.size(); ++i) {
            if (splits[i] == "%d") {
                Quaternary write("write", nums[count++]);
                midCodes.push_back(write);
            } else if (splits[i] == "\\n") {
                midCodes.push_back(Quaternary("LineBreak"));
            } else {
                Quaternary write("write", Operand("$STRING" + to_string(curStrNum++), splits[i]));
                midCodes.push_back(write);
                global_strings.push_back(write);
                //global_vars.push_back(write);
            }
        }
    } else if (firstWord == ";") {
        getSym(0);
    } else {
        Exp();
        curExpr.clear();
        preRead(1);
        if (firstWord == ";") {
            getSym(0);
        } else {
            curError = LACKSEMICERROR;
            errorOutput();
        }
    }

    outfile1 << "<Stmt>" << endl;
    initialize_regs();
}

vector<Operand> Compiler::Cond() {
    vector<Operand> op1_vec = LOrExp();
    initialize_regs();

    outfile1 << "<Cond>" << endl;
    return op1_vec;
}

vector<Operand> Compiler::LOrExp() {
    vector<Operand> op1_vec = LAndExp();
    Quaternary bnez("bgtz", op1_vec[0], Operand("or_success_" + to_string(or_num)));
    midCodes.push_back(bnez);

    outfile1 << "<LOrExp>" << endl;
    while (true) {
        preRead(1);
        if (firstWord == "||") {
            getSym(0);

            op1_vec = LAndExp();
            Quaternary bnez("bgtz", op1_vec[0], Operand("or_success_" + to_string(or_num)));
            midCodes.push_back(bnez);

            outfile1 << "<LOrExp>" << endl;
        } else {
            break;
        }
    }

    Quaternary jump("jump", Operand("or_fail_" + to_string(or_num)));
    midCodes.push_back(jump);
    //Generate label_success
    Quaternary label_success("label", Operand("or_success_" + to_string(or_num)));
    midCodes.push_back(label_success);
    Quaternary assign_one("=", Operand(1), op1_vec[0]);
    midCodes.push_back(assign_one);
    Quaternary label_fail("label", Operand("or_fail_" + to_string(or_num++)));
    midCodes.push_back(label_fail);

    //回收寄存器
    string reg1 = op1_vec[0].tokenName;
    //遍历queue查找寄存器是否已经在regs中
    bool hasReg = false;
    for (int j = 0; j < regs.size(); ++j) {
        if (regs.front() == reg1) {
            hasReg = true;
        }
        regs.push(regs.front());
        regs.pop();
    }
    if ((reg1[0] == 't' || reg1[0] == 's') && hasReg == false) {
        regs.push(reg1);
    }

    return op1_vec;
    //outfile << "<LOrExp>" << endl;
}

vector<Operand> Compiler::LAndExp() {
    vector<Operand> op1_vec = EqExp();
    /** branch if equal zero */
    if (op1_vec[0].tokenName != "") {
        Quaternary beqz("beqz", op1_vec[0], Operand("and_fail_" + to_string(and_num)));
        midCodes.push_back(beqz);
    } else {
        Operand tmp = getNewReg();
        Quaternary assign("=", op1_vec[0], tmp);
        midCodes.push_back(assign);
        Quaternary beqz("beqz", tmp, Operand("and_fail_" + to_string(and_num)));
        midCodes.push_back(beqz);
        op1_vec[0] = tmp;
    }

    outfile1 << "<LAndExp>" << endl;
    while (true) {
        preRead(1);
        if (firstWord == "&&") {
            getSym(0);

            op1_vec = EqExp();
            if (op1_vec[0].tokenName != "") {
                Quaternary beqz("beqz", op1_vec[0], Operand("and_fail_" + to_string(and_num)));
                midCodes.push_back(beqz);
            } else {
                Operand tmp = getNewReg();
                Quaternary assign("=", op1_vec[0], tmp);
                midCodes.push_back(assign);
                Quaternary beqz("beqz", tmp, Operand("and_fail_" + to_string(and_num)));
                midCodes.push_back(beqz);
                op1_vec[0] = tmp;
            }

            outfile1 << "<LAndExp>" << endl;
        } else {
            break;
        }
    }

    Quaternary jump("jump", Operand("and_success_" + to_string(and_num)));
    midCodes.push_back(jump);
    //Generate label
    Quaternary label_fail("label", Operand("and_fail_" + to_string(and_num)));
    midCodes.push_back(label_fail);
    Quaternary assign_zero("=", Operand(0), op1_vec[0]);
    midCodes.push_back(assign_zero);
    Quaternary label_success("label", Operand("and_success_" + to_string(and_num++)));
    midCodes.push_back(label_success);

    //回收寄存器
    string reg1 = op1_vec[0].tokenName;
    //遍历queue查找寄存器是否已经在regs中
    bool hasReg = false;
    for (int j = 0; j < regs.size(); ++j) {
        if (regs.front() == reg1) {
            hasReg = true;
        }
        regs.push(regs.front());
        regs.pop();
    }
    if ((reg1[0] == 't' || reg1[0] == 's') && hasReg == false) {
        regs.push(reg1);
    }

    return op1_vec;
    //outfile << "<LAndExp>" << endl;
}

vector<Operand> Compiler::EqExp() {
    vector<Operand> op1_vec = RelExp();
    outfile1 << "<EqExp>" << endl;
    while (true) {
        preRead(1);
        if (firstWord == "==" || firstWord == "!=") {
            getSym(0);
            string opcode = curWord;
            vector<Operand> op2_vec = RelExp();
            if (op1_vec[0].isValueKnown == true && op2_vec[0].isValueKnown == true) {
                long value = calculate(opcode, op1_vec[0], op2_vec[0]);
                Operand op3(value);
                op1_vec.clear();
                op1_vec.push_back(op3);
            } else {
                Operand op3 = getNewReg();
                Quaternary quaternary(opcode, op1_vec[0], op2_vec[0], op3);
                midCodes.push_back(quaternary);
                //回收寄存器
                string reg1 = op1_vec[0].tokenName;
                string reg2 = op2_vec[0].tokenName;
                //遍历queue查找寄存器是否已经在regs中
                bool hasReg = false;
                for (int j = 0; j < regs.size(); ++j) {
                    if (regs.front() == reg1) {
                        hasReg = true;
                    }
                    regs.push(regs.front());
                    regs.pop();
                }
                if ((reg1[0] == 't' || reg1[0] == 's') && hasReg == false) {
                    regs.push(reg1);
                }

                //遍历queue查找寄存器是否已经在regs中
                hasReg = false;
                for (int j = 0; j < regs.size(); ++j) {
                    if (regs.front() == reg2) {
                        hasReg = true;
                    }
                    regs.push(regs.front());
                    regs.pop();
                }
                if ((reg2[0] == 't' || reg2[0] == 's') && hasReg == false) {
                    regs.push(reg2);
                }
                op1_vec.clear();
                op1_vec.push_back(op3);
            }
            outfile1 << "<EqExp>" << endl;
        } else {
            break;
        }
    }

    return op1_vec;
    //outfile << "<EqExp>" << endl;
}

vector<Operand> Compiler::RelExp() {
    vector<Operand> op1_vec = AddExp();
    outfile1 << "<RelExp>" << endl;

    while (true) {
        preRead(1);
        if (firstWord == "<" || firstWord == ">" || firstWord == "<=" || firstWord == ">=") {
            getSym(0);
            string opcode = curWord;
            vector<Operand> op2_vec = AddExp();
            if (op1_vec[0].isValueKnown == true && op2_vec[0].isValueKnown == true) {
                long value = calculate(opcode, op1_vec[0], op2_vec[0]);
                Operand op3(value);
                op1_vec.clear();
                op1_vec.push_back(op3);
            } else {
                Operand op3 = getNewReg();
                Quaternary quaternary(opcode, op1_vec[0], op2_vec[0], op3);
                midCodes.push_back(quaternary);

                //回收寄存器
                string reg1 = op1_vec[0].tokenName;
                string reg2 = op2_vec[0].tokenName;
                //遍历queue查找寄存器是否已经在regs中
                bool hasReg = false;
                for (int j = 0; j < regs.size(); ++j) {
                    if (regs.front() == reg1) {
                        hasReg = true;
                    }
                    regs.push(regs.front());
                    regs.pop();
                }
                if ((reg1[0] == 't' || reg1[0] == 's') && hasReg == false) {
                    regs.push(reg1);
                }

                //遍历queue查找寄存器是否已经在regs中
                hasReg = false;
                for (int j = 0; j < regs.size(); ++j) {
                    if (regs.front() == reg2) {
                        hasReg = true;
                    }
                    regs.push(regs.front());
                    regs.pop();
                }
                if ((reg2[0] == 't' || reg2[0] == 's') && hasReg == false) {
                    regs.push(reg2);
                }
                op1_vec.clear();
                op1_vec.push_back(op3);
            }
            outfile1 << "<RelExp>" << endl;
        } else {
            break;
        }
    }

    return op1_vec;
    //outfile << "<RelExp>" << endl;
}

vector<Operand> Compiler::ConstExp() {
    vector<Operand> op1_vec = AddExp();

    /*
    Operand op3("t" + to_string(curTmpNum++));
    Quaternary quaternary("=", op1, op3);
    midCodes.push_back(quaternary);
    */

    outfile1 << "<ConstExp>" << endl;
    return op1_vec;
}

vector<Operand> Compiler::AddExp() {
    vector<Operand> op1_vec = MulExp();
    outfile1 << "<AddExp>" << endl;

    while (true) {
        preRead(1);
        if (firstWord == "+" || firstWord == "-") {
            getSym(0);
            curExpr += curWord;
            string opcode = curWord;
            vector<Operand> op2_vec = MulExp();
            if (op1_vec[0].isValueKnown == true && op2_vec[0].isValueKnown == true) {
                long value = calculate(opcode, op1_vec[0], op2_vec[0]);
                Operand op3(value);
                op1_vec.clear();
                op1_vec.push_back(op3);
            } else {
                Operand op3 = getNewReg();
                Quaternary quaternary(opcode, op1_vec[0], op2_vec[0], op3);
                midCodes.push_back(quaternary);
                //回收寄存器
                string reg1 = op1_vec[0].tokenName;
                string reg2 = op2_vec[0].tokenName;
                //遍历queue查找寄存器是否已经在regs中
                bool hasReg = false;
                for (int j = 0; j < regs.size(); ++j) {
                    if (regs.front() == reg1) {
                        hasReg = true;
                    }
                    regs.push(regs.front());
                    regs.pop();
                }
                if ((reg1[0] == 't' || reg1[0] == 's') && hasReg == false) {
                    regs.push(reg1);
                }

                //遍历queue查找寄存器是否已经在regs中
                hasReg = false;
                for (int j = 0; j < regs.size(); ++j) {
                    if (regs.front() == reg2) {
                        hasReg = true;
                    }
                    regs.push(regs.front());
                    regs.pop();
                }
                if ((reg2[0] == 't' || reg2[0] == 's') && hasReg == false) {
                    regs.push(reg2);
                }
                op1_vec.clear();
                op1_vec.push_back(op3);

            }
            outfile1 << "<AddExp>" << endl;
        } else {
            break;
        }
    }

    return op1_vec;
    //outfile << "<AddExp>" << endl;
}

vector<Operand> Compiler::MulExp() {
    vector<Operand> op1_vec = UnaryExp();
    outfile1 << "<MulExp>" << endl;

    while (true) {
        preRead(1);
        if (firstWord == "*" || firstWord == "/" || firstWord == "%") {
            getSym(0);
            curExpr += curWord;
            string opcode = curWord;
            vector<Operand> op2_vec = UnaryExp();
            if (op1_vec[0].isValueKnown == true && op2_vec[0].isValueKnown == true) {
                int value = calculate(opcode, op1_vec[0], op2_vec[0]);
                Operand op3(value);
                op1_vec.clear();
                op1_vec.push_back(op3);
            } else {
                Operand op3 = getNewReg();
                Quaternary quaternary(opcode, op1_vec[0], op2_vec[0], op3);
                midCodes.push_back(quaternary);
                //回收寄存器
                string reg1 = op1_vec[0].tokenName;
                string reg2 = op2_vec[0].tokenName;
                //遍历queue查找寄存器是否已经在regs中
                bool hasReg = false;
                for (int j = 0; j < regs.size(); ++j) {
                    if (regs.front() == reg1) {
                        hasReg = true;
                    }
                    regs.push(regs.front());
                    regs.pop();
                }
                if ((reg1[0] == 't' || reg1[0] == 's') && hasReg == false) {
                    regs.push(reg1);
                }

                //遍历queue查找寄存器是否已经在regs中
                hasReg = false;
                for (int j = 0; j < regs.size(); ++j) {
                    if (regs.front() == reg2) {
                        hasReg = true;
                    }
                    regs.push(regs.front());
                    regs.pop();
                }
                if ((reg2[0] == 't' || reg2[0] == 's') && hasReg == false) {
                    regs.push(reg2);
                }
                op1_vec.clear();
                op1_vec.push_back(op3);

            }
            outfile1 << "<MulExp>" << endl;
        } else {
            break;
        }
    }

    return op1_vec;
    //outfile << "<MulExp>" << endl;
}

vector<Operand> Compiler::UnaryExp() {
    vector<Operand> op1_vec;
    if (preRead(2) == -1) {
        error();
    }
    if (firstWord == "(" || firstSym == INTCON) {
        op1_vec = PrimaryExp();
    } else if (firstSym == IDENT) {
        string name;
        int count = 0;
        vector<ParamType> paramType;
        if (secondWord == "(") {

            Symbol symbol;
            getSym(0);  //读Ident
            name = curWord;
            int res = findFunction(name, symbol);
            //是函数调用
            //需要保存环境
            midCodes.push_back(Quaternary("store_env", Operand(symbol)));
            if (isParam == 1) {
                curExpr += curWord;
            }
            /*
            if (checkUndefine() == -1) {
                curError = UNDEFINEERROR;
                errorOutput();
            }
            */
            getSym(0);  //读左括号
            if (isParam == 1) {
                curExpr += curWord;
            }

            preRead(1); //预读一个符号，如果是右括号直接退出
            if (firstWord == ")") {
                getSym(0);
                if (isParam == 1) {
                    curExpr += curWord;
                }
            } else {
                FuncRParams(count, paramType);
                isParam = 0;

                preRead(1);
                if (firstWord == ")") {
                    getSym(0);
                    curExpr += curWord;
                } else {
                    curError = LACKRPARENTERROR;
                    errorOutput();
                }

                //检查函数参数数量或类型不匹配
                if (res != -1) {
                    if (symbol.paramList.size() != count) {
                        curError = PARAMNUMUNMATCHERROR;
                        errorOutput();
                    } else if (checkParamUnmatch(symbol.paramList, paramType) == -1) {
                        curError = PARAMTYPEUNMATCHERROR;
                        errorOutput();
                    }
                } else {
                    curError = UNDEFINEERROR;
                    errorOutput();
                }
            }
            midCodes.push_back(Quaternary("call", Operand(symbol)));
            midCodes.push_back(Quaternary("restore_env", Operand(symbol)));

            Operand op3 = getNewReg();
            Operand op1("RET");
            midCodes.push_back(Quaternary("=", op1, op3));

            outfile1 << "<UnaryExp>" << endl;
            return vector<Operand>(1, op3);
        } else if (secondWord == "[") {
            op1_vec = PrimaryExp();       //数组中的值
        } else {
            op1_vec = PrimaryExp();
        }
    } else if (firstWord == "+" || firstWord == "-" || firstWord == "!") {
        UnaryOp();
        string op = curWord;
        vector<Operand> op2_vec = UnaryExp();
        if (op2_vec[0].isValueKnown) {
            int value = calculate(op, Operand(0), op2_vec[0]);
            outfile1 << "<UnaryExp>" << endl;
            return vector<Operand>(1, value);
        } else {
            Operand op3 = getNewReg();
            Quaternary load(op, Operand(0), op2_vec[0], op3);
            midCodes.push_back(load);
            op1_vec.push_back(op3);
            /*
            //回收寄存器
            string reg1 = op1_vec[0].tokenName;
            if (reg1[0] == 't' || reg1[0] == 's') {
                regs.push(reg1);
            }
             */
        }
    } else {
        error();
    }
    outfile1 << "<UnaryExp>" << endl;
    return op1_vec;
}

void Compiler::UnaryOp() {
    getSym(0);
    curExpr += curWord;
    if (curWord != "+" && curWord != "-" && curWord != "!") {
        error();
    }
    outfile1 << "<UnaryOp>" << endl;
}

void Compiler::FuncRParams(int &count, vector<ParamType> &paramType) {
    vector<Operand> op1_vec;
    isParam = 1;
    ParamType pt;
    op1_vec = Exp();
    midCodes.push_back(Quaternary("push", op1_vec));
    recycle_regs(op1_vec);
    isParam = 0;
    findIdentType(pt);
    paramType.push_back(pt);
    ++count;
    //curTmpNum = 0;

    while (true) {
        preRead(1);
        if (firstWord == ",") {
            getSym(0);
            curExpr.clear();
            isParam = 1;
            op1_vec = Exp();
            midCodes.push_back(Quaternary("push", op1_vec));
            recycle_regs(op1_vec);
            isParam = 0;
            findIdentType(pt);
            paramType.push_back(pt);
            ++count;
            //curTmpNum = 0;
        } else {
            break;
        }
    }
    curExpr.clear();

    outfile1 << "<FuncRParams>" << endl;
}

vector<Operand> Compiler::PrimaryExp() {
    vector<Operand> op1_vec;
    preRead(1);
    if (firstWord == "(") {
        getSym(0);
        curExpr += curWord;
        op1_vec = Exp();

        preRead(1);
        if (firstWord == ")") {
            getSym(0);
            curExpr += curWord;
        } else {
            curError = LACKRPARENTERROR;
            errorOutput();
        }

    } else if (firstSym == IDENT) {
        op1_vec = LVal();
    } else if (firstSym == INTCON) {
        op1_vec = Number();
    } else {
        error();
    }
    outfile1 << "<PrimaryExp>" << endl;
    return op1_vec;
}

vector<Operand> Compiler::LVal() {
    vector<Operand> tmp;
    Symbol symbol;
    getSym(0);   //读Ident
    findIdent(symbol);

    curExpr += curWord;

    if (checkUndefine() == -1) {
        curError = UNDEFINEERROR;
        errorOutput();
    }

    while (true) {
        preRead(1);
        if (firstWord == "[") {
            getSym(0);
            curExpr += curWord;
            vector<Operand> len = Exp();
            tmp.insert(tmp.end(), len.begin(), len.end());
            preRead(1);
            if (firstWord != "]") {
                curError = LACKRBRACKERROR;
                errorOutput();
            } else {
                getSym(0);
                curExpr += curWord;
            }
        } else {
            break;
        }
    }

    outfile1 << "<LVal>" << endl;

    Operand op1(symbol);
    vector<Operand> op2_vec;
    vector<Operand> op3_vec;
    if (tmp.size() == 0) {
        if (symbol.dimension == 0) {
            if (curLev == 0 || symbol.isConst == 1) {
                Operand op3(symbol.value);
                op3_vec.push_back(op3);
                Quaternary load("=", op1, op3);
                midCodes.push_back(load);
            } else {
                Operand op3 = getNewReg();
                op3_vec.push_back(op3);
                Quaternary load("=", op1, op3);
                midCodes.push_back(load);
            }
        } else if (symbol.dimension == 1) {
            /*
            for (long i = 0; i < symbol.di_len[0]; i++) {
                Operand op2(i);
                op2_vec.push_back(op2);
                if (curLev == 0 || symbol.isConst == 1) {
                    Operand op3(symbol.values[i]);
                    op3_vec.push_back(op3);
                } else {
                    Operand op3(regs.front());
                    regs.pop();
                    op3_vec.push_back(op3);
                }
            }
            Quaternary load("=[]", op1, op2_vec, op3_vec);
            midCodes.push_back(load);

            recycle_regs(op2_vec);
            */
            //理论上，这种情况只可能出现在函数调用中，传基地址即可
            Operand op2(0);
            op2_vec.push_back(op2);
            Operand op3 = getNewReg();
            op3_vec.push_back(op3);
            Quaternary load("deliver_address", op1, op2_vec, op3_vec);
            midCodes.push_back(load);

            recycle_regs(op2_vec);
        } else if (symbol.dimension == 2) {
            /*
            for (long i = 0; i < symbol.di_len[0] * symbol.di_len[1]; i++) {
                Operand op2(i);
                op2_vec.push_back(op2);
                if (curLev == 0 || symbol.isConst == 1) {
                    Operand op3(symbol.values[i]);
                    op3_vec.push_back(op3);
                } else {
                    Operand op3(regs.front());
                    regs.pop();
                    op3_vec.push_back(op3);
                }
            }

            Quaternary load("=[]", op1, op2_vec, op3_vec);
            midCodes.push_back(load);

            recycle_regs(op2_vec);
            */

            //理论上，这种情况只可能出现在函数调用中，传基地址即可
            Operand op2(0);
            op2_vec.push_back(op2);
            Operand op3 = getNewReg();
            op3_vec.push_back(op3);
            Quaternary load("deliver_address", op1, op2_vec, op3_vec);
            midCodes.push_back(load);

            recycle_regs(op2_vec);
        }
    } else if (tmp.size() == 1) {
        /**
         * 注意：tmp中的值可能在编译期无法计算
         */
        if (symbol.dimension == 1) {
            if (tmp[0].isValueKnown == true) {
                Operand op2(tmp[0].value);
                op2_vec.push_back(op2);
            } else {
                //tmp[0]的值在编译期无法计算
                //Operand op2(tmp[0].name);
                op2_vec.push_back(tmp[0]);
            }

            if ((curLev == 0 || symbol.isConst == 1) && tmp[0].isValueKnown) {
                Operand op3(symbol.values[op2_vec[0].value]);
                op3_vec.push_back(op3);
                Quaternary load("=[]", op1, op2_vec, op3_vec);
                midCodes.push_back(load);
            } else {
                Operand op3 = getNewReg();
                op3_vec.push_back(op3);
                Quaternary load("=[]", op1, op2_vec, op3_vec);
                midCodes.push_back(load);

                recycle_regs(op2_vec);
            }
        } else if (symbol.dimension == 2) {
            if (tmp[0].isValueKnown == true) {
                for (long i = symbol.di_len[1] * tmp[0].value; i < symbol.di_len[1] * (tmp[0].value + 1); i++) {
                    Operand op2(i);
                    op2_vec.push_back(op2);
                    if (curLev == 0 || symbol.isConst == 1) {
                        Operand op3(symbol.values[i]);
                        op3_vec.push_back(op3);
                    } else {
                        Operand op3 = getNewReg();
                        op3_vec.push_back(op3);
                    }
                }
                Quaternary load("=[]", op1, op2_vec, op3_vec);
                midCodes.push_back(load);

                recycle_regs(op2_vec);
            }
            else {
                //tmp[0]的值在编译期无法计算
                //理论上，这种情况只可能出现在函数调用中，传基地址即可
                Operand op2(tmp[0].name);
                op2_vec.push_back(op2);
                Operand op3 = getNewReg();
                op3_vec.push_back(op3);
                Quaternary mul("*", op2, Operand(symbol.di_len[1]), op2);
                midCodes.push_back(mul);
                Quaternary load("deliver_address", op1, op2_vec, op3_vec);
                midCodes.push_back(load);

                recycle_regs(op2_vec);
            }
        }
    } else if (tmp.size() == 2) {
        /**
         * 注意：tmp中的值可能在编译期无法计算
         */
        if (tmp[0].isValueKnown == true && tmp[1].isValueKnown == true) {
            Operand op2(tmp[0].value * symbol.di_len[1] + tmp[1].value);
            if (curLev == 0 || symbol.isConst == 1) {
                Operand op3(symbol.values[op2.value]);
                op3_vec.push_back(op3);
                Quaternary load("=[]", op1, op2, op3);
                midCodes.push_back(load);
            } else {
                Operand op3 = getNewReg();
                op3_vec.push_back(op3);
                Quaternary load("=[]", op1, op2, op3);
                midCodes.push_back(load);

                recycle_regs(op2_vec);
            }
        } else if (tmp[0].isValueKnown == true) {
            //tmp[1]的值在编译期无法计算
            int offset1 = tmp[0].value * symbol.di_len[1];
            //计算偏移量
            Operand op3 = getNewReg();
            Quaternary add("+", Operand(offset1), tmp[1], op3);
            midCodes.push_back(add);
            recycle_regs(op2_vec);

            Operand op4 = getNewReg();
            op3_vec.push_back(op4);
            Quaternary load("=[]", op1, op3, op4);
            midCodes.push_back(load);
            recycle_regs(op2_vec);
        } else {
            //tmp[0]的值在编译期无法计算
            //0r tmp[0]、tmp[1]的值在编译期均无法计算
            Operand op3 = getNewReg();
            Quaternary mul("*", Operand(symbol.di_len[1]), tmp[0], op3);
            midCodes.push_back(mul);
            recycle_regs(op2_vec);

            Operand op4 = getNewReg();
            Quaternary add("+", op3, tmp[1], op4);
            midCodes.push_back(add);
            recycle_regs(op2_vec);

            Operand op5 = getNewReg();
            op3_vec.push_back(op5);
            Quaternary load("=[]", op1, op4, op5);
            midCodes.push_back(load);
            recycle_regs(op2_vec);
        }
    }
    return op3_vec;
}

vector<Operand> Compiler::Number() {
    getSym(0);  //读入int型整数
    curExpr += curWord;

    /**
     * 四元式
     */
    Operand op1(atoi(curWord.c_str()));

    /*
    Operand op3("t" + to_string(curTmpNum++));
    op3.value = op1.value;
    op3.isValueKnown = 1;
    Quaternary quaternary("=", op1, op3);
    midCodes.push_back(quaternary);
    */

    outfile1 << "<Number>" << endl;
    return vector<Operand>(1, op1);
}

vector<Operand> Compiler::Exp() {
    vector<Operand> op1_vec = AddExp();
    outfile1 << "<Exp>" << endl;
    return op1_vec;
}

Operand Compiler::getNewReg() {
    if (regs.empty()) {
        Symbol symbol;
        symbol.dimension = 0;
        symbol.name = "#"+ to_string(tmpVarIdx++);
        symbol.lev = curLev;
        symbol.isConst = 0;
        symbol.type = INT;
        symbol.isPara = 0;
        Operand op(symbol);
        midCodes.push_back(Quaternary("var", op));
        return op;
    }
    else {
        Operand op(regs.front());
        regs.pop();
        return op;
    }
}

void Compiler::recycle_regs(vector<Operand> op_vec) {
    //回收寄存器
    for (long i = 0; i < op_vec.size(); ++i) {
        bool hasReg = false;
        string reg = op_vec[i].tokenName;
        //遍历queue查找寄存器是否已经在regs中
        for (int j = 0; j < regs.size(); ++j) {
            if (regs.front() == reg) {
                hasReg = true;
            }
            regs.push(regs.front());
            regs.pop();
        }
        if ((reg[0] == 't' || reg[0] == 's') && hasReg == false) {
            regs.push(reg);
        }
    }
    //回收临时变量
    for (long i = 0; i < op_vec.size(); ++i) {
        string name = op_vec[i].name;
        if (name[0] == '#') {
            tmpVarIdx--;
        }
    }
}