#pragma once

#include "base.hpp"

namespace interpreter {
    template <class type>
    static constexpr bool is_valid_regtype() {
        return (std::is_arithmetic<type>() || std::is_pointer<type>())
        && sizeof(type) <= 8
        && std::is_trivially_copyable<type>();
    }
    
    enum class reg_t: uint8_t {
        ZERO = 0,
        STK_PTR = 1,
        HEAP_PTR = 2,
        EXPR_PTR = 3,
        RA = 4,
        RES = 5,
        RES_TYPE = 6,
        T1 = 16,
        T2 = 17,
        T3 = 18,
        T4 = 19,
    };

    inline string to_string(reg_t x) {
        switch (x) {
            case reg_t::ZERO: {return "ZERO";}
            case reg_t::STK_PTR: {return "STK_PTR";}
            case reg_t::HEAP_PTR: {return "HEAP_PTR";}
            case reg_t::EXPR_PTR: {return "EXPR_PTR";}
            case reg_t::RA: {return "RA";}
            case reg_t::RES: {return "RES";}
            case reg_t::RES_TYPE: {return "RES_TYPE";}
            case reg_t::T1: {return "T1";}
            case reg_t::T2: {return "T2";}
            case reg_t::T3: {return "T3";}
            case reg_t::T4: {return "T4";}
        }
        return "UNKNOWN";
    }
    inline std::ostream& operator<<(std::ostream& os, reg_t x) {
        return os << to_string(x);
    }

    struct Registers {
        static const int SIZE = 32;
        
        inst* inst_ptr;
        array<int64_t,SIZE> reg;

        Registers() {
            reg.fill(0);
        }

        int64_t& operator[](uint8_t indx) {
            return reg[indx];
        }
        int64_t& operator[](reg_t indx) {
            return reg[uint8_t(indx)];
        }

        int64_t& reg_as_int64(size_t indx) {return *(int64_t*)(&reg[indx]);}
        uint64_t& reg_as_uint64(size_t indx) {return *(uint64_t*)(&reg[indx]);}
        double& reg_as_double(size_t indx) {return *(double*)(&reg[indx]);}
        template <class type>
        type*& reg_as_ptr(size_t indx) {return *(type**)(&reg[indx]);}

        int8_t*& stk_ptr() {return *(int8_t**)(&reg[uint8_t(reg_t::STK_PTR)]);}
        int8_t*& heap_ptr() {return *(int8_t**)(&reg[uint8_t(reg_t::HEAP_PTR)]);}
        int8_t*& expr_ptr() {return *(int8_t**)(&reg[uint8_t(reg_t::STK_PTR)]);}
    };
}