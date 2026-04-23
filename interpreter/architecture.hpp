#pragma once

#include "base.hpp"
#include "instructions.hpp"
#include "registers.hpp"

namespace interpreter {
    struct Architecture {
        vector<inst> inst_mem;
        vector<int8_t> const_mem;
        vector<int8_t> stack_mem;
        vector<int8_t> heap_mem;
        vector<int8_t> expr_stack;
        vector<int8_t> syscall_buf;
        Registers reg;
        
        Architecture() {
            inst_mem.resize(2048);
            const_mem.resize(2048);
            stack_mem.resize(2048);
            heap_mem.resize(2048);
            expr_stack.resize(128);

            reg.inst_ptr = inst_mem.data();
            reg.stk_ptr() = stack_mem.data()+stack_mem.size();
            reg.heap_ptr() = heap_mem.data();
            reg.expr_ptr() = expr_stack.data();
        }

        void execute(inst& instruction) {
            switch (instruction.type()) {
                case inst_t::no_op: {break;}
                case inst_t::exit: {return;}
                case inst_t::jump: {
                    reg.inst_ptr += instruction.imm()-1;
                    break;
                }
                case inst_t::jump_r: {
                    reg.inst_ptr = (inst*)(reg[instruction.rs1()]-1);
                    break;
                }
                case inst_t::bnez: {
                    if (reg[instruction.rs1()] != 0) {
                        reg.inst_ptr += instruction.imm()-1;
                    }
                    break;
                }
                case inst_t::beqz: {
                    if (reg[instruction.rs1()] == 0) {
                        reg.inst_ptr += instruction.imm()-1;
                    }
                    break;
                }
                case inst_t::call: {
                    reg[reg_t::RA] = int64_t(reg.inst_ptr);
                    reg.inst_ptr += instruction.imm()-1;
                    break;
                }
                case inst_t::call_r: {
                    reg[reg_t::RA] = int64_t(reg.inst_ptr);
                    reg.inst_ptr = (inst*)(reg[instruction.rs1()]-1);
                    break;
                }
                case inst_t::ret: {
                    reg.inst_ptr = (inst*)(reg[reg_t::RA]);
                    break;
                }
                case inst_t::mv: {
                    reg[instruction.rd()] = reg[instruction.rs1()];
                    break;
                }
                case inst_t::load_imm: {
                    reg[instruction.rd()] = instruction.imm();
                    break;
                }
                case inst_t::add: {
                    reg[instruction.rd()] = reg[instruction.rs1()]+reg[instruction.rs2()];
                    break;
                }
                case inst_t::sub: {
                    reg[instruction.rd()] = reg[instruction.rs1()]-reg[instruction.rs2()];
                    break;
                }
                case inst_t::mul: {
                    reg[instruction.rd()] = reg[instruction.rs1()]*reg[instruction.rs2()];
                    break;
                }
                case inst_t::div: {
                    reg[instruction.rd()] = reg[instruction.rs1()]/reg[instruction.rs2()];
                    break;
                }
                case inst_t::mod: {
                    reg[instruction.rd()] = reg[instruction.rs1()]%reg[instruction.rs2()];
                    break;
                }
                case inst_t::addi: {
                    reg[instruction.rd()] = reg[instruction.rs1()]+instruction.imm();
                    break;
                }
                case inst_t::muli: {
                    reg[instruction.rd()] = reg[instruction.rs1()]*instruction.imm();
                    break;
                }
                case inst_t::divi: {
                    reg[instruction.rd()] = reg[instruction.rs1()]/instruction.imm();
                    break;
                }
                case inst_t::modi: {
                    reg[instruction.rd()] = reg[instruction.rs1()]%instruction.imm();
                    break;
                }
                case inst_t::b_sl: {
                    reg[instruction.rd()] = reg[instruction.rs1()] << reg[instruction.rs2()];
                    break;
                }
                case inst_t::b_sr: {
                    reg[instruction.rd()] = reg[instruction.rs1()] >> reg[instruction.rs2()];
                    break;
                }
                case inst_t::b_and: {
                    reg[instruction.rd()] = reg[instruction.rs1()] & reg[instruction.rs2()];
                    break;
                }
                case inst_t::b_or: {
                    reg[instruction.rd()] = reg[instruction.rs1()] | reg[instruction.rs2()];
                    break;
                }
                case inst_t::b_xor: {
                    reg[instruction.rd()] = reg[instruction.rs1()] ^ reg[instruction.rs2()];
                    break;
                }
                case inst_t::b_not: {
                    reg[instruction.rd()] = ~reg[instruction.rs1()];
                    break;
                }
                case inst_t::b_sli: {
                    reg[instruction.rd()] = reg[instruction.rs1()] << instruction.imm();
                    break;
                }
                case inst_t::b_sri: {
                    reg[instruction.rd()] = reg[instruction.rs1()] >> instruction.imm();
                    break;
                }
                case inst_t::b_andi: {
                    reg[instruction.rd()] = reg[instruction.rs1()] & instruction.imm();
                    break;
                }
                case inst_t::b_ori: {
                    reg[instruction.rd()] = reg[instruction.rs1()] | instruction.imm();
                    break;
                }
                case inst_t::b_xori: {
                    reg[instruction.rd()] = reg[instruction.rs1()] ^ instruction.imm();
                    break;
                }
                case inst_t::b_noti: {
                    reg[instruction.rd()] = ~instruction.imm();
                    break;
                }
                case inst_t::eq: {
                    reg[instruction.rd()] = (reg[instruction.rs1()] == reg[instruction.rs2()]);
                    break;
                }
                case inst_t::ne: {
                    reg[instruction.rd()] = (reg[instruction.rs1()] != reg[instruction.rs2()]);
                    break;
                }
                case inst_t::lt: {
                    reg[instruction.rd()] = (reg[instruction.rs1()] < reg[instruction.rs2()]);
                    break;
                }
                case inst_t::le: {
                    reg[instruction.rd()] = (reg[instruction.rs1()] <= reg[instruction.rs2()]);
                    break;
                }
                case inst_t::store_64: {
                    int64_t& trg = *(int64_t*)(reg[instruction.rs1()]+instruction.imm());
                    trg = reg[instruction.rs2()];
                    break;
                }
                case inst_t::load_64: {
                    int64_t val = *(int64_t*)(reg[instruction.rs1()]+instruction.imm());
                    reg[instruction.rd()] = val;
                    break;
                }
                case inst_t::store_32: {
                    int32_t& trg = *(int32_t*)(reg[instruction.rs1()]+instruction.imm());
                    trg = reg[instruction.rs2()];
                    break;
                }
                case inst_t::load_32: {
                    int32_t val = *(int32_t*)(reg[instruction.rs1()]+instruction.imm());
                    reg[instruction.rd()] = val;
                    break;
                }
                case inst_t::load_32u: {
                    uint32_t val = *(uint32_t*)(reg[instruction.rs1()]+instruction.imm());
                    reg[instruction.rd()] = val;
                    break;
                }
                case inst_t::store_16: {
                    int16_t& trg = *(int16_t*)(reg[instruction.rs1()]+instruction.imm());
                    trg = reg[instruction.rs2()];
                    break;
                }
                case inst_t::load_16: {
                    int16_t val = *(int16_t*)(reg[instruction.rs1()]+instruction.imm());
                    reg[instruction.rd()] = val;
                    break;
                }
                case inst_t::load_16u: {
                    uint16_t val = *(uint16_t*)(reg[instruction.rs1()]+instruction.imm());
                    reg[instruction.rd()] = val;
                    break;
                }
                case inst_t::store_8: {
                    int8_t& trg = *(int8_t*)(reg[instruction.rs1()]+instruction.imm());
                    trg = reg[instruction.rs2()];
                    break;
                }
                case inst_t::load_8: {
                    int8_t val = *(int8_t*)(reg[instruction.rs1()]+instruction.imm());
                    reg[instruction.rd()] = val;
                    break;
                }
                case inst_t::load_8u: {
                    uint8_t val = *(uint8_t*)(reg[instruction.rs1()]+instruction.imm());
                    reg[instruction.rd()] = val;
                    break;
                }
                case inst_t::push_expr: {
                    reg.expr_ptr() += 8;
                    *(int64_t*)(reg.expr_ptr()) = reg[instruction.rs1()];
                    break;
                }
                case inst_t::pop_expr: {
                    reg[instruction.rd()] = *(int64_t*)(reg.expr_ptr());
                    reg.expr_ptr() -= 8;
                    break;
                }
                case inst_t::syscall: {
                    std::cout << "syscall: " << reg[instruction.rs1()] << std::endl;
                    break;
                }
            }
            reg.inst_ptr++;
        }
        void run() {
            while (reg.inst_ptr->type() != inst_t::exit) {
                std::cout << reg.inst_ptr-inst_mem.data() << '\n';
                execute(*reg.inst_ptr);
            }
        }
    };
}