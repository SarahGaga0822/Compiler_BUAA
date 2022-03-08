#include<map>
#define MAINTK      1
#define CONSTTK     2
#define INTTK       3
#define BREAKTK     4
#define CONTINUETK  5
#define IFTK        6
#define ELSETK      7
#define WHILETK     8
#define GETINTTK    9
#define PRINTFTK    10
#define RETURNTK    11
#define VOIDTK      12
#define RESERVE     13      //保留字统称

#define INTCON      14
#define IDENT       15
#define STRCON      16
#define ARITHOPER   17      //加减乘除余
#define SEMICN      18      //;
#define COMMA       19      //,
#define BRACKETCHAR     20  //各类括号统称
#define LEQ         21      //小于等于
#define LSS         22      //小于
#define GEQ         23      //大于等于
#define GRE         24      //大于
#define NEQ         25      //!=
#define NOT         26      //!
#define EQL         27      //==
#define ASSIGN      28      //=
#define AND         29      //&&
#define OR          30      //||

#define SINGLEAND   31      //&
#define SINGLEOR    32      //|

#define ANNOTATION  33      //注释

//错误类型
#define ILLEGALSYMBOLERROR      34  //非法符号
#define REDEFINEERROR           35  //名字重定义
#define UNDEFINEERROR           36  //未定义的名字
#define PARAMNUMUNMATCHERROR    37  //函数参数个数不匹配
#define PARAMTYPEUNMATCHERROR   38  //函数参数类型不匹配
#define UNMATCHRETURNERROR      39  //无返回值的函数存在不匹配的return语句
#define LACKRETURNERROR         40  //有返回值的函数缺少return语句
#define CHANGECONSTERROR        41  //不能改变常量的值
#define LACKSEMICERROR          42  //缺少分号
#define LACKRPARENTERROR        43  //缺少右小括号’)’
#define LACKRBRACKERROR         44  //缺少右中括号’]’
#define UNMATCHEXPNUMERROR      45  //printf中格式字符与表达式个数不匹配
#define BREAKORCONTINUEERROR    46  //在非循环块中使用break和continue语句

#define GLOBAL_BEGIN            0x10010000
#define SP_TOP                  0x7fffeffc

using namespace std;

const map<string, string> reservedWords = {
    {"main", "MAINTK"}, 
    {"const", "CONSTTK"}, 
    {"int", "INTTK"}, 
    {"break", "BREAKTK"},
    {"continue", "CONTINUETK"}, 
    {"if", "IFTK"}, 
    {"else", "ELSETK"},  
    {"while", "WHILETK"}, 
    {"getint", "GETINTTK"}, 
    {"printf", "PRINTFTK"}, 
    {"return", "RETURNTK"},
    {"void", "VOIDTK"}
};

const map<string, string> arithOpers = {
    {"+", "PLUS"},
    {"-", "MINU"},
    {"*", "MULT"},
    {"/", "DIV"},
    {"%", "MOD"}
};

const map<string, string> bracketChars = {
    {"(", "LPARENT"},
    {")", "RPARENT"},
    {"[", "LBRACK"},
    {"]", "RBRACK"},
    {"{", "LBRACE"},
    {"}", "RBRACE"}
};

const map<int, string> errors = {
    {ILLEGALSYMBOLERROR, "a"},
    {REDEFINEERROR, "b"},
    {UNDEFINEERROR, "c"},
    {PARAMNUMUNMATCHERROR, "d"},
    {PARAMTYPEUNMATCHERROR, "e"},
    {UNMATCHRETURNERROR, "f"},
    {LACKRETURNERROR, "g"},
    {CHANGECONSTERROR, "h"},
    {LACKSEMICERROR, "i"},
    {LACKRPARENTERROR, "j"},
    {LACKRBRACKERROR, "k"},
    {UNMATCHEXPNUMERROR, "l"},
    {BREAKORCONTINUEERROR, "m"}
};

typedef struct ParamType {
    string name;
    int isConst;
    int type;       //类型，int或void
    int dimension;  //维数
} ParamType;
