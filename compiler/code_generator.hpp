#pragma once

#include "ast.hpp"
#include "mem_layout.hpp"
#include "../interpreter/instructions.hpp"

namespace compiler {
    struct CodeGenerator {
        using inst = interpreter::inst;
        using reg_t = interpreter::reg_t;
        
        private:
        ast::block* top_level;
        StackBlockMap& stack_blocks;
        unordered_map<ast::f_def*,int> f_index;
        StackModel stack_model;

        public:
        vector<inst> main_insts;
        vector<vector<inst>> func_insts;
        
        CodeGenerator(ast::block* top_level, StackBlockMap& stack_blocks, unordered_map<ast::f_def*,int>& f_index):
        top_level(top_level), stack_blocks(stack_blocks), f_index(f_index) {
            func_insts.resize(f_index.size());
        }

        void generate() {
            generate_stmt(top_level, main_insts);
        }

        void generate_stmt(ast::stmt* stmt, vector<inst>& insts) {
            auto f_stack_block = stack_blocks.find(stmt);
            if (f_stack_block != stack_blocks.end()) {
                stack_model.push_block(f_stack_block->second);
                insts.push_back(inst::addi(reg_t::STK_PTR, reg_t::STK_PTR, -stack_model.top().size()));
            }
            if (ast::var_decl* var_decl = dynamic_cast<ast::var_decl*>(stmt)) {
                if (ast::var_def* var_def = dynamic_cast<ast::var_def*>(var_decl)) {
                    generate_expr(var_def->expression, insts);
                    insts.push_back(inst::store_64(reg_t::RES, stack_model.get_tot_offset(var_decl), reg_t::STK_PTR));
                }
            } else if (ast::f_def* f_def = dynamic_cast<ast::f_def*>(stmt)) {
                int index = f_index[f_def];
                auto& f_insts = func_insts[index];
                stack_model.push_block(stack_blocks[f_def]);
                f_insts.push_back(inst::addi(reg_t::STK_PTR, reg_t::STK_PTR, -stack_model.top().size()));
                f_insts.push_back(inst::store_64(reg_t::RA, stack_model.top().RA(), reg_t::STK_PTR));
                generate_block_reuse_stack(f_def->body, f_insts);
                f_insts.push_back(inst::load_64(reg_t::RA, stack_model.top().RA(), reg_t::STK_PTR));
                f_insts.push_back(inst::addi(reg_t::STK_PTR, reg_t::STK_PTR, stack_model.top().size()));
                f_insts.push_back(inst::ret());
                stack_model.pop_block();
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
                insts.push_back(inst::addi(reg_t::STK_PTR, reg_t::STK_PTR, stack_model.top().size()));
                stack_model.pop_block();
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

        void generate_expr(ast::expr* expr, vector<inst>& insts, reg_t res_reg = reg_t::RES) {
            if (ast::var_ref* var_ref = dynamic_cast<ast::var_ref*>(expr)) {
                insts.push_back(inst::load_64(res_reg, stack_model.get_tot_offset(var_ref->resolve), reg_t::STK_PTR));
            } else if (ast::f_call* f_call = dynamic_cast<ast::f_call*>(expr)) {
                ast::f_def* f_def = f_call->resolve;
                StackBlock& stack_block = stack_blocks[f_def];
                for (int i = 0; i < f_def->params.size(); i++) {
                    ast::var_decl* param = f_def->params[i];
                    ast::expr* arg = f_call->args[i];
                    generate_expr(arg, insts);
                    insts.push_back(inst::store_64(reg_t::RES, stack_block.find_var(param)->offset-stack_block.size(), reg_t::STK_PTR));
                }
                insts.push_back(inst::call(f_index[f_def])); // placeholder
            } else if (ast::literal* literal = dynamic_cast<ast::literal*>(expr)) {
                insts.push_back(inst::load_imm(res_reg, std::stoi(literal->value)));
            } else if (ast::asn* asn = dynamic_cast<ast::asn*>(expr)) {
                generate_expr(asn->expression, insts, res_reg);
                insts.push_back(inst::store_64(res_reg, stack_model.get_tot_offset(asn->var->resolve), reg_t::STK_PTR));
            } else if (ast::op_asn* op_asn = dynamic_cast<ast::op_asn*>(expr)) {
                int32_t offset = stack_model.get_tot_offset(op_asn->var->resolve);
                generate_expr(op_asn->expression, insts);
                insts.push_back(inst::load_64(reg_t::T1, offset, reg_t::STK_PTR));
                if (op_asn->op == "+") {
                    insts.push_back(inst::add(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == "-") {
                    insts.push_back(inst::sub(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == "*") {
                    insts.push_back(inst::mul(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == "/") {
                    insts.push_back(inst::div(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == "%") {
                    insts.push_back(inst::mod(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == "<<") {
                    insts.push_back(inst::b_sl(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == ">>") {
                    insts.push_back(inst::b_sr(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == "&") {
                    insts.push_back(inst::b_and(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == "^") {
                    insts.push_back(inst::b_xor(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_asn->op == "|") {
                    insts.push_back(inst::b_or(res_reg, reg_t::T1, reg_t::RES));
                }
                insts.push_back(inst::store_64(res_reg, offset, reg_t::STK_PTR));
            } else if (ast::op_bin* op_bin = dynamic_cast<ast::op_bin*>(expr)) {
                if (op_bin->rhs->is_trivial()) {
                    generate_expr(op_bin->lhs, insts, reg_t::T1);
                    generate_expr(op_bin->rhs, insts);
                } else if (op_bin->lhs->is_trivial()) {
                    generate_expr(op_bin->rhs, insts);
                    generate_expr(op_bin->lhs, insts, reg_t::T1);
                } else {
                    generate_expr(op_bin->lhs, insts);
                    insts.push_back(inst::push_expr(reg_t::RES));
                    generate_expr(op_bin->rhs, insts);
                    insts.push_back(inst::pop_expr(reg_t::T1));
                }
                if (op_bin->op == "+") {
                    insts.push_back(inst::add(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "-") {
                    insts.push_back(inst::sub(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "*") {
                    insts.push_back(inst::mul(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "/") {
                    insts.push_back(inst::div(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "%") {
                    insts.push_back(inst::mod(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "<<") {
                    insts.push_back(inst::b_sl(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == ">>") {
                    insts.push_back(inst::b_sr(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "&") {
                    insts.push_back(inst::b_and(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "^") {
                    insts.push_back(inst::b_xor(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "|") {
                    insts.push_back(inst::b_or(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "&&") {
                    insts.push_back(inst::b_and(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "||") {
                    insts.push_back(inst::b_or(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "==") {
                    insts.push_back(inst::eq(res_reg, reg_t::T1, reg_t::RES));
                } else if (op_bin->op == "!=") {
                    insts.push_back(inst::ne(res_reg, reg_t::T1, reg_t::RES));
                }
            } else if (ast::op_un* op_un = dynamic_cast<ast::op_un*>(expr)) {
                generate_expr(op_un->expression, insts);
                if (op_un->op == "-") {
                    insts.push_back(inst::sub(res_reg, reg_t::ZERO, reg_t::RES));
                } else if (op_un->op == "~") {
                    insts.push_back(inst::b_not(res_reg, reg_t::RES));
                } else if (op_un->op == "!") {
                    insts.push_back(inst::eq(res_reg, reg_t::RES, reg_t::ZERO));
                } else {
                    throw std::runtime_error("Unsupported unary operator: "+op_un->op);
                }
            } else {
                throw std::runtime_error("Unknown case");
            }
        }
    };
}