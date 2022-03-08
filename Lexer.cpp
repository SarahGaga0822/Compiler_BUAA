#include<iostream>
#include<fstream>
#include<string>
#include<set>
#include<map>
#include<cctype>

#include "Compiler.h"

using namespace std;

#define MAXN 1024

int Compiler :: getSym(int preRead) {
    curWord.clear();

    if (getChar(preRead) == -1) {
        return -1;
    }

    while (curChar == ' ' || curChar == '\n' || curChar == '\t' || curChar == '\r') {
        getChar(preRead);
    }

    if (isLetter(curChar)) {
        while (isLetter(curChar) || isdigit(curChar)) {
            curWord += curChar;
            getChar(preRead);
        }
        --curIndex;
        if (isReserved(curWord) == 1) {
            curSym = RESERVE;
            //auto iter = reservedWords.find(curWord);
            //outfile << (*iter).second << ' ' << curWord << endl;
        }
        else {
            curSym = IDENT;
            //outfile << "IDENFR" << ' ' << curWord << endl;
        }
    }

    else if (isdigit(curChar)) {
        while (isdigit(curChar)) {
            curWord += curChar;
            getChar(preRead);
        }
        curSym = INTCON;
        curIndex--;
        //outfile << "INTCON" << ' ' << curWord << endl;
        //curSym = INTCON;
    }

    else if (curChar == '\"') {
        curWord += curChar;
        getChar(preRead);
        while (curChar != '\"') {
            curWord += curChar;
            getChar(preRead);
        }
        curWord += curChar;
        curSym = STRCON;
        //outfile << "STRCON" << ' ' << curWord << endl;
    }

    else if (curChar == '+' || curChar == '-' || curChar == '*' || curChar == '%') {
        curWord += curChar;
        curSym = ARITHOPER;
        //auto iter = arithOpers.find(curWord);       
        //outfile << (*iter).second << ' ' << curWord << endl;
    }

    else if (curChar == '/') {
        curWord += curChar;
        //getChar();
        if (getChar(preRead) == 1) {
            curChar = '\n';
            --curIndex;
        }
        if (curChar == '/') {
            curWord += curChar;
            //getline(infile, curLine);
            curLineLength = curLine.length();
            curIndex = curLineLength;
            curSym = ANNOTATION;
        }
        else if (curChar == '*') {
            curWord += curChar;
            //getChar();
            if (getChar(preRead) == 1) {
                curChar = '\n';
                --curIndex;
            }
            curWord += curChar;
            while (true) {
                while (curChar != '*') {
                    //getChar();
                    if (getChar(preRead) == 1) {
                        curChar = '\n';
                        --curIndex;
                    }
                    curWord += curChar;
                }

                //getChar();
                if (getChar(preRead) == 1) {
                    curChar = '\n';
                    --curIndex;
                }
                curWord += curChar;
                if (curChar == '/') {
                    break;
                }
            }
            curSym = ANNOTATION;
        }
        else {
            --curIndex;
            curSym = ARITHOPER;
            //outfile << "DIV" << ' ' << curWord << endl;
        }
    }

    else if (curChar == ';') {
        curWord += curChar;
        curSym = SEMICN;
        //outfile << "SEMICN" << ' ' << curWord << endl;
    }

    else if (curChar == ',') {
        curWord += curChar;
        curSym = COMMA;
        //outfile << "COMMA" << ' ' << curWord << endl;
    }

    else if (curChar == '(' || curChar == ')' ||
        curChar == '[' || curChar == ']' ||
        curChar == '{' || curChar == '}') {
        curWord += curChar;
        curSym = BRACKETCHAR;
        //auto iter = bracketChars.find(curWord);
        //outfile << (*iter).second << ' ' << curWord << endl;
    }

    else if (curChar == '<') {
        curWord += curChar;
        getChar(preRead);
        if (curChar == '=') {
            curWord += curChar;
            curSym = LEQ;
            //outfile << "LEQ" << ' ' << curWord << endl;
        }
        else {
            curIndex--;
            curSym = LSS;
            //outfile << "LSS" << ' ' << curWord << endl;
        }
    }

    else if (curChar == '>') {
        curWord += curChar;
        getChar(preRead);
        if (curChar == '=') {
            curWord += curChar;
            curSym = GEQ;
            //outfile << "GEQ" << ' ' << curWord << endl;
        }
        else {
            curIndex--;
            curSym = GRE;
            //outfile << "GRE" << ' ' << curWord << endl;
        }
    }

    else if (curChar == '!') {
        curWord += curChar;
        getChar(preRead);
        if (curChar == '=') {
            curWord += curChar;
            curSym = NEQ;
            //outfile << "NEQ" << ' ' << curWord << endl;
        }
        else {
            curIndex--;
            curSym = NOT;
            //outfile << "NOT" << ' ' << curWord << endl;
        }
    }

    else if (curChar == '=') {
        curWord += curChar;
        getChar(preRead);
        if (curChar == '=') {
            curWord += curChar;
            curSym = EQL;
            //outfile << "EQL" << ' ' << curWord << endl;
        }
        else {
            curIndex--;
            curSym = ASSIGN;
            //outfile << "ASSIGN" << ' ' << curWord << endl;
        }
    }

    else if (curChar == '&') {
        curWord += curChar;
        getChar(preRead);
        if (curChar == '&') {
            curWord += curChar;
            curSym = AND;
            //outfile << "AND" << ' ' << curWord << endl;
        }
        else {
            curSym = SINGLEAND;
            curIndex--;
        }
    }

    else if (curChar == '|') {
        curWord += curChar;
        getChar(preRead);
        if (curChar == '|') {
            curWord += curChar;
            curSym = OR;
            //outfile << "OR" << ' ' << curWord << endl;
        }
        else {
            curSym = SINGLEOR;
            curIndex--;
        }
    }

    if (preRead != 1) {
        LexerOutput();
    }

    if (curLineNum > code.size() && curIndex >= curLineLength) {
        return -1;
    }
    
    while (curSym == ANNOTATION) {
        getSym(preRead);
    }


    return 0;
    /*
    while (getChar() == 1) {

    }
    */
    
}

int Compiler :: isLetter(char c) {
    if (isalpha(c) || c == '_') {
        return 1;
    }
    return 0;
}

int Compiler :: isReserved(string s) {
    if (reservedWords.find(s) != reservedWords.end()) {
        return 1;
    }
    return 0;
}

void Compiler :: LexerOutput() {
    if (curSym == RESERVE) {
        auto iter = reservedWords.find(curWord);
        outfile1 << (*iter).second << ' ' << curWord << endl;
    } else if (curSym == IDENT) {
        outfile1 << "IDENFR" << ' ' << curWord << endl;
    } else if (curSym == INTCON) {
        outfile1 << "INTCON" << ' ' << curWord << endl;
    } else if (curSym == STRCON) {
        outfile1 << "STRCON" << ' ' << curWord << endl;
    } else if (curSym == ARITHOPER) {
        auto iter = arithOpers.find(curWord);       
        outfile1 << (*iter).second << ' ' << curWord << endl;
    } else if (curSym == SEMICN) {
        outfile1 << "SEMICN" << ' ' << curWord << endl;
    } else if (curSym == COMMA) {
        outfile1 << "COMMA" << ' ' << curWord << endl;
    } else if (curSym == BRACKETCHAR) {
        auto iter = bracketChars.find(curWord);
        outfile1 << (*iter).second << ' ' << curWord << endl;
    } else if (curSym == LEQ) {
        outfile1 << "LEQ" << ' ' << curWord << endl;
    } else if (curSym == LSS) {
        outfile1 << "LSS" << ' ' << curWord << endl;
    } else if (curSym == GEQ) {
        outfile1 << "GEQ" << ' ' << curWord << endl;
    } else if (curSym == GRE) {
        outfile1 << "GRE" << ' ' << curWord << endl;
    } else if (curSym == NEQ) {
        outfile1 << "NEQ" << ' ' << curWord << endl;
    } else if (curSym == NOT) {
        outfile1 << "NOT" << ' ' << curWord << endl;
    } else if (curSym == EQL) {
        outfile1 << "EQL" << ' ' << curWord << endl;
    } else if (curSym == ASSIGN) {
        outfile1 << "ASSIGN" << ' ' << curWord << endl;
    } else if (curSym == AND) {
        outfile1 << "AND" << ' ' << curWord << endl;
    } else if (curSym == OR) {
        outfile1 << "OR" << ' ' << curWord << endl;
    }

}

int Compiler :: preRead(int times) {
    //times表示预读的单词数量
    int tmpLineLength = curLineLength;
    int tmpLineNum = curLineNum;
    string tmpLine = curLine;
    int tmpIndex = curIndex;
    string tmpWord = curWord;
    int tmpSym = curSym;
    getSym(1);
    firstWord = curWord;
    firstSym = curSym;
    firstLineNum = curLineNum;
    if (times >= 2) {
        getSym(1);
        secondWord = curWord;
    } 
    if (times >= 3) {
        getSym(1);
        thirdWord = curWord;
    }
    curLineLength = tmpLineLength;
    curLineNum = tmpLineNum;
    curLine = tmpLine;
    curIndex = tmpIndex;
    curWord = tmpWord;
    curSym = tmpSym;
    if ((firstWord.length() == 0 && times >= 1) || 
        (secondWord.length() == 0 && times >= 2) || 
        (thirdWord.length() == 0 && times >= 3)) {
        return -1;
    }
    return 0;
}


int Compiler :: getChar(int preRead) {
    //1代表预读
    /*
    int flag = 0;   
    while (curLineLength == 0 || curIndex >= curLineLength) {
        if (infile.eof() && (curIndex >= curLineLength)) {
            return -1;
            
        } else {
            flag = 1;
            ++curLineNum;
            curIndex = 0;
            getline(infile, curLine);
            //curLine += '\n';
            curLineLength = curLine.length();
        }
    }
    
    curChar = curLine.at(curIndex++);
    cout << curChar << ' ' << curLineNum << endl;
    if (flag == 1) {
        return 1;
    }
    return 0;
    */

    int flag = 0;
    while (curLineLength == 0 || curIndex >= curLineLength) {
        if (curLineNum >= code.size() && curIndex >= curLineLength) {
            return -1;
        }
        else {
            flag = 1;
            ++curLineNum;
            curIndex = 0;
            curLine = code[curLineNum - 1];
            curLineLength = curLine.length();
        }
    }

    curChar = curLine.at(curIndex++);
    cout << curChar << ' ' << curLineNum << endl;
    if (flag == 1) {
        return 1;
    }
    return 0;
}


