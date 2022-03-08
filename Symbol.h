//TYPE
#define INT         47
#define VOID        48
#define FUNCTION    49

#include<string>
#include<vector>
using namespace std;

class Symbol {
public:
    string name;
    int isConst;                //0代表不是const, 1代表是const
    int type;                   //1代表整型变量
    int isPara;
    int dimension;              //维数
    vector<long> di_len;        //每一维的长度
    vector<Symbol> paramList;   //函数参数列表
    int size;                   //函数需要多少空间
    int returnType;             //函数返回值类型, 1代表void, 2代表int
    int lev;                    //该符号所属层级
    int isValid;

    long value;                  //全局变量的值
    vector<long> values;         //全局数组的值（一维、二维）

};


