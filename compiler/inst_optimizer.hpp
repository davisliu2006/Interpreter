#pragma once

#include "../interpreter/instructions.hpp"

namespace compiler {
    /*
    Things we want to optimize:
    - Remove no-ops such as add_i rd, rs1, 0
    - Ensure we are not producing jump 1 instructions
    - If a source is already in the destination register, we should not reload it from the same source
    */
    struct InstOptimizer {
        using inst = interpreter::inst;
        using inst_t = interpreter::inst_t;
        using reg_t = interpreter::reg_t;
        
        private:
        vector<inst>& original;
        vector<inst> optimized;
        vector<int> inst_remapping;

        public:
        InstOptimizer(vector<inst>& insts): original(insts) {}

        vector<inst> optimize() {
            inst_remapping = vector<int>(original.size(), -1);
            remove_redundant();
            remap_insts();
            return optimized;
        }

        private:
        void remap_insts() {
            for (int old_i = 0; old_i < original.size(); old_i++) {
                if (inst_remapping[old_i] != -1) {
                    int new_i = inst_remapping[old_i];
                    if (original[old_i].is_relative_jump()) {
                        int old_trg = old_i+original[old_i].imm();
                        int new_trg = inst_remapping[old_trg];
                        assert(new_trg != -1);
                        optimized[new_i].set_imm(new_trg-new_i);
                    }
                }
            }
        }

        void remove_redundant() {
            for (int i = 0; i < original.size(); i++) {
                inst& curr = original[i];
                if (curr.type() == inst_t::jump) {
                    assert(curr.imm() != 1);
                }
                if (
                    curr.rd() == curr.rs1() && (
                        curr.type() == inst_t::addi && curr.imm() == 0
                        || curr.type() == inst_t::muli && curr.imm() == 1
                        || curr.type() == inst_t::divi && curr.imm() == 1
                        || curr.type() == inst_t::b_sli && curr.imm() == 0
                        || curr.type() == inst_t::b_sri && curr.imm() == 0
                        || curr.type() == inst_t::mv
                    )
                ) {
                    continue;
                }
                if (
                    !optimized.empty()
                ) {
                    inst& prev = optimized.back();
                    if (
                        curr.type() == inst_t::load_64 && prev.type() == inst_t::store_64
                        && curr.rd() == prev.rs2() && curr.rs1() == prev.rs1() && curr.imm() == prev.imm()
                    ) {
                        continue;
                    }
                }
                inst_remapping[i] = optimized.size();
                optimized.push_back(curr);
            }
        }
    };
}