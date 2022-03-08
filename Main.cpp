#include<iostream>
#include<fstream>
using namespace std;

#include "Compiler.h"

int main() {
    string inPath, outPath1, outPath2, outPath3, outPath4, outPath5;

    inPath = "D:\\Compiler\\experiment\\Sarah\\GenerateCode\\GenerateMIPS7\\testfile.txt";
    outPath1 = "D:\\Compiler\\experiment\\Sarah\\GenerateCode\\GenerateMIPS7\\syntax.txt";
    outPath2 = "D:\\Compiler\\experiment\\Sarah\\GenerateCode\\GenerateMIPS7\\error.txt";
    outPath3 = "D:\\Compiler\\experiment\\Sarah\\GenerateCode\\GenerateMIPS7\\midCodes.txt";
    outPath4 = "D:\\Compiler\\experiment\\Sarah\\GenerateCode\\GenerateMIPS7\\mips.txt";
    outPath5 = "D:\\Compiler\\experiment\\Sarah\\GenerateCode\\GenerateMIPS7\\output.txt";

    Compiler compiler(inPath, outPath1, outPath2, outPath3, outPath4, outPath5);
    compiler.Process();

    return 0;
}