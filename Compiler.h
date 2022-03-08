#include<iostream>
#include<fstream>
#include<vector>
#include<stack>
#include<map>
#include<utility>
#include<queue>

#include "limits.h"
#include "definition.h"
#include "Symbol.h"
#include "Quaternary.h"
#include "VarAddress.h"

using namespace std;

class Compiler {
public:
    Compiler(string inPath, string outPath1, string outPath2, string outPath3, string outPath4, string outPath5);

    void Process();

private:
    vector<string> code;

    ifstream infile;
    ofstream outfile1;
    ofstream outfile2;
    ofstream outfile3;
    ofstream outfile4;
    ofstream outfile5;

    string curWord;  //当前识别到的词
    int curSym;  //当前类别码

    //预读（最多预读3个word）
    string firstWord;
    int firstSym;
    int firstLineNum;
    string secondWord;
    string thirdWord;

    string curLine;     //
    char curChar;       //当前识别到的字符
    int curLineNum;     //当前的行数
    int curLineLength;  //当前行的长度
    int curIndex;       //当前字符在行内的下标
    string curExpr;
    string curFunctionName;     //当前函数块的名字

    int hasReturn;      //标记是否有return

    int isParam;
    int isLoop;

    //错误处理
    int curError;
    int curLev;     //当前层级
    stack<int> levStack;

    //符号表管理
    int curFuncSize;          //当前函数块内变量要占多少空间，包括普通变量和数组，不包括形参
    vector<Symbol> symbolTab;       //符号表
    vector<int> indexTab;           //分程序索引表

    //中间代码生成
    int reg_num;
    int and_num;
    int or_num;
    int if_num;
    int tmpVarIdx;
    stack<int> ifStack;
    int while_num;
    stack<int> whileStack;
    queue<string> regs;                //寄存器池
    vector<Quaternary> global_vars; //全局变量，还包括需输出的字符串
    vector<Quaternary> global_strings;  //字符串
    vector<Quaternary> midCodes;    //不包括global_vars中的内容

    int curStrNum = 0;

    //生成mips汇编代码
    vector<string> mips_codes;              //存储生成的mips指令
    long sp_top;                            //栈顶指针的位置，记录当前内存中有多大空间已经存储了变量
    vector<VarAddress> var_sp;              //存放每个symbol对应的起始地址
    long global_top;
    vector<VarAddress> global_address;      //全局变量的相对地址

    int isLetter(char c);

    int isReserved(string s);

    int getSym(int preRead);

    void LexerOutput();

    int getChar(int preRead);

    int preRead(int times);  //预读一个word

    //SyntaxAnalysis
    void CompUnit();

    //void Decl();
    void ConstDecl();

    void VarDecl();

    void ConstDef();    // 包含普通变量、⼀维数组、⼆维数组共三种情况

    void VarDef();

    void FuncDef();

    void MainFuncDef();

    void FuncType();

    void FuncFParams(vector<Symbol> &paramList);

    void FuncFParam(Symbol &symbol);

    void Block();

    void BlockItem();

    int hasEqual();

    void Stmt();

    vector<Operand> Cond();

    vector<Operand> ConstExp();

    vector<Operand> AddExp();

    vector<Operand> MulExp();

    vector<Operand> RelExp();

    vector<Operand> EqExp();

    vector<Operand> LAndExp();

    vector<Operand> LOrExp();

    vector<Operand> UnaryExp();

    vector<Operand> PrimaryExp();

    vector<Operand> Exp();

    void ConstInitVal(vector<vector<Operand>> &res, vector<Operand> &tmp);

    void InitVal(vector<vector<Operand>> &res, vector<Operand> &tmp);

    void FuncRParams(int &count, vector<ParamType> &paramType);

    vector<Operand> LVal();

    void initialize_regs();

    void recycle_regs(vector<Operand> op_vec);

    vector<Operand> Number();

    void UnaryOp();

    //错误处理
    int checkFormatString();

    int checkRedefine(string name, int targetLev);

    int checkUndefine();

    void findIdentType(ParamType &pt);

    int findFunction(string name, Symbol &symbol);

    int checkParamUnmatch(vector<Symbol> paramList, vector<ParamType> paramType);

    void error();

    void errorOutput();

    //mips汇编代码生成
    void gen_MIPS();

    void gen_global_vars();

    void gen_global_strings();

    void gen_stmt();

    long get_global_address(Symbol symbol);

    long get_var_address(Symbol symbol);

    void assignSingle(Quaternary q);

    void assignArray(Quaternary q);

    void loadArray(Quaternary q);

    void arithmetic(Quaternary q);

    void relation(Quaternary q);

    void add_global(Symbol symbol);

    void add_var(Symbol symbol);

    void add_FParam(Symbol symbol);

    void endBlock();

    void store_regs();

    void restore_regs();

    Operand getNewReg();

    //Utils
    void findIdent(Symbol &symbol);

    long calculate(string opcode, Operand op1, Operand op2);

    void defMidCode(Symbol &symbol, vector<vector<Operand>> values, vector<Operand> tmp);

    void splitString(string str, vector<string> &splits);

    int isPow(long pow, long num);

    long pow_index(long pow, long num);
};
