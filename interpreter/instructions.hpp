#pragma once

#include "base.hpp"
#include "registers.hpp"

namespace interpreter {
    enum class inst_t: uint8_t {
        no_op, exit,
        jump, jump_r, bnez, beqz, call, call_r, ret,
        mv, load_imm,
        add, sub, mul, div, mod, addi, muli, divi, modi,
        b_sl, b_sr, b_and, b_or, b_xor, b_not, b_sli, b_sri, b_andi, b_ori, b_xori, b_noti,
        eq, ne, lt, le,
        store_64, load_64,
        store_32, load_32, load_32u,
        store_16, load_16, load_16u,
        store_8, load_8, load_8u,
        syscall
    };

    struct inst {
        uint64_t bits;

        inst() : bits(0) {}
        inst(inst_t type, reg_t rd, reg_t rs1, reg_t rs2, int32_t imm) {
            bits = uint64_t(type) | uint64_t(rd) << 8
                | uint64_t(rs1) << 16 | uint64_t(rs2) << 24
                | (uint64_t(imm) << 32);
        }
        
        inst_t type() const {return inst_t(bits & 0xff);}
        reg_t rd() const {return reg_t((bits >> 8) & 0xff);}
        reg_t rs1() const {return reg_t((bits >> 16) & 0xff);}
        reg_t rs2() const {return reg_t((bits >> 24) & 0xff);}
        int32_t imm() const {return int32_t(bits >> 32);}

        static inst exit() {
            return inst(inst_t::exit, reg_t::ZERO, reg_t::ZERO, reg_t::ZERO, 0);
        }
        static inst jump(int32_t d_pos) {
            return inst(inst_t::jump, reg_t::ZERO, reg_t::ZERO, reg_t::ZERO, d_pos);
        }
        static inst jump_r(reg_t rs1) {
            return inst(inst_t::jump_r, reg_t::ZERO, rs1, reg_t::ZERO, 0);
        }
        static inst bnez(reg_t rs1, int32_t d_pos) {
            return inst(inst_t::bnez, reg_t::ZERO, rs1, reg_t::ZERO, d_pos);
        }
        static inst beqz(reg_t rs1, int32_t d_pos) {
            return inst(inst_t::beqz, reg_t::ZERO, rs1, reg_t::ZERO, d_pos);
        }
        static inst call(int32_t d_pos) {
            return inst(inst_t::call, reg_t::ZERO, reg_t::ZERO, reg_t::ZERO, d_pos);
        }
        static inst call_r(reg_t rs1) {
            return inst(inst_t::call_r, reg_t::ZERO, rs1, reg_t::ZERO, 0);
        }
        static inst ret() {
            return inst(inst_t::ret, reg_t::ZERO, reg_t::ZERO, reg_t::ZERO, 0);
        }
        static inst mv(reg_t rd, reg_t rs1) {
            return inst(inst_t::mv, rd, rs1, reg_t::ZERO, 0);
        }
        static inst load_imm(reg_t rd, int32_t imm) {
            return inst(inst_t::load_imm, rd, reg_t::ZERO, reg_t::ZERO, imm);
        }
        static inst add(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::add, rd, rs1, rs2, 0);
        }
        static inst sub(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::sub, rd, rs1, rs2, 0);
        }
        static inst mul(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::mul, rd, rs1, rs2, 0);
        }
        static inst div(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::div, rd, rs1, rs2, 0);
        }
        static inst mod(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::mod, rd, rs1, rs2, 0);
        }
        static inst addi(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::addi, rd, rs1, reg_t::ZERO, imm);
        }
        static inst muli(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::muli, rd, rs1, reg_t::ZERO, imm);
        }
        static inst divi(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::divi, rd, rs1, reg_t::ZERO, imm);
        }
        static inst modi(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::modi, rd, rs1, reg_t::ZERO, imm);
        }
        static inst b_sl(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::b_sl, rd, rs1, rs2, 0);
        }
        static inst b_sr(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::b_sr, rd, rs1, rs2, 0);
        }
        static inst b_and(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::b_and, rd, rs1, rs2, 0);
        }
        static inst b_or(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::b_or, rd, rs1, rs2, 0);
        }
        static inst b_xor(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::b_xor, rd, rs1, rs2, 0);
        }
        static inst b_not(reg_t rd, reg_t rs1) {
            return inst(inst_t::b_not, rd, rs1, reg_t::ZERO, 0);
        }
        static inst b_sli(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::b_sli, rd, rs1, rs2, 0);
        }
        static inst b_sri(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::b_sri, rd, rs1, rs2, 0);
        }
        static inst b_andi(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::b_andi, rd, rs1, reg_t::ZERO, imm);
        }
        static inst b_ori(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::b_ori, rd, rs1, reg_t::ZERO, imm);
        }
        static inst b_xori(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::b_xori, rd, rs1, reg_t::ZERO, imm);
        }
        static inst b_noti(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::b_noti, rd, rs1, reg_t::ZERO, imm);
        }
        static inst eq(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::eq, rd, rs1, rs2, 0);
        }
        static inst ne(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::ne, rd, rs1, rs2, 0);
        }
        static inst lt(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::lt, rd, rs1, rs2, 0);
        }
        static inst le(reg_t rd, reg_t rs1, reg_t rs2) {
            return inst(inst_t::le, rd, rs1, rs2, 0);
        }
        static inst store_64(reg_t rs2, int32_t imm, reg_t rs1) {
            return inst(inst_t::store_64, reg_t::ZERO, rs1, rs2, imm);
        }
        static inst load_64(reg_t rd, int32_t imm, reg_t rs1) {
            return inst(inst_t::load_64, rd, rs1, reg_t::ZERO, imm);
        }
        static inst store_32(reg_t rs2, int32_t imm, reg_t rs1) {
            return inst(inst_t::store_32, reg_t::ZERO, rs1, rs2, imm);
        }
        static inst load_32(reg_t rd, int32_t imm, reg_t rs1) {
            return inst(inst_t::load_32, rd, rs1, reg_t::ZERO, imm);
        }
        static inst load_32u(reg_t rd, int32_t imm, reg_t rs1) {
            return inst(inst_t::load_32u, rd, rs1, reg_t::ZERO, imm);
        }
        static inst store_16(reg_t rs2, int32_t imm, reg_t rs1) {
            return inst(inst_t::store_16, reg_t::ZERO, rs1, rs2, imm);
        }
        static inst load_16(reg_t rd, int32_t imm, reg_t rs1) {
            return inst(inst_t::load_16, rd, rs1, reg_t::ZERO, imm);
        }
        static inst load_16u(reg_t rd, int32_t imm, reg_t rs1) {
            return inst(inst_t::load_16u, rd, rs1, reg_t::ZERO, imm);
        }
        static inst store_8(reg_t rs2, int32_t imm, reg_t rs1) {
            return inst(inst_t::store_8, reg_t::ZERO, rs1, rs2, imm);
        }
        static inst load_8(reg_t rd, int32_t imm, reg_t rs1) {
            return inst(inst_t::load_8, rd, rs1, reg_t::ZERO, imm);
        }
        static inst load_8u(reg_t rd, int32_t imm, reg_t rs1) {
            return inst(inst_t::load_8u, rd, rs1, reg_t::ZERO, imm);
        }
    };
}