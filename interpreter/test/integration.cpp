#include <cassert>
#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../../compiler/compiler.hpp"
#include "../architecture.hpp"

using namespace std;
using namespace interpreter;

vector<Test> tests = {
    Test("factorial", []() {
        Architecture arch;
        string code =
            "int fact(int x) {\n"
            "    if (x <= 1) {return 1;}\n"
            "    return x*fact(x-1);\n"
            "}\n"
            "fact(7);\n"
        ;
        arch.load_program(compiler::compile(code));
        arch.run();
        cout << arch.reg[reg_t::RES] << '\n';
        assert(arch.reg[reg_t::RES] == 5040);
    }),
    Test("fibonacci", []() {
        Architecture arch;
        string code =
            "int fib(int n) {\n"
            "    if (n == 0) {return 0;}\n"
            "    if (n == 1) {return 1;}\n"
            "    return fib(n-1) + fib(n-2);\n"
            "}\n"
            "fib(20);\n"
        ;
        arch.load_program(compiler::compile(code));
        double t0 = time();
        arch.run();
        double t1 = time();
        cout << "Prgoram ran in: " << (t1-t0)*1000 << "ms\n";
        cout << arch.reg[reg_t::RES] << '\n';
        assert(arch.reg[reg_t::RES] == 6765);
    }),
    Test("stress", []() {
        Architecture arch;
        string code =
            "int fib(int n) {\n"
            "    if (n == 0) {return 0;}\n"
            "    if (n == 1) {return 1;}\n"
            "    return fib(n-1) + fib(n-2);\n"
            "}\n"
            "fib(34);\n"
        ;
        arch.load_program(compiler::compile(code));
        double t0 = time();
        arch.run();
        double t1 = time();
        cout << "Prgoram ran in: " << (t1-t0)*1000 << "ms\n";
        cout << arch.reg[reg_t::RES] << '\n';
    })
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    cout << setprecision(15);

    compiler::init();

    for (const Test& test: tests) {
        test.run();
    }

    return 0;
}