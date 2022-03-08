cmake_minimum_required(VERSION 3.10)
project(GenerateMIPS7)

set(CMAKE_CXX_STANDARD 14)

add_executable(GenerateMIPS7
        Compiler.cpp
        Compiler.h
        definition.h
        Error.cpp
        Lexer.cpp
        Main.cpp
        Symbol.h
        Syntax.cpp
        Quaternary.h Operand.h
        Utils.cpp
        GenerateMIPS.cpp
        VarAddress.h)

