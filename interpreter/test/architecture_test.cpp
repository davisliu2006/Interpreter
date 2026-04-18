#include <cassert>
#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../architecture.hpp"

using namespace std;
using namespace interpreter;

vector<Test> tests = {
    Test("factorial", []() {
        Architecture arch;
        arch.inst_mem = {
            inst::load_imm(reg_t::T1, 7),
            inst::load_imm(reg_t::RES, 1),
            inst::beqz(reg_t::T1, 4),
            inst::mul(reg_t::RES, reg_t::RES, reg_t::T1),
            inst::addi(reg_t::T1, reg_t::T1, -1),
            inst::jump(-3),
            inst::exit()
        };
        arch.run();
        cout << arch.reg[reg_t::RES] << '\n';
        assert(arch.reg[reg_t::RES] == 5040);
    })
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    cout << setprecision(15);

    for (const Test& test: tests) {
        test.run();
    }

    return 0;
}