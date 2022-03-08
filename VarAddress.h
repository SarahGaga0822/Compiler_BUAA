//
// Created by Sarah on 2021/11/16.
//

#ifndef GENERATEMIPS_VARADDRESS_H
#define GENERATEMIPS_VARADDRESS_H

class VarAddress {
public:
    Symbol symbol;
    long address;

    VarAddress(Symbol symbol, long address) : symbol(symbol), address(address) {}
};

#endif //GENERATEMIPS_VARADDRESS_H
