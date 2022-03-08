//
// Created by Sarah on 2021/11/7.
//
#include <regex>
#include "Compiler.h"

using namespace std;

void Compiler::findIdent(Symbol &symbol) {
    for (int i = symbolTab.size() - 1; i >= 0; i--) {
        if (symbolTab[i].name == curWord && symbolTab[i].isValid == 1) {
            symbol = symbolTab[i];
            return;
        }
    }
}

long Compiler::calculate(string opcode, Operand op1, Operand op2) {
    long res = 0;
    if (opcode == "+") {
        res = op1.value + op2.value;
    } else if (opcode == "-") {
        res = op1.value - op2.value;
    } else if (opcode == "*") {
        res = op1.value * op2.value;
    } else if (opcode == "/") {
        res = op1.value / op2.value;
    } else if (opcode == "%") {
        res = op1.value % op2.value;
    } else if (opcode == "!") {
        res = op2.value == 0 ? 1 : 0;
    } else if (opcode == "=") {
        res = op1.value;
    } else if (opcode == "<") {
        res = op1.value < op2.value ? 1 : 0;
    } else if (opcode == ">") {
        res = op1.value > op2.value ? 1 : 0;
    } else if (opcode == "<=") {
        res = op1.value <= op2.value ? 1 : 0;
    } else if (opcode == ">=") {
        res = op1.value >= op2.value ? 1 : 0;
    } else if (opcode == "==") {
        res = op1.value == op2.value ? 1 : 0;
    } else if (opcode == "!=") {
        res = op1.value == op2.value ? 0 : 1;
    }
    return res;
}

//生成赋初值的常量、变量定义的中间代码（普通变量、一维数组、二维数组）
void Compiler::defMidCode(Symbol &symbol, vector<vector<Operand>> values, vector<Operand> tmp) {
    //生成中间代码
    //如果维数为0，说明是常量赋值
    vector<Operand> line;
    vector<Operand> op1_vec;
    vector<Operand> op2_vec;
    if (symbol.dimension == 0) {
        symbol.value = tmp[0].value;
        Operand op3(symbol);
        op3.isValueKnown = 1;
        Quaternary quaternary("=", tmp[0], op3);
        cout << tmp[0].value << endl;
        midCodes.push_back(quaternary);
    } else if (symbol.dimension == 1) {
        line = values[0];
        long i = 0;
        for (; i < line.size(); i++) {
            if (i >= symbol.di_len[0]) {
                break;
            }
            symbol.values.push_back(line[i].value);
            op1_vec.push_back(Operand(i));
            op2_vec.push_back(line[i]);
        }
        //初值长度不够，需要补零
        for (; i < symbol.di_len[0]; i++) {
            symbol.values.push_back(0);
            op1_vec.push_back(Operand(i));
            op2_vec.push_back(Operand(0));
        }
        Operand op3(symbol);
        op3.isValueKnown = 1;
        Quaternary quaternary("[]=", op1_vec, op2_vec, op3);
        midCodes.push_back(quaternary);
    } else if (symbol.dimension == 2) {
        long oneD_len = symbol.di_len[0];
        long twoD_len = symbol.di_len[1];
        long i = 0;
        long count = 0;
        for (; i < values.size(); i++) {
            if (i >= oneD_len) {
                break;
            }
            line = values[i];
            long j = 0;
            for (; j < line.size(); j++) {
                /*
                if (j >= twoD_len) {
                    break;
                }
                 */
                symbol.values.push_back(line[j].value);
                op1_vec.push_back(Operand(i * twoD_len + j));
                op2_vec.push_back(line[j]);
                ++count;
            }
            //给每一行赋值时，如果长度不够，就要补零
            for (; j < twoD_len; j++) {
                symbol.values.push_back(0);
                if (i * twoD_len + j >= count) {
                    op1_vec.push_back(Operand(i * twoD_len + j));
                    op2_vec.push_back(Operand(0));
                    ++count;
                }
            }
        }
        for (; i < oneD_len; i++) {
            for (int j = 0; j < twoD_len; j++) {
                symbol.values.push_back(0);
                if (i * twoD_len + j >= count) {
                    op1_vec.push_back(Operand(i * twoD_len + j));
                    op2_vec.push_back(Operand(0));
                    ++count;
                }
            }
        }
        for (; count < oneD_len * twoD_len; count++) {
            symbol.values.push_back(0);
            op1_vec.push_back(Operand(count));
            op2_vec.push_back(Operand(0));
        }
        Operand op3(symbol);
        op3.isValueKnown = 1;
        Quaternary quaternary("[]=", op1_vec, op2_vec, op3);
        midCodes.push_back(quaternary);
    }
}

void Compiler::splitString(string str, vector<string> &splits) {
    str = str.substr(1, str.size() - 2);
    int i = 0;
    int last = -1;
    for (; i < str.size(); ++i) {
        if (i < str.size() - 1 && str.at(i) == '\\' && str.at(i + 1) == 'n') {
            if (last < i - 1) {
                splits.push_back(str.substr(last + 1, i - last - 1));
            }
            splits.push_back("\\n");
            last = i + 1;
        }
        else if (i < str.size() - 1 && str.at(i) == '%' && str.at(i + 1) == 'd') {
            if (last < i - 1) {
                splits.push_back(str.substr(last + 1, i - last - 1));
            }
            splits.push_back("%d");
            last = i + 1;
        }
    }
    if ((last + 1) <= str.size() - 1) {
        splits.push_back(str.substr(last + 1, str.size() -1 - last));
    }

}

int Compiler::isPow(long pow, long num) {
    //only positive numbers
    while (num > 1) {
        if (num % pow != 0) {
            return -1;  //代表不是幂
        }
        num /= pow;
    }
    return 1;
}

long Compiler::pow_index(long pow, long num) {
    long index = 0;
    while (num >= pow) {
        num /= pow;
        ++index;
    }
    return index;
}