//
// Created by Sarah on 2021/11/6.
//

#ifndef GENERATEMIPS_OPERAND_H
#define GENERATEMIPS_OPERAND_H

class Operand {
public:
    string name;            //代表符号的真实名字 有可能是int型整数或符号名称或value值
    string tokenName;       //t0、t1等token
    long value;
    bool isValueKnown;
    bool isNum;
    Symbol symbol;

    Operand() {}
    Operand(long value) : name(to_string(value)), value(value), isValueKnown(true), isNum(true) {
        tokenName = "";
    }
    Operand(string tokenName) : name(tokenName), tokenName(tokenName), value(0), isValueKnown(false), isNum(false) {
        symbol.name = tokenName;
    }
    Operand(Symbol symbol) : name(symbol.name), value(0), isValueKnown(false), symbol(symbol), isNum(false) {
        tokenName = "";
        if (symbol.lev == 0) {
            isValueKnown = true;
        }
    }
    Operand(string tokenName, string name) : name(name), tokenName(tokenName), value(0), isValueKnown(true), isNum(false) {}
};

#endif //GENERATEMIPS_OPERAND_H
