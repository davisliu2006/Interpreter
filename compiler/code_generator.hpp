#pragma once

#include "ast.hpp"
#include "mem_layout.hpp"
#include "../interpreter/instructions.hpp"

namespace compiler {
    struct CodeGenerator {
        using inst = interpreter::inst;
        using reg_t = interpreter::reg_t;
        
        ast::block* top_level;
        StackBlockMap& stack_blocks;
        vector<inst> main_insts;
        vector<vector<inst>> func_insts;
        vector<StackBlock> stack_model;

        CodeGenerator(ast::block* top_level, StackBlockMap& stack_blocks):
        top_level(top_level), stack_blocks(stack_blocks) {}

        void generate() {
            generate_stmt(top_level, main_insts);
        }

        void generate_stmt(ast::stmt* stmt, vector<inst>& insts) {
            auto f_stack_block = stack_blocks.find(stmt);
            if (f_stack_block != stack_blocks.end()) {
                stack_model.push_back(f_stack_block->second);
                insts.push_back(inst::addi(reg_t::STK_PTR, reg_t::STK_PTR, stack_model.back().size()));
            }
            if (ast::var_decl* var_decl = dynamic_cast<ast::var_decl*>(stmt)) {
                if (ast::var_def* var_def = dynamic_cast<ast::var_def*>(var_decl)) {
                    generate_expr(var_def->expression, insts);
                }

            } else if (ast::f_def* f_def = dynamic_cast<ast::f_def*>(stmt)) {
                // TODO
            } else if (ast::c_if* c_if = dynamic_cast<ast::c_if*>(stmt)) {
                vector<IfAddr> branch_offsets;
                for (ast::if_branch* branch: c_if->branches) {
                    branch_offsets.push_back(IfAddr());
                    branch_offsets.back().cond_offset = insts.size();
                    generate_expr(branch->cond, insts);
                    branch_offsets.back().cond_jump_offset = insts.size();
                    insts.push_back(inst::beqz(reg_t::RES, 0)); // placeholder
                    generate_block(branch->body, insts);
                    branch_offsets.back().end_jump_offset = insts.size();
                    insts.push_back(inst::jump(0)); // placeholder
                }
                int end_offset = insts.size();
                for (int i = 0; i < branch_offsets.size(); i++) {
                    insts[branch_offsets[i].cond_jump_offset] = inst::beqz(
                        reg_t::RES, (i == branch_offsets.size()-1? 
                            end_offset : branch_offsets[i+1].cond_offset
                        ) - branch_offsets[i].cond_jump_offset
                    );
                    insts[branch_offsets[i].end_jump_offset] = inst::jump(
                        end_offset - branch_offsets[i].end_jump_offset
                    );
                }
            } else if (ast::c_for* c_for = dynamic_cast<ast::c_for*>(stmt)) {
                ForAddr for_addr;
                generate_stmt(c_for->init, insts);
                for_addr.cond_offset = insts.size();
                generate_expr(c_for->cond, insts);
                for_addr.cond_jump_offset = insts.size();
                insts.push_back(inst::beqz(reg_t::RES, 0)); // placeholder
                generate_block_reuse_stack(c_for->body, insts);
                generate_stmt(c_for->upd, insts);
                for_addr.end_jump_offset = insts.size();
                insts.push_back(inst::jump(0)); // placeholder
                int end_offset = insts.size();
                insts[for_addr.cond_jump_offset] = inst::beqz(reg_t::RES, end_offset-for_addr.cond_jump_offset);
                insts[for_addr.end_jump_offset] = inst::jump(for_addr.cond_offset-for_addr.end_jump_offset);
            } else if (ast::c_while* c_while = dynamic_cast<ast::c_while*>(stmt)) {
                WhileAddr while_addr;
                while_addr.cond_offset = insts.size();
                generate_expr(c_while->cond, insts);
                while_addr.cond_jump_offset = insts.size();
                insts.push_back(inst::beqz(reg_t::RES, 0)); // placeholder
                generate_block(c_while->body, insts);
                while_addr.end_jump_offset = insts.size();
                insts.push_back(inst::jump(while_addr.cond_offset-while_addr.end_jump_offset));
                int end_offset = insts.size();
                insts[while_addr.cond_jump_offset] = inst::beqz(reg_t::RES, end_offset-while_addr.cond_jump_offset);
            } else if (ast::expr* expr = dynamic_cast<ast::expr*>(stmt)) {
                generate_expr(expr, insts);
            } else if (ast::f_return* f_return = dynamic_cast<ast::f_return*>(stmt)) {
                generate_expr(f_return->expression, insts);
                insts.push_back(inst::ret());
            } else if (ast::block* block = dynamic_cast<ast::block*>(stmt)) {
                generate_block(block, insts);
            } else {
                throw std::runtime_error("Unknown case");
            }
            if (f_stack_block != stack_blocks.end()) {
                stack_model.pop_back();
            }
        }

        void generate_block(ast::block* block, vector<inst>& insts) {
            generate_block_reuse_stack(block, insts);
        }

        void generate_block_reuse_stack(ast::block* block, vector<inst>& insts) {
            for (ast::stmt* stmt: block->stmts) {
                generate_stmt(stmt, insts);
            }
        }

        void generate_expr(ast::expr* expr, vector<inst>& insts) {
            if (ast::var_ref* var_ref = dynamic_cast<ast::var_ref*>(expr)) {
                insts.push_back(inst::load_64(reg_t::RES, stack_model.back().find_var(var_ref->resolve)->offset, reg_t::STK_PTR));
            } else if (ast::f_call* f_call = dynamic_cast<ast::f_call*>(expr)) {
                // TODO
            } else if (ast::literal* literal = dynamic_cast<ast::literal*>(expr)) {
                insts.push_back(inst::load_imm(reg_t::RES, std::stoi(literal->value)));
            } else if (ast::asn* asn = dynamic_cast<ast::asn*>(expr)) {
                insts.push_back(inst::store_64(reg_t::RES, stack_model.back().find_var(var_ref->resolve)->offset, reg_t::STK_PTR));
            } else if (ast::op_asn* op_asn = dynamic_cast<ast::op_asn*>(expr)) {
                // TODO
            } else if (ast::op_bin* op_bin = dynamic_cast<ast::op_bin*>(expr)) {
                generate_expr(op_bin->lhs, insts);
                insts.push_back(inst::push_expr(reg_t::RES));
                generate_expr(op_bin->rhs, insts);
                insts.push_back(inst::pop_expr(reg_t::T1));
                if (op_bin->op == "+") {
                    insts.push_back(inst::add(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "-") {
                    insts.push_back(inst::sub(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "*") {
                    insts.push_back(inst::mul(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "/") {
                    insts.push_back(inst::div(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "%") {
                    insts.push_back(inst::mod(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "<<") {
                    insts.push_back(inst::b_sl(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == ">>") {
                    insts.push_back(inst::b_sr(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "&") {
                    insts.push_back(inst::b_and(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "^") {
                    insts.push_back(inst::b_xor(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "|") {
                    insts.push_back(inst::b_or(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "&&") {
                    insts.push_back(inst::b_and(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "||") {
                    insts.push_back(inst::b_or(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "==") {
                    insts.push_back(inst::eq(reg_t::RES, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "!=") {
                    insts.push_back(inst::ne(reg_t::RES, reg_t::T1, reg_t::RES));
                }
            } else {
                throw std::runtime_error("Unknown case");
            }
        }
    };
}