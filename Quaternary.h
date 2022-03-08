//
// Created by Sarah on 2021/11/6.
//
/**
 * 四元式结构
 */
#ifndef GENERATEMIPS_QUATERNARY_H
#define GENERATEMIPS_QUATERNARY_H

#include "Operand.h"
using namespace std;
class Quaternary {
public:
    string opcode;  //操作符
    vector<Operand> op1_vec;
    vector<Operand> op2_vec;
    vector<Operand> op3_vec;

    Quaternary(string opcode, Operand op1, Operand op2, Operand op3) : opcode(opcode) {
        op1_vec.push_back(op1);
        op2_vec.push_back(op2);
        op3_vec.push_back(op3);
    }
    Quaternary(string opcode, Operand op1, Operand op3) : opcode(opcode) {
        op1_vec.push_back(op1);
        op3_vec.push_back(op3);
    }
    Quaternary(string opcode) : opcode(opcode) {}

    Quaternary(string opcode, Operand op1) : opcode(opcode) {
        op1_vec.push_back(op1);
    }

    Quaternary(string opcode, vector<Operand> op1_vec) : opcode(opcode), op1_vec(op1_vec) {
    }

    Quaternary(string opcode, vector<Operand> op2_vec, vector<Operand> op3_vec) 
        : opcode(opcode), op2_vec(op2_vec), op3_vec(op3_vec) {}

    Quaternary(string opcode, vector<Operand> op1_vec, vector<Operand> op2_vec, vector<Operand> op3_vec)
        : opcode(opcode), op1_vec(op1_vec), op2_vec(op2_vec), op3_vec(op3_vec) {}

    Quaternary(string opcode, Operand op1, vector<Operand> op2_vec, vector<Operand> op3_vec)
        : opcode(opcode), op2_vec(op2_vec), op3_vec(op3_vec) {
        op1_vec.push_back(op1);
    }

    Quaternary(string opcode, vector<Operand> op1_vec, vector<Operand> op2_vec, Operand op3)
            : opcode(opcode), op1_vec(op1_vec), op2_vec(op2_vec) {
        op3_vec.push_back(op3);
    }
    vector<string> toString() {
        vector<string> res_vec;
        string res;
        if ("read" == opcode ||
        "const" == opcode || "var" == opcode ||
        "para" == opcode ||
        "push" == opcode || "call" == opcode) {
            res = opcode + " " + op1_vec[0].name;
            res_vec.push_back(res);
        }
        else if ("LineBreak" == opcode) {
            res = opcode;
            res_vec.push_back(res);
        }
        else if ("int" == opcode || "void" == opcode || "end_func" == opcode) {
            res = opcode+ " " + op1_vec[0].name + "()";
            res_vec.push_back(res);
        }
        else if ("[]=" == opcode) {
            for (int i = 0; i < op2_vec.size(); ++i) {
                res = op3_vec[0].name + "[" + op1_vec[i].name + "] = " + op2_vec[i].name;
                res_vec.push_back(res);
            }
        }
        else if ("=" == opcode) {
            res = op3_vec[0].name + " = " + op1_vec[0].name;
            res_vec.push_back(res);
        }
        else if ("=[]" == opcode) {
            for (int i = 0; i < op2_vec.size(); ++i) {
                res = op3_vec[i].name + " = " + op1_vec[0].name + "[" + op2_vec[i].name + "]";
                res_vec.push_back(res);
            }
        }
        else if (opcode == "+" || opcode == "-" || opcode == "*" || opcode == "/" || opcode == "%"
                || opcode == "<" || opcode == ">" || opcode == "<=" || opcode == ">="
                || opcode == "==" || opcode == "!="
                || opcode == "&&"
                || opcode == "||") {
            res = op3_vec[0].name + " = " + op1_vec[0].name + " " + opcode + " " + op2_vec[0].name;
            res_vec.push_back(res);
        }
        else if (opcode == "Block" || "EndBlock" == opcode || "main" == opcode) {
            res = "\n" + opcode;
            res_vec.push_back(res);
        }
        else if (opcode == "write") {
            res = opcode + " " + op1_vec[0].tokenName + " " + op1_vec[0].name;
            res_vec.push_back(res);
        }
        else if (opcode == "deliver_address") {
            res = opcode + " " + op3_vec[0].tokenName + " = " + op1_vec[0].name + "[" + op2_vec[0].name + "]";
            res_vec.push_back(res);
        }
        else if (opcode == "label") {
            res = opcode + " " + op1_vec[0].tokenName + ":";
            res_vec.push_back(res);
        } else if (opcode == "bgtz" || opcode == "beqz") {
            res = opcode + " " + op1_vec[0].name + " " + op3_vec[0].tokenName;
            res_vec.push_back(res);
        } else if (opcode == "jump") {
            res = opcode + " " + op1_vec[0].tokenName;
            res_vec.push_back(res);
        } else if ("store_env" == opcode || "restore_env" == opcode) {
            res = opcode + " " + op1_vec[0].name;
            res_vec.push_back(res);
        } else if ("return" == opcode) {
            if (op1_vec.empty()) {
                res = opcode;
                res_vec.push_back(res);
            } else {
                res = opcode + " " + op1_vec[0].name;
                res_vec.push_back(res);
            }
        }
        return res_vec;
    }
};

#endif //GENERATEMIPS_QUATERNARY_H
