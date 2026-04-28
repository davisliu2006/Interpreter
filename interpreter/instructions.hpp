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
        push_expr, pop_expr,
        syscall
    };

    inline string to_string(inst_t x) {
        switch (x) {
            case inst_t::no_op: {return "no_op";}
            case inst_t::exit: {return "exit";}
            case inst_t::jump: {return "jump";}
            case inst_t::jump_r: {return "jump_r";}
            case inst_t::bnez: {return "bnez";}
            case inst_t::beqz: {return "beqz";}
            case inst_t::call: {return "call";}
            case inst_t::call_r: {return "call_r";}
            case inst_t::ret: {return "ret";}
            case inst_t::mv: {return "mv";}
            case inst_t::load_imm: {return "load_imm";}
            case inst_t::add: {return "add";}
            case inst_t::sub: {return "sub";}
            case inst_t::mul: {return "mul";}
            case inst_t::div: {return "div";}
            case inst_t::mod: {return "mod";}
            case inst_t::addi: {return "addi";}
            case inst_t::muli: {return "muli";}
            case inst_t::divi: {return "divi";}
            case inst_t::modi: {return "modi";}
            case inst_t::b_sl: {return "b_sl";}
            case inst_t::b_sr: {return "b_sr";}
            case inst_t::b_and: {return "b_and";}
            case inst_t::b_or: {return "b_or";}
            case inst_t::b_xor: {return "b_xor";}
            case inst_t::b_not: {return "b_not";}
            case inst_t::b_sli: {return "b_sli";}
            case inst_t::b_sri: {return "b_sri";}
            case inst_t::b_andi: {return "b_andi";}
            case inst_t::b_ori: {return "b_ori";}
            case inst_t::b_xori: {return "b_xori";}
            case inst_t::b_noti: {return "b_noti";}
            case inst_t::eq: {return "eq";}
            case inst_t::ne: {return "ne";}
            case inst_t::lt: {return "lt";}
            case inst_t::le: {return "le";}
            case inst_t::store_64: {return "store_64";}
            case inst_t::load_64: {return "load_64";}
            case inst_t::store_32: {return "store_32";}
            case inst_t::load_32: {return "load_32";}
            case inst_t::load_32u: {return "load_32u";}
            case inst_t::store_16: {return "store_16";}
            case inst_t::load_16: {return "load_16";}
            case inst_t::load_16u: {return "load_16u";}
            case inst_t::store_8: {return "store_8";}
            case inst_t::load_8: {return "load_8";}
            case inst_t::load_8u: {return "load_8u";}
            case inst_t::push_expr: {return "push_expr";}
            case inst_t::pop_expr: {return "pop_expr";}
            case inst_t::syscall: {return "syscall";}
        }
        return "unknown";
    }
    inline std::ostream& operator<<(std::ostream& os, inst_t x) {
        return os << to_string(x);
    }

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
        void set_imm(int32_t imm) {(bits &= 0x00000000ffffffff) |= uint64_t(imm)<<32;}

        string to_string() const {
            using interpreter::to_string, std::to_string;
            return to_string(type())+" "+to_string(rd())+" "+to_string(rs1())+" "+to_string(rs2())+" "+to_string(imm());
        }

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
        static inst b_sli(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::b_sli, rd, rs1, reg_t::ZERO, imm);
        }
        static inst b_sri(reg_t rd, reg_t rs1, int32_t imm) {
            return inst(inst_t::b_sri, rd, rs1, reg_t::ZERO, imm);
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
        static inst push_expr(reg_t rs) {
            return inst(inst_t::push_expr, reg_t::ZERO, rs, reg_t::ZERO, 0);
        }
        static inst pop_expr(reg_t rd) {
            return inst(inst_t::pop_expr, rd, reg_t::ZERO, reg_t::ZERO, 0);
        }

        static bool is_relative_jump(inst_t type) {
            return type == inst_t::jump
                || type == inst_t::bnez
                || type == inst_t::beqz
                || type == inst_t::call;
        }
        bool is_relative_jump() const {return is_relative_jump(type());}
    };
}