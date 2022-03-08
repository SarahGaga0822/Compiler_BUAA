//
// Created by Sarah on 2021/11/11.
//
#include "Compiler.h"

void Compiler::gen_MIPS() {
    curLev = 0;
    while (!levStack.empty()) {
        levStack.pop();
    }

    gen_global_vars();   //存入全局变量的mips代码
    gen_global_strings();

    mips_codes.push_back("\n.text");
    mips_codes.push_back("jal main");
    mips_codes.push_back("li $v0 10");
    mips_codes.push_back("syscall");

    gen_stmt();

    for (int i = 0; i < mips_codes.size(); ++i) {
        outfile4 << mips_codes[i] << endl;
    }
}

void Compiler::gen_global_vars() {
    int i = 0;
    for (; i < midCodes.size(); ++i) {
        Quaternary q = midCodes[i];
        if (q.opcode == "void" || q.opcode == "int" || q.opcode == "main") {
            //如果识别到函数定义，不算全局变量
            break;
        }
        if (q.opcode == "const" || q.opcode == "var") {
            add_global(q.op1_vec[0].symbol);
        }
        if ((q.opcode == "=" || q.opcode == "[]=") && q.op3_vec[0].tokenName == "" && q.op3_vec[0].isNum == false) {
            global_vars.push_back(q);
        }
    }
    midCodes.erase(midCodes.begin(), midCodes.begin() + i);

    mips_codes.push_back(".data");
    for (i = 0; i < global_vars.size(); ++i) {
        //生成全局变量（不包括字符串）的mips代码
        Quaternary q = global_vars[i];
        string instruct;
        if (q.opcode == "write") {
            instruct += q.op1_vec[0].tokenName + ": .asciiz ";
            instruct += "\"" + q.op1_vec[0].name + "\"";
        } else if (q.opcode == "=") {
            instruct += q.op3_vec[0].name + ": .word " + q.op1_vec[0].name;
        } else if (q.opcode == "[]=") {
            instruct += q.op3_vec[0].name + ": .word ";
            for (int j = 0; j < q.op1_vec.size(); ++j) {
                instruct += q.op2_vec[j].name + " ";
            }
        }
        mips_codes.push_back(instruct);
    }
}

void Compiler::gen_global_strings() {
    for (int i = 0; i < global_strings.size(); ++i) {
        Quaternary q = global_strings[i];
        string instruct;
        instruct += q.op1_vec[0].tokenName + ": .asciiz ";
        instruct += "\"" + q.op1_vec[0].name + "\"";
        mips_codes.push_back(instruct);
    }
    mips_codes.push_back("$ENDL: .asciiz \"\\n\"");
}

void Compiler::gen_stmt() {
    long ra_address = 0;
    stack<long> ra_address_stack;
    stack<Symbol> func_stack;
    stack<long> push_times_stack;

    long i = 0;
    for (; i < midCodes.size(); ++i) {
        Quaternary q = midCodes[i];
        string instruct;
        if (q.opcode == "void" || q.opcode == "int") {
            //函数定义
            ++curLev;
            var_sp.clear();
            sp_top = -1;
            mips_codes.push_back("\n" + q.op1_vec[0].name + ":");
        } else if (q.opcode == "var" || q.opcode == "const") {
            //给变量在内存中分配位置
            Symbol symbol = q.op1_vec[0].symbol;
            add_var(symbol);
        } else if (q.opcode == "=") {
            assignSingle(q);
        } else if (q.opcode == "[]=") {
            assignArray(q);
        } else if (q.opcode == "=[]") {
            loadArray(q);
        } else if (q.opcode == "Block") {
            levStack.push(++curLev);
        } else if (q.opcode == "EndBlock") {
            endBlock();
        } else if (q.opcode == "main") {
            mips_codes.push_back("\nmain:");
        } else if (q.opcode == "+" || q.opcode == "-" || q.opcode == "*" || q.opcode == "/" || q.opcode == "%" ||
                   q.opcode == "!") {
            arithmetic(q);
        } else if (q.opcode == "read") {
            mips_codes.push_back("li $v0, 5");
            mips_codes.push_back("syscall");
            if (q.op1_vec[0].tokenName == "") {
                long address = get_var_address(q.op1_vec[0].symbol);
                string sw;
                if (address > 0) {
                    //sw $t0, -20($sp)
                    sw = "sw $v0, -" + to_string(4 * address) + "($sp)";
                } else if (address == 0) {
                    sw = "sw $v0, 0($sp)";
                }
                mips_codes.push_back(sw);
            } else {
                mips_codes.push_back("move $" + q.op1_vec[0].tokenName + ", $v0");
            }
        } else if (q.opcode == "write") {
            if (q.op1_vec[0].tokenName == "") {
                if (q.op1_vec[0].isValueKnown == 1 && q.op1_vec[0].isNum == 1) {
                    //常数
                    mips_codes.push_back("li $a0, " + q.op1_vec[0].name);
                } else {
                    //寄存器不够时分配的临时变量
                    long address = get_var_address(q.op1_vec[0].symbol);
                    string lw;
                    if (address > 0) {
                        //lw $t0, -20($sp)
                        lw = "lw $a0, -" + to_string(4 * address) + "($sp)";
                    } else if (address == 0) {
                        lw = "lw $a0, 0($sp)";
                    }
                    mips_codes.push_back(lw);
                }
                mips_codes.push_back("li $v0, 1");
                mips_codes.push_back("syscall");
            } else if (q.op1_vec[0].tokenName[0] == '$') {
                //string
                mips_codes.push_back("la $a0, " + q.op1_vec[0].tokenName);
                mips_codes.push_back("li $v0, 4");
                mips_codes.push_back("syscall");
            } else {
                //t0、t1等临时变量
                mips_codes.push_back("move $a0, $" + q.op1_vec[0].name);
                mips_codes.push_back("li $v0, 1");
                mips_codes.push_back("syscall");
            }
        } else if (q.opcode == "LineBreak") {
            //输出'\n'
            mips_codes.push_back("la $a0, $ENDL");
            mips_codes.push_back("li $v0, 4");
            mips_codes.push_back("syscall");
        } else if (q.opcode == "para") {
            //函数参数进入符号表
            Symbol symbol = q.op1_vec[0].symbol;
            add_FParam(symbol);
        } else if (q.opcode == "return") {
            if (!q.op1_vec.empty()) {
                Operand op1 = q.op1_vec[0];
                if (op1.isValueKnown == 1 && op1.isNum == 1) {
                    mips_codes.push_back("li $v0, " + op1.name);
                } else {
                    //t0、t1等token
                    mips_codes.push_back("move $v0, $" + op1.tokenName);
                }
            }
            mips_codes.push_back("jr $ra");
        } else if (q.opcode == "push") {
            int push_times = push_times_stack.top();
            push_times_stack.pop();
            push_times_stack.push(push_times + 1);

            Operand op1 = q.op1_vec[0];
            if (op1.isValueKnown == 1 && op1.isNum == 1) {
                Symbol symbol;
                symbol.name = op1.name;
                var_sp.push_back(VarAddress(symbol, ++sp_top));
            } else {
                var_sp.push_back(VarAddress(op1.symbol, ++sp_top));
            }

            long address;
            string sw;
            Symbol func = func_stack.top();
            int dimension = func.paramList[push_times].dimension;
            if (dimension == 0) {
                //普通变量传值
                if (op1.isValueKnown == 1 && op1.isNum == 1) {
                    //常数
                    mips_codes.push_back("li $s0, " + op1.name);
                    if (sp_top > 0) {
                        sw = "sw $s0, -" + to_string(4 * sp_top) + "($sp)";
                    } else {
                        sw = "sw $s0, 0($sp)";
                    }
                } else {
                    address = get_var_address(op1.symbol);
                    if (address == -1) {
                        //全局变量
                        mips_codes.push_back("lw $s0, " + op1.name);
                        sw = "sw $s0, -" + to_string(4 * sp_top) + "($sp)";
                    } else if (sp_top > 0) {
                        sw = "sw $" + op1.name + " -" + to_string(4 * sp_top) + "($sp)";
                    } else {
                        sw = "sw $" + op1.name + " 0($sp)";
                    }
                }
                mips_codes.push_back(sw);
            } else {
                //一维或二维数组传地址
                /**
                 * t3 = f[2]
                 * t4 = f[3]
                 * push t3
                 */
                int j = i - 1;
                for (; j >= 0; --j) {
                    Quaternary last = midCodes[j];
                    if (last.op3_vec[0].name == op1.name) {
                        break;
                    }
                }
                Quaternary target = midCodes[j];
                address = get_var_address(target.op1_vec[0].symbol);
                Operand op2 = target.op2_vec[0];
                if (address == -1) {
                    //全局数组
                    address = get_global_address(target.op1_vec[0].symbol);
                    if (op2.isValueKnown == 1 && op2.isNum == 1) {
                        address += op2.value;
                        address = GLOBAL_BEGIN + address * 4;
                        mips_codes.push_back("li $s0, " + to_string(address));
                        if (sp_top > 0) {
                            sw = "sw $s0, -" + to_string(4 * sp_top) + "($sp)";
                        } else {
                            sw = "sw $s0, 0($sp)";
                        }
                    } else {
                        /** t0 = a[t1] */
                        string tmpName = "$" + op2.tokenName;
                        mips_codes.push_back("addu " + tmpName + ", " + tmpName + ", " + to_string(address));
                        mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                        mips_codes.push_back("addu " + tmpName + ", " + tmpName + ", " + to_string(GLOBAL_BEGIN));
                        if (sp_top > 0) {
                            sw = "sw " + tmpName + ", -" + to_string(4 * sp_top) + "($sp)";
                        } else {
                            sw = "sw " + tmpName + ", 0($sp)";
                        }
                    }
                } else {
                    if (op2.isValueKnown == 1 && op2.isNum == 1) {
                        if (target.op1_vec[0].symbol.isPara == 0) {
                            address = (address - op2.value) * 4;
//                        address = SP_TOP - address;
                            mips_codes.push_back("subu $s0, $sp, " + to_string(address));
//                        mips_codes.push_back("li $s0, " + to_string(address));
                            if (sp_top > 0) {
                                sw = "sw $s0, -" + to_string(4 * sp_top) + "($sp)";
                            } else {
                                sw = "sw $s0, 0($sp)";
                            }
                        } else {
                            //op1数组是函数参数，需要先获取基地址
                            if (address > 0) {
                                mips_codes.push_back("lw $s0, -" + to_string(4 * address) + "($sp)");
                            } else {
                                mips_codes.push_back("lw $s0, 0($sp)");
                            }
                            mips_codes.push_back("addu $s0, $s0, " + to_string(op2.value * 4));
                            if (sp_top > 0) {
                                sw = "sw $s0, -" + to_string(4 * sp_top) + "($sp)";
                            } else {
                                sw = "sw $s0, 0($sp)";
                            }
                        }
                    } else {
                        string tmpName = "$" + op2.tokenName;
                        if (target.op1_vec[0].symbol.isPara == 0) {
                            mips_codes.push_back("li $s0, " + to_string(address));
                            mips_codes.push_back("subu " + tmpName + ", $s0, " + tmpName);
                            mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                            mips_codes.push_back("subu " + tmpName + ", $sp, " + tmpName);
                            if (sp_top > 0) {
                                sw = "sw " + tmpName + ", -" + to_string(4 * sp_top) + "($sp)";
                            } else {
                                sw = "sw " + tmpName + ", 0($sp)";
                            }
                        } else {
                            //op1数组是函数参数，需要先获取基地址
                            if (address > 0) {
                                mips_codes.push_back("lw $s0, -" + to_string(4 * address) + "($sp)");
                            } else {
                                mips_codes.push_back("lw $s0, 0($sp)");
                            }
                            mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                            mips_codes.push_back("addu $s0, $s0, " + tmpName);
                            if (sp_top > 0) {
                                sw = "sw $s0, -" + to_string(4 * sp_top) + "($sp)";
                            } else {
                                sw = "sw $s0, 0($sp)";
                            }
                        }
                    }
                }
                mips_codes.push_back(sw);
            }

        } else if (q.opcode == "call") {
            long address;
            for (int j = var_sp.size() - 1; j >= 0; --j) {
                VarAddress tmp = var_sp[j];
                if (tmp.symbol.name == "$ra") {
                    ra_address = ra_address_stack.top();
                    ra_address_stack.pop();
                    break;
                }
            }
            address = (ra_address + 1) * 4;

            mips_codes.push_back("subu $sp, $sp, " + to_string(address));
            mips_codes.push_back("jal " + q.op1_vec[0].name);
            mips_codes.push_back("addu $sp, $sp, " + to_string(address));
        } else if (q.opcode == "store_env") {
            store_regs();
            Symbol ra;
            ra.name = "$ra";
            func_stack.push(q.op1_vec[0].symbol);
            push_times_stack.push(0);
            var_sp.push_back(VarAddress(ra, ++sp_top));
            ra_address_stack.push(sp_top);
            if (sp_top > 0) {
                mips_codes.push_back("sw $ra, -" + to_string(sp_top * 4) + "($sp)");
            } else {
                mips_codes.push_back("sw $ra, 0($sp)");
            }

        } else if (q.opcode == "restore_env") {
            int j, k;
            for (j = var_sp.size() - 1; j >= 0; --j) {
                if (var_sp[j].address == ra_address) {
                    break;
                }
            }

            for (k = j; k >= j - reg_num; --k) {
                VarAddress tmp = var_sp[k];
                if (tmp.address > 0) {
                    mips_codes.push_back("lw " + tmp.symbol.name + ", -" + to_string(tmp.address * 4) + "($sp)");
                } else {
                    mips_codes.push_back("lw " + tmp.symbol.name + ", 0($sp)");
                }
            }

            for (k = j + 1; k < var_sp.size(); ++k) {
                var_sp[k].address = var_sp[k].address - (reg_num + 1);
            }

            var_sp.erase(var_sp.begin() + j - reg_num, var_sp.begin() + j + 1);

            Symbol func = func_stack.top();
            func_stack.pop();
            push_times_stack.pop();
            var_sp.erase(var_sp.end() - func.paramList.size(), var_sp.end());

            sp_top = var_sp.size() == 0 ? -1 : var_sp[var_sp.size() - 1].address;

            /*
            int j = var_sp.size() - 1;
            for (; j >= 0; --j) {
                VarAddress tmp = var_sp[j];
                if (tmp.symbol.name == "$ra") {
                    if (tmp.address > 0) {
                        mips_codes.push_back("lw $ra, -" + to_string(tmp.address * 4) + "($sp)");
                    } else {
                        mips_codes.push_back("lw $ra, 0($sp)");
                    }
                    break;
                }
            }
            var_sp.erase(var_sp.begin() + j, var_sp.end());
            sp_top = var_sp.size() == 0 ? -1 : var_sp[var_sp.size() - 1].address;
            */
        } else if (q.opcode == "end_func") {
            mips_codes.push_back("jr $ra");
        } else if (q.opcode == "<" || q.opcode == ">" || q.opcode == "<=" || q.opcode == ">=" || q.opcode == "==" ||
                   q.opcode == "!=") {
            relation(q);
        } else if (q.opcode == "bgtz" || q.opcode == "beqz") {
            string branch;
            if (q.op1_vec[0].tokenName == "") {
                mips_codes.push_back("li $s0, " + q.op1_vec[0].name);
                branch = q.opcode + " $s0, " + q.op3_vec[0].tokenName;
            } else {
                branch = q.opcode + " $" + q.op1_vec[0].name + ", " + q.op3_vec[0].tokenName;
            }
            mips_codes.push_back(branch);
        } else if (q.opcode == "label") {
            mips_codes.push_back(q.op1_vec[0].tokenName + ":");
        } else if (q.opcode == "jump") {
            mips_codes.push_back("j " + q.op1_vec[0].tokenName);
        }
    }
}

void Compiler::assignSingle(Quaternary q) {
    Operand op1 = q.op1_vec[0];
    Operand op3 = q.op3_vec[0];
    string sw;
    string lw;
    long address;
    if (op3.tokenName == "") {
        /** c = 2 or
         * c = a (a is a global var) or
         * c = d (d is a local var) or
         * c = t0 (t0 is a temporary var)
         */
        //等式左边不是t0、t1等临时变量，需将值存入内存
        if (op3.isNum == false) {
            if (op1.tokenName == "") {
                if (op1.isValueKnown == 1 && op1.isNum == 1) {
                    //右边是常数
                    mips_codes.push_back("li $s0, " + op1.name);
                } else {
                    address = get_var_address(op1.symbol);
                    if (address == -1) {
                        mips_codes.push_back("lw $s0, " + op1.name);
                    } else if (address > 0) {
                        //sw $t0, -20($sp)
                        mips_codes.push_back("lw $s0, -" + to_string(4 * address) + "($sp)");
                    } else {
                        mips_codes.push_back("lw $s0, 0($sp)");
                    }
                }
                address = get_var_address(op3.symbol);
                if (address == -1) {
                    //说明左边是全局变量
                    //sw $t0, a+0
                    sw = "sw $s0, " + op3.name;
                } else if (address > 0) {
                    //sw $t0, -20($sp)
                    sw = "sw $s0, -" + to_string(4 * address) + "($sp)";
                } else {
                    sw = "sw $s0, 0($sp)";
                }
                mips_codes.push_back(sw);
            } else {
                address = get_var_address(op3.symbol);
                if (address == -1) {
                    sw = "sw $" + op1.tokenName + ", " + op3.name;
                } else if (address > 0) {
                    sw = "sw $" + op1.tokenName + ", -" + to_string(4 * address) + "($sp)";
                } else {
                    sw = "sw $" + op1.tokenName + ", 0($sp)";
                }
                mips_codes.push_back(sw);
            }
        }

    } else {
        //等式左边是t0、t1等临时变量，无需将值存入内存，只需存入寄存器
        //右边是可以算出来的常量，包括常数（直接取value或name）、全局变量（可能被更改过，故不能直接取值，需要取name）、局部变量（需要从地址中取值）、RET（函数返回值）
        if (op1.isValueKnown == 1 && op1.isNum == 1) {
            //右边是常数
            mips_codes.push_back("li $" + op3.name + ", " + op1.name);
        } else if (op1.tokenName == "RET") {
            mips_codes.push_back("move $" + op3.name + ", $v0");
        } else {
            //右边是symbol
            address = get_var_address(op1.symbol);
            if (address == -1) {
                //右边是全局变量
                //lw $t0, a
                lw = "lw $" + op3.name + ", " + op1.name;
            } else if (address > 0) {
                lw = "lw $" + op3.name + ", -" + to_string(4 * address) + "($sp)";
            } else {
                lw = "lw $" + op3.name + ", 0($sp)";
            }
            mips_codes.push_back(lw);
        }
    }
}

void Compiler::assignArray(Quaternary q) {
    /** op3[op1] = op2; */
    string sw;
    string lw;
    Operand op3 = q.op3_vec[0];
    //给数组赋值的等式左边一定不是t0、t1等临时变量，需将值存入内存
    for (int i = 0; i < q.op1_vec.size(); ++i) {
        long address = get_var_address(op3.symbol);
        Operand op1 = q.op1_vec[i];
        Operand op2 = q.op2_vec[i];
        if (op1.isValueKnown == 1 && op2.isValueKnown == 1) {
            /**
             * li $t0, 1
             * sw $t0, -4($sp)
             */
            mips_codes.push_back("li $t0, " + to_string(op2.value));
            if (address == -1) {
                //说明是全局变量
                //sw $t0, a($s0)
                mips_codes.push_back("li $s0, " + to_string(op1.value * 4));
                sw = "sw $t0, " + op3.name + "($s0)";
            } else {
                if (op3.symbol.isPara == 0) {
                    address = (address - op1.value) * 4;
                    if (address > 0) {
                        //sw $t0, -4($sp)
                        sw = "sw $t0, -" + to_string(address) + "($sp)";
                    } else {
                        sw = "sw $t0, 0($sp)";
                    }
                } else {
                    //函数调用的参数，先从内存中取出数组的绝对基地址
                    if (address > 0) {
                        //sw $t0, -4($sp)
                        mips_codes.push_back("lw $t0, -" + to_string(4 * address) + "($sp)");
                    } else {
                        mips_codes.push_back("lw $t0, 0($sp)");
                    }
                    if (op1.value != 0) {
                        mips_codes.push_back("addu $t0, $t0, " + to_string(op1.value * 4));
                    }
                    mips_codes.push_back("li $t1, " + to_string(op2.value));
                    sw = "sw $t1, 0($t0)";
                }
            }
        } else if (op1.isValueKnown == 1) {
            /** a[1] = t0 */
            if (address == -1) {
                //sw $t0, a($s0)
                mips_codes.push_back("li $s0, " + to_string(op1.value * 4));
                sw = "sw $" + op2.tokenName + ", " + op3.name + "($s0)";
            } else {
                if (op3.symbol.isPara == 0) {
                    /**
                        subi $s0, $sp, address
                        sw $t0, 0($s0)
                    */
                    address = (address - op1.value) * 4;
                    if (address > 0) {
                        sw = "sw $" + op2.tokenName + ", -" + to_string(address) + "($sp)";
                    } else {
                        sw = "sw $" + op2.tokenName + ", 0($sp)";
                    }
                } else {
                    //函数调用的参数，先从内存中取出数组的绝对基地址
                    if (address > 0) {
                        //sw $s0, -4($sp)
                        mips_codes.push_back("lw $s0, -" + to_string(4 * address) + "($sp)");
                    } else {
                        mips_codes.push_back("lw $s0, 0($sp)");
                    }
                    if (op1.value != 0) {
                        mips_codes.push_back("addu $s0, $s0, " + to_string(op1.value * 4));
                    }
                    sw = "sw $" + op2.tokenName + ", 0($s0)";
                }
            }
        } else if (op2.isValueKnown == 1) {
            /** a[t0] = 5 */
            if (address == -1) {
                /** 将op2存入$s0 */
                mips_codes.push_back("li $s0, " + to_string(op2.value));
                mips_codes.push_back("sll $" + op1.name + ", $" + op1.name + ", 2");
                /** sw $s0, a($t0) */
                sw = "sw $s0, " + op3.name + "($" + op1.name + ")";
            } else {
                if (op3.symbol.isPara == 0) {
                    /** 将op2存入$s0
                     *  li  $s0, valueOf(op2)
                     *  sub $t0, valueOf(address), $t0
                     *  sll $t0, $t0, 2 (mul 4)
                     *  sub $t0, $sp, $t0
                     *  sw  $s0, 0($t0)
                     *  以上 $t0 需用 op1.tokenName 代替
                     */
                    string tmpName = "$" + op1.tokenName;
                    mips_codes.push_back("li $s0, " + to_string(op2.value));
                    mips_codes.push_back("li $s1, " + to_string(address));
                    mips_codes.push_back("subu " + tmpName + ", $s1, " + tmpName);
                    mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                    mips_codes.push_back("subu " + tmpName + ", $sp, " + tmpName);
                    sw = "sw $s0, 0(" + tmpName + ")";
                } else {
                    //$s0存地址，$s1存op2.value
                    if (address > 0) {
                        //sw $s0, -4($sp)
                        mips_codes.push_back("lw $s0, -" + to_string(4 * address) + "($sp)");
                    } else {
                        mips_codes.push_back("lw $s0, 0($sp)");
                    }
                    string tmpName = "$" + op1.tokenName;
                    mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                    mips_codes.push_back("addu $s0, $s0, " + tmpName);
                    mips_codes.push_back("li $s1, " + to_string(op2.value));
                    sw = "sw $s1, 0($s0)";
                }
            }
        } else {
            /** a[t0] = t1 */
            if (address == -1) {
                mips_codes.push_back("sll $" + op1.tokenName + ", $" + op1.tokenName + ", 2");
                sw = "sw $" + op2.tokenName + ", " + op3.name + "($" + op1.tokenName + ")";
            } else {
                if (op3.symbol.isPara == 0) {
                    string tmpName = "$" + op1.tokenName;
                    mips_codes.push_back("li $s0, " + to_string(address));
                    mips_codes.push_back("subu " + tmpName + ", $s0, " + tmpName);
                    mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                    mips_codes.push_back("subu " + tmpName + ", $sp, " + tmpName);
                    sw = "sw $" + op2.tokenName + ", 0(" + tmpName + ")";
                } else {
                    //$s0存地址
                    if (address > 0) {
                        //sw $s0, -4($sp)
                        mips_codes.push_back("lw $s0, -" + to_string(4 * address) + "($sp)");
                    } else {
                        mips_codes.push_back("lw $s0, 0($sp)");
                    }
                    string tmpName = "$" + op1.tokenName;
                    mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                    mips_codes.push_back("addu $s0, $s0, " + tmpName);
                    sw = "sw $" + op2.tokenName + ", 0($s0)";
                }
            }
        }
        mips_codes.push_back(sw);
    }
}

void Compiler::loadArray(Quaternary q) {
    /** op3 = op1[op2] */
    Operand op1 = q.op1_vec[0];
    long address;
    string lw;
    for (int i = 0; i < q.op2_vec.size(); ++i) {
        Operand op2 = q.op2_vec[i];
        Operand op3 = q.op3_vec[i];
        address = get_var_address(op1.symbol);
        if (op3.isNum == false) {
            if (op2.isValueKnown == 1) {
                if (address == -1) {
                    mips_codes.push_back("li $s0, " + to_string(op2.value * 4));
                    lw = "lw $" + op3.tokenName + ", " + op1.name + "($s0)";
                } else {
                    if (op1.symbol.isPara == 0) {
                        /**
                            subi $s0, $sp, address
                            lw $t0, 0($s0)
                        */
                        address = (address - op2.value) * 4;
                        if (address > 0) {
                            lw = "lw $" + op3.tokenName + ", -" + to_string(address) + "($sp)";
                        } else {
                            lw = "lw $" + op3.tokenName + ", 0($sp)";
                        }
                    } else {
                        //函数调用的参数，先从内存中取出数组的绝对基地址
                        if (address > 0) {
                            //sw $s0, -4($sp)
                            mips_codes.push_back("lw $s0, -" + to_string(4 * address) + "($sp)");
                        } else {
                            mips_codes.push_back("lw $s0, 0($sp)");
                        }
                        if (op2.value != 0) {
                            mips_codes.push_back("addu $s0, $s0, " + to_string(op2.value * 4));
                        }
                        lw = "lw $" + op3.tokenName + ", 0($s0)";
                    }
                }
                mips_codes.push_back(lw);
            } else {
                if (address == -1) {
                    mips_codes.push_back("sll $" + op2.tokenName + ", $" + op2.tokenName + ", 2");
                    lw = "lw $" + op3.tokenName + ", " + op1.name + "($" + op2.tokenName + ")";
                    mips_codes.push_back(lw);
                } else {
                    if (op1.symbol.isPara == 0) {
                        string tmpName;
                        if (op2.tokenName == "") {
                            //op2是临时变量
                            long op2_address = get_var_address(op2.symbol);
                            mips_codes.push_back("lw $s1, -" + to_string(op2_address * 4) + "($sp)");
                            tmpName = "$s1";
                        } else {
                            tmpName = "$" + op2.tokenName;
                        }
                        mips_codes.push_back("li $s0, " + to_string(address));
                        mips_codes.push_back("subu " + tmpName + ", $s0, " + tmpName);
                        mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                        mips_codes.push_back("subu " + tmpName + ", $sp, " + tmpName);
                        lw = "lw $" + op3.tokenName + ", 0(" + tmpName + ")";
                        mips_codes.push_back(lw);
                    } else {
                        //$s0存地址
                        if (address > 0) {
                            //sw $s0, -4($sp)
                            mips_codes.push_back("lw $s0, -" + to_string(4 * address) + "($sp)");
                        } else {
                            mips_codes.push_back("lw $s0, 0($sp)");
                        }

                        string tmpName;
                        if (op2.tokenName == "") {
                            //op2是临时变量
                            long op2_address = get_var_address(op2.symbol);
                            mips_codes.push_back("lw $s1, -" + to_string(op2_address * 4) + "($sp)");
                            tmpName = "$s1";
                        } else {
                            tmpName = "$" + op2.tokenName;
                        }
                        mips_codes.push_back("sll " + tmpName + ", " + tmpName + ", 2");
                        mips_codes.push_back("addu $s0, $s0, " + tmpName);

                        if (op3.tokenName == "") {
                            mips_codes.push_back("lw $s0, 0($s0)");
                            long op3_address = get_var_address(op3.symbol);
                            mips_codes.push_back("sw $s0, -" + to_string(4 * op3_address) + "($sp)");
                        } else {
                            lw = "lw $" + op3.tokenName + ", 0($s0)";
                            mips_codes.push_back(lw);
                        }
                    }
                }
            }
        }

    }
}

void Compiler::arithmetic(Quaternary q) {
    string opcode = q.opcode;
    Operand op1 = q.op1_vec[0];
    Operand op2 = q.op2_vec[0];
    Operand op3 = q.op3_vec[0];
    string name1 = op1.tokenName == "" ? op1.name : "$" + op1.name;
    string name2 = op2.tokenName == "" ? op2.name : "$" + op2.name;
    if (opcode == "+") {
        string add;
        if ((op1.tokenName == "" && op1.isNum == 0) && (op2.tokenName == "" && op2.isNum == 0)) {
            //op1和op2都是因为寄存器不够而被分配的临时变量
            long address = get_var_address(op1.symbol);
            string lw;
            if (address > 0) {
                //lw $t0, -20($sp)
                lw = "lw $s0, -" + to_string(4 * address) + "($sp)";
            } else if (address == 0) {
                lw = "lw $s0, 0($sp)";
            }
            mips_codes.push_back(lw);

            address = get_var_address(op2.symbol);
            if (address > 0) {
                //lw $t0, -20($sp)
                lw = "lw $s1, -" + to_string(4 * address) + "($sp)";
            } else if (address == 0) {
                lw = "lw $s1, 0($sp)";
            }
            mips_codes.push_back(lw);

            if (op3.tokenName == "" && op3.isNum == 0) {
                mips_codes.push_back("addu $s0, $s0, $s1");
                address = get_var_address(op3.symbol);
                if (address > 0) {
                    //lw $t0, -20($sp)
                    mips_codes.push_back("sw $s0, -" + to_string(4 * address) + "($sp)");
                } else if (address == 0) {
                    mips_codes.push_back("sw $s0, 0($sp)");
                }
            } else {
                mips_codes.push_back("addu $" + op3.name + ", $s0, $s1");
            }

        } else if (op2.tokenName == "" && op2.isNum == 0) {
            //op2是临时变量，op1不是临时变量
            long address = get_var_address(op2.symbol);
            string lw;
            if (address > 0) {
                //lw $t0, -20($sp)
                lw = "lw $s0, -" + to_string(4 * address) + "($sp)";
            } else if (address == 0) {
                lw = "lw $s0, 0($sp)";
            }
            mips_codes.push_back(lw);

            if (op3.tokenName == "" && op3.isNum == 0) {
                mips_codes.push_back("addu $s0, $s0, " + name1);
                address = get_var_address(op3.symbol);
                if (address > 0) {
                    //lw $t0, -20($sp)
                    mips_codes.push_back("sw $s0, -" + to_string(4 * address) + "($sp)");
                } else if (address == 0) {
                    mips_codes.push_back("sw $s0, 0($sp)");
                }
            } else {
                mips_codes.push_back("addu $" + op3.name + ", $s0, " + name1);
            }

        } else if (op1.tokenName == "" && op1.isNum == 0) {
            //op1是临时变量，op2不是临时变量
            long address = get_var_address(op1.symbol);
            string lw;
            if (address > 0) {
                //lw $t0, -20($sp)
                lw = "lw $s0, -" + to_string(4 * address) + "($sp)";
            } else if (address == 0) {
                lw = "lw $s0, 0($sp)";
            }
            mips_codes.push_back(lw);

            if (op3.tokenName == "" && op3.isNum == 0) {
                mips_codes.push_back("addu $s0, $s0, " + name2);
                address = get_var_address(op3.symbol);
                if (address > 0) {
                    //lw $t0, -20($sp)
                    mips_codes.push_back("sw $s0, -" + to_string(4 * address) + "($sp)");
                } else if (address == 0) {
                    mips_codes.push_back("sw $s0, 0($sp)");
                }
            } else {
                mips_codes.push_back("addu $" + op3.name + ", $s0, " + name2);
            }
        } else {
            //op1、op2均不是临时变量
            if (op3.tokenName == "" && op3.isNum == 0) {
                //op3是临时变量
                if (op1.tokenName == "" && op1.isNum == 1) {
                    add = "addu $s0, " + name2 + ", " + name1;
                } else {
                    add = "addu $s0, " + name1 + ", " + name2;
                }
                mips_codes.push_back(add);

                long address = get_var_address(op3.symbol);
                if (address > 0) {
                    //lw $t0, -20($sp)
                    mips_codes.push_back("sw $s0, -" + to_string(4 * address) + "($sp)");
                } else if (address == 0) {
                    mips_codes.push_back("sw $s0, 0($sp)");
                }
            } else {
                if (op1.tokenName == "" && op1.isNum == 1) {
                    add = "addu $" + op3.name + ", " + name2 + ", " + name1;
                } else {
                    add = "addu $" + op3.name + ", " + name1 + ", " + name2;
                }
                mips_codes.push_back(add);
            }
        }


//        string add;
//        if (op1.tokenName == "") {
//            add = "addu $" + op3.name + ", " + name2 + ", " + name1;
//        } else {
//            add = "addu $" + op3.name + ", " + name1 + ", " + name2;
//        }
//        mips_codes.push_back(add);
    } else if (opcode == "-") {
        string sub;
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            sub = "subu $" + op3.name + ", $s0, " + name2;
        } else {
            sub = "subu $" + op3.name + ", " + name1 + ", " + name2;
        }
        mips_codes.push_back(sub);
    } else if (opcode == "*") {
        string mul;
        if ((op1.isValueKnown == true && op1.value == 0) || (op2.isValueKnown == true && op2.value == 0)) {
            if (op3.tokenName == "" && op3.isNum == 0) {
                mips_codes.push_back("li $s0, 0");
                long op3_address = get_var_address(op3.symbol);
                mips_codes.push_back("sw $s0, -" + to_string(4 * op3_address) + "($sp)");
            } else {
                mips_codes.push_back("li $" + op3.name + ", 0");
            }
        } else {
            if ((op1.tokenName == "" && op1.isNum == 0) && (op2.tokenName == "" && op2.isNum == 0)) {
                //op1和op2都是因为寄存器不够而被分配的临时变量
                long address = get_var_address(op1.symbol);
                string lw;
                if (address > 0) {
                    //lw $t0, -20($sp)
                    lw = "lw $s0, -" + to_string(4 * address) + "($sp)";
                } else if (address == 0) {
                    lw = "lw $s0, 0($sp)";
                }
                mips_codes.push_back(lw);

                address = get_var_address(op2.symbol);
                if (address > 0) {
                    //lw $t0, -20($sp)
                    lw = "lw $s1, -" + to_string(4 * address) + "($sp)";
                } else if (address == 0) {
                    lw = "lw $s1, 0($sp)";
                }
                mips_codes.push_back(lw);

                if (op3.tokenName == "" && op3.isNum == 0) {
                    mips_codes.push_back("mul $s0, $s0, $s1");
                    address = get_var_address(op3.symbol);
                    if (address > 0) {
                        //lw $t0, -20($sp)
                        mips_codes.push_back("sw $s0, -" + to_string(4 * address) + "($sp)");
                    } else if (address == 0) {
                        mips_codes.push_back("sw $s0, 0($sp)");
                    }
                } else {
                    mips_codes.push_back("mul $" + op3.name + ", $s0, $s1");
                }

            } else if (op2.tokenName == "" && op2.isNum == false) {
                //op2是临时变量，op1不是临时变量
                long address = get_var_address(op2.symbol);
                string lw;
                if (address > 0) {
                    //lw $t0, -20($sp)
                    lw = "lw $s0, -" + to_string(4 * address) + "($sp)";
                } else if (address == 0) {
                    lw = "lw $s0, 0($sp)";
                }
                mips_codes.push_back(lw);

                if (op3.tokenName == "" && op3.isNum == 0) {
                    mips_codes.push_back("mul $s0, $s0, " + name1);
                    address = get_var_address(op3.symbol);
                    if (address > 0) {
                        //lw $t0, -20($sp)
                        mips_codes.push_back("sw $s0, -" + to_string(4 * address) + "($sp)");
                    } else if (address == 0) {
                        mips_codes.push_back("sw $s0, 0($sp)");
                    }
                } else {
                    mips_codes.push_back("mul $" + op3.name + ", $s0, " + name1);
                }

            } else if (op1.tokenName == "" && op1.isNum == 0) {
                //op1是临时变量，op2不是临时变量
                long address = get_var_address(op1.symbol);
                string lw;
                if (address > 0) {
                    //lw $t0, -20($sp)
                    lw = "lw $s0, -" + to_string(4 * address) + "($sp)";
                } else if (address == 0) {
                    lw = "lw $s0, 0($sp)";
                }
                mips_codes.push_back(lw);

                if (op3.tokenName == "" && op3.isNum == 0) {
                    mips_codes.push_back("mul $s0, $s0, " + name2);
                    address = get_var_address(op3.symbol);
                    if (address > 0) {
                        //lw $t0, -20($sp)
                        mips_codes.push_back("sw $s0, -" + to_string(4 * address) + "($sp)");
                    } else if (address == 0) {
                        mips_codes.push_back("sw $s0, 0($sp)");
                    }
                } else {
                    mips_codes.push_back("mul $" + op3.name + ", $s0, " + name2);
                }

            } else {
                //op1、op2均不是临时变量
                if (op3.tokenName == "" && op3.isNum == 0) {
                    //op3是临时变量
                    if (op1.tokenName == "" && op1.isNum == 1) {
                        mul = "mul $s0, " + name2 + ", " + name1;
                    } else {
                        mul = "mul $s0, " + name1 + ", " + name2;
                    }
                    mips_codes.push_back(mul);

                    long address = get_var_address(op3.symbol);
                    if (address > 0) {
                        //lw $t0, -20($sp)
                        mips_codes.push_back("sw $s0, -" + to_string(4 * address) + "($sp)");
                    } else if (address == 0) {
                        mips_codes.push_back("sw $s0, 0($sp)");
                    }
                } else {
                    if (op1.tokenName == "" && op1.isNum == 1) {
                        mul = "mul $" + op3.name + ", " + name2 + ", " + name1;
                    } else {
                        mul = "mul $" + op3.name + ", " + name1 + ", " + name2;
                    }
                    mips_codes.push_back(mul);
                }
            }
        }

    } else if (opcode == "/") {
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            mips_codes.push_back("div $s0, " + name2);
        } else if (op2.tokenName == "") {
            mips_codes.push_back("li $s0, " + name2);
            mips_codes.push_back("div " + name1 + ", $s0");
        } else {
            mips_codes.push_back("div " + name1 + ", " + name2);
        }
        mips_codes.push_back("mflo $" + op3.name);
//        if (op2.tokenName == "" && isPow(2, op2.value) == 1 && op2.value > 0) {
//            //除数是常数且是2的幂
//            long index = pow_index(2, op2.value);
//            string div = "srl $" + op3.name + ", " + name1 + ", " + to_string(index);
//            mips_codes.push_back(div);
//        } else {
//            if (op1.tokenName == "") {
//                mips_codes.push_back("li $s0, " + name1);
//                mips_codes.push_back("div $s0, " + name2);
//            } else if (op2.tokenName == "") {
//                mips_codes.push_back("li $s0, " + name2);
//                mips_codes.push_back("div " + name1 + ", $s0");
//            } else {
//                mips_codes.push_back("div " + name1 + ", " + name2);
//            }
//            mips_codes.push_back("mflo $" + op3.name);
//        }
    } else if (opcode == "%") {
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            mips_codes.push_back("div $s0, " + name2);
        } else if (op2.tokenName == "") {
            mips_codes.push_back("li $s0, " + name2);
            mips_codes.push_back("div " + name1 + ", $s0");
        } else {
            mips_codes.push_back("div " + name1 + ", " + name2);
        }
        mips_codes.push_back("mfhi $" + op3.name);
    } else if (opcode == "!") {
        //set equal
        mips_codes.push_back("seq $" + op3.name + ", $" + op2.name + ", 0");
    }
}

void Compiler::relation(Quaternary q) {
    string opcode = q.opcode;
    Operand op1 = q.op1_vec[0];
    Operand op2 = q.op2_vec[0];
    Operand op3 = q.op3_vec[0];
    string name1 = op1.tokenName == "" ? op1.name : "$" + op1.name;
    string name2 = op2.tokenName == "" ? op2.name : "$" + op2.name;
    if (q.opcode == "<") {
        string less;
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            less = "slt $" + op3.name + ", $s0, " + name2;
        } else if (op2.tokenName == "") {
            mips_codes.push_back("li $s0, " + name2);
            less = "slt $" + op3.name + ", " + name1 + ", $s0";
        } else {
            less = "slt $" + op3.name + ", " + name1 + ", " + name2;
        }
        mips_codes.push_back(less);
    } else if (q.opcode == ">") {
        string greater;
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            greater = "sgt $" + op3.name + ", $s0, " + name2;
        } else {
            greater = "sgt $" + op3.name + ", " + name1 + ", " + name2;
        }
        mips_codes.push_back(greater);
    } else if (q.opcode == "<=") {
        string le;
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            le = "sle $" + op3.name + ", $s0, " + name2;
        } else {
            le = "sle $" + op3.name + ", " + name1 + ", " + name2;
        }
        mips_codes.push_back(le);
    } else if (q.opcode == ">=") {
        string ge;
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            ge = "sge $" + op3.name + ", $s0, " + name2;
        } else {
            ge = "sge $" + op3.name + ", " + name1 + ", " + name2;
        }
        mips_codes.push_back(ge);
    } else if (q.opcode == "==") {
        string equal;
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            equal = "seq $" + op3.name + ", $s0, " + name2;
        } else {
            equal = "seq $" + op3.name + ", " + name1 + ", " + name2;
        }
        mips_codes.push_back(equal);
    } else if (q.opcode == "!=") {
        string notEqual;
        if (op1.tokenName == "") {
            mips_codes.push_back("li $s0, " + name1);
            notEqual = "sne $" + op3.name + ", $s0, " + name2;
        } else {
            notEqual = "sne $" + op3.name + ", " + name1 + ", " + name2;
        }
        mips_codes.push_back(notEqual);
    }
}

void Compiler::add_global(Symbol symbol) {
    if (symbol.dimension == 0) {
        global_address.push_back(VarAddress(symbol, global_top++));
    } else if (symbol.dimension == 1) {
        global_address.push_back(VarAddress(symbol, global_top));
        global_top += symbol.di_len[0];
    } else if (symbol.dimension == 2) {
        global_address.push_back(VarAddress(symbol, global_top));
        global_top += symbol.di_len[0] * symbol.di_len[1];
    }
}

void Compiler::add_var(Symbol symbol) {
    if (symbol.dimension == 0) {
        var_sp.push_back(VarAddress(symbol, ++sp_top));
    } else if (symbol.dimension == 1) {
        sp_top += symbol.di_len[0];
        var_sp.push_back(VarAddress(symbol, sp_top));
    } else if (symbol.dimension == 2) {
        sp_top += symbol.di_len[0] * symbol.di_len[1];
        var_sp.push_back(VarAddress(symbol, sp_top));
    }
}

void Compiler::add_FParam(Symbol symbol) {
    /**
     * 对于 int 类型的参数，遵循按值传递；
     * 对于数组类型的参数，则形参接收的是实参数组的地址，并通过地址间接访问实参数组中的元素
     */
    var_sp.push_back(VarAddress(symbol, ++sp_top));

}

void Compiler::endBlock() {
    int tmp = levStack.top();
    levStack.pop();
    int i = var_sp.size() - 1;
    for (; i >= 0; --i) {
        VarAddress s = var_sp[i];
        if (s.symbol.lev != tmp) {
            break;
        }
    }
    var_sp.erase(var_sp.begin() + (i + 1), var_sp.end());
    sp_top = var_sp.size() == 0 ? -1 : var_sp[var_sp.size() - 1].address;
}

long Compiler::get_global_address(Symbol symbol) {
    for (int i = global_address.size() - 1; i >= 0; --i) {
        VarAddress tmp = global_address[i];
        if (tmp.symbol.name == symbol.name) {
            return tmp.address;
        }
    }
    return -1;
}

long Compiler::get_var_address(Symbol symbol) {
    for (int i = var_sp.size() - 1; i >= 0; --i) {
        VarAddress tmp = var_sp[i];
        if (tmp.symbol.name == symbol.name) {
            return tmp.address;
        }
    }
    return -1;
}

void Compiler::store_regs() {
    for (int i = 0; i <= 9; ++i) {
        Symbol reg_t;
        reg_t.name = "$t" + to_string(i);

        var_sp.push_back(VarAddress(reg_t, ++sp_top));
        if (sp_top > 0) {
            mips_codes.push_back("sw " + reg_t.name + ", -" + to_string(sp_top * 4) + "($sp)");
        } else {
            mips_codes.push_back("sw " + reg_t.name + ", 0($sp)");
        }
    }
    for (int i = 2; i <= 7; ++i) {
        Symbol reg_s;
        reg_s.name = "$s" + to_string(i);

        var_sp.push_back(VarAddress(reg_s, ++sp_top));
        if (sp_top > 0) {
            mips_codes.push_back("sw " + reg_s.name + ", -" + to_string(sp_top * 4) + "($sp)");
        } else {
            mips_codes.push_back("sw " + reg_s.name + ", 0($sp)");
        }
    }
}