#pragma once

#include "ast.hpp"
#include "mem_layout.hpp"
#include "../interpreter/instructions.hpp"

namespace compiler {
    struct CodeGenerator {
        using inst = interpreter::inst;
        using inst_t = interpreter::inst_t;
        using reg_t = interpreter::reg_t;
        
        private:
        ast::block* top_level;
        StackBlockMap& stack_blocks;
        FIndexMap& f_index;
        StackModel stack_model;
        std::stack<FBodyAddr> f_body_addrs;

        public:
        vector<inst> main_insts;
        vector<vector<inst>> func_insts;
        
        CodeGenerator(ast::block* top_level, StackBlockMap& stack_blocks, unordered_map<ast::f_def*,int>& f_index):
        top_level(top_level), stack_blocks(stack_blocks), f_index(f_index) {
            func_insts.resize(f_index.size());
        }

        vector<inst> generate() {
            generate_block(top_level, main_insts);
            main_insts.push_back(inst::exit());
            // transform calls
            vector<int> func_global_offsets(func_insts.size());
            int tot = main_insts.size();
            for (int i = 0; i < func_insts.size(); i++) {
                func_global_offsets[i] = tot;
                tot += func_insts[i].size();
            }
            for (int i = 0; i < main_insts.size(); i++) { // transform calls in main
                if (main_insts[i].type() == inst_t::call) {
                    int call_fi = main_insts[i].imm();
                    assert(0 <= call_fi && call_fi < func_global_offsets.size());
                    main_insts[i] = inst::call(func_global_offsets[call_fi]-i);
                }
            }
            for (int fi = 0; fi < func_insts.size(); fi++) { // transform calls in functions
                auto& insts = func_insts[fi];
                int f_base = func_global_offsets[fi];
                for (int i = 0; i < insts.size(); i++) {
                    if (insts[i].type() == inst_t::call) {
                        int call_fi = insts[i].imm();
                        assert(0 <= call_fi && call_fi < func_global_offsets.size());
                        insts[i] = inst::call(func_global_offsets[call_fi]-(f_base+i));
                    }
                }
            }
            // combined insts
            vector<inst> combined_insts;
            for (const inst& inst: main_insts) {
                combined_insts.push_back(inst);
            }
            for (const vector<inst>& insts: func_insts) {
                for (const inst& inst: insts) {
                    combined_insts.push_back(inst);
                }
            }
            return combined_insts;
        }

        private:
        void push_stack(ast::stmt* scope_stmt, vector<inst>& insts) {
            auto f = stack_blocks.find(scope_stmt);
            if (f == stack_blocks.end()) {
                throw std::runtime_error("Missing stack block for scope");
            }
            stack_model.push_block(f->second);
            if (stack_model.top().size() > 0) {
                insts.push_back(inst::addi(reg_t::STK_PTR, reg_t::STK_PTR, -stack_model.top().size()));
            }
        }
        void pop_stack(vector<inst>& insts) {
            if (stack_model.top().size() > 0) {
                insts.push_back(inst::addi(reg_t::STK_PTR, reg_t::STK_PTR, stack_model.top().size()));
            }
            stack_model.pop_block();
        }

        void generate_stmt(ast::stmt* stmt, vector<inst>& insts) {
            if (ast::var_decl* var_decl = dynamic_cast<ast::var_decl*>(stmt)) {
                if (ast::var_def* var_def = dynamic_cast<ast::var_def*>(var_decl)) {
                    generate_expr(var_def->expression, insts);
                    insts.push_back(inst::store_64(reg_t::RES, stack_model.get_tot_offset(var_decl), reg_t::STK_PTR));
                }
            } else if (ast::f_def* f_def = dynamic_cast<ast::f_def*>(stmt)) {
                int index = f_index[f_def];
                auto& f_insts = func_insts[index];
                push_stack(f_def, f_insts);
                f_insts.push_back(inst::store_64(reg_t::RA, stack_model.top().RA(), reg_t::STK_PTR));
                for (int i = int(f_def->params.size())-1; i >= 0; i--) {
                    ast::var_decl* param = f_def->params[i];
                    f_insts.push_back(inst::pop_expr(reg_t::RES));
                    f_insts.push_back(inst::store_64(
                        reg_t::RES, stack_model.get_tot_offset(param), reg_t::STK_PTR
                    ));
                }
                f_body_addrs.push(FBodyAddr());
                FBodyAddr& f_body_addr = f_body_addrs.top();
                generate_block_reuse_stack(f_def->body, f_insts);
                if (!f_body_addr.early_ret_offsets.empty()
                && f_body_addr.early_ret_offsets.back() == int(f_insts.size())-1) {
                    f_body_addr.early_ret_offsets.pop_back();
                    f_insts.pop_back();
                }
                for (int i = 0; i < f_body_addr.early_ret_offsets.size(); i++) {
                    int jump_offset = f_body_addr.early_ret_offsets[i];
                    assert(f_insts[jump_offset].type() == inst_t::jump);
                    f_insts[jump_offset] = inst::jump(f_insts.size()-jump_offset);
                }
                f_body_addrs.pop();
                f_insts.push_back(inst::load_64(reg_t::RA, stack_model.top().RA(), reg_t::STK_PTR));
                pop_stack(f_insts);
                f_insts.push_back(inst::ret());
            } else if (ast::c_if* c_if = dynamic_cast<ast::c_if*>(stmt)) {
                vector<IfAddr> branch_offsets;
                for (ast::if_branch* branch: c_if->branches) {
                    branch_offsets.push_back(IfAddr());
                    branch_offsets.back().cond_offset = insts.size();
                    generate_expr(branch->cond, insts);
                    branch_offsets.back().cond_jump_offset = insts.size();
                    insts.push_back(inst::beqz(reg_t::RES, 0)); // placeholder
                    generate_block(branch->body, insts);
                    if (branch != c_if->branches.back()) {
                        branch_offsets.back().end_jump_offset = insts.size();
                        insts.push_back(inst::jump(0)); // placeholder
                    }
                }
                int end_offset = insts.size();
                for (int i = 0; i < int(branch_offsets.size())-1; i++) {
                    insts[branch_offsets[i].cond_jump_offset] = inst::beqz(
                        reg_t::RES,
                        branch_offsets[i+1].cond_offset - branch_offsets[i].cond_jump_offset
                    );
                    insts[branch_offsets[i].end_jump_offset] = inst::jump(
                        end_offset - branch_offsets[i].end_jump_offset
                    );
                }
                insts[branch_offsets.back().cond_jump_offset] = inst::beqz(
                    reg_t::RES,
                    end_offset - branch_offsets.back().cond_jump_offset
                );
            } else if (ast::c_for* c_for = dynamic_cast<ast::c_for*>(stmt)) {
                push_stack(c_for, insts);
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
                pop_stack(insts);
            } else if (ast::c_while* c_while = dynamic_cast<ast::c_while*>(stmt)) {
                push_stack(c_while, insts);
                WhileAddr while_addr;
                while_addr.cond_offset = insts.size();
                generate_expr(c_while->cond, insts);
                while_addr.cond_jump_offset = insts.size();
                insts.push_back(inst::beqz(reg_t::RES, 0)); // placeholder
                generate_block_reuse_stack(c_while->body, insts);
                while_addr.end_jump_offset = insts.size();
                insts.push_back(inst::jump(while_addr.cond_offset-while_addr.end_jump_offset));
                int end_offset = insts.size();
                insts[while_addr.cond_jump_offset] = inst::beqz(reg_t::RES, end_offset-while_addr.cond_jump_offset);
                pop_stack(insts);
            } else if (ast::expr* expr = dynamic_cast<ast::expr*>(stmt)) {
                generate_expr(expr, insts);
            } else if (ast::f_return* f_return = dynamic_cast<ast::f_return*>(stmt)) {
                generate_expr(f_return->expression, insts);
                assert(!f_body_addrs.empty() && "return statement used outside function");
                f_body_addrs.top().early_ret_offsets.push_back(insts.size());
                insts.push_back(inst::jump(0)); // placeholder
            } else if (ast::block* block = dynamic_cast<ast::block*>(stmt)) {
                generate_block(block, insts);
            } else {
                throw std::runtime_error("Unknown case");
            }
        }

        void generate_block(ast::block* block, vector<inst>& insts) {
            push_stack(block, insts);
            generate_block_reuse_stack(block, insts);
            pop_stack(insts);
        }

        void generate_block_reuse_stack(ast::block* block, vector<inst>& insts) {
            for (ast::stmt* stmt: block->stmts) {
                generate_stmt(stmt, insts);
            }
        }

        void generate_expr(ast::expr* expr, vector<inst>& insts, reg_t res_reg = reg_t::RES) {
            if (ast::var_ref* var_ref = dynamic_cast<ast::var_ref*>(expr)) {
                if (!var_ref->resolve) {
                    throw std::runtime_error("Unresolved variable reference: "+var_ref->name);
                }
                insts.push_back(inst::load_64(res_reg, stack_model.get_tot_offset(var_ref->resolve), reg_t::STK_PTR));
            } else if (ast::f_call* f_call = dynamic_cast<ast::f_call*>(expr)) {
                if (!f_call->resolve) {
                    throw std::runtime_error("Unresolved function call: "+f_call->func);
                }
                ast::f_def* f_def = f_call->resolve;
                for (int i = 0; i < f_call->args.size(); i++) {
                    ast::expr* arg = f_call->args[i];
                    generate_expr(arg, insts);
                    insts.push_back(inst::push_expr(reg_t::RES));
                }
                insts.push_back(inst::call(f_index[f_def])); // placeholder
            } else if (ast::literal* literal = dynamic_cast<ast::literal*>(expr)) {
                insts.push_back(inst::load_imm(res_reg, std::stoi(literal->value)));
            } else if (ast::asn* asn = dynamic_cast<ast::asn*>(expr)) {
                if (!asn->var->resolve) {
                    throw std::runtime_error("Unresolved assignment target: "+asn->var->name);
                }
                generate_expr(asn->expression, insts, res_reg);
                insts.push_back(inst::store_64(res_reg, stack_model.get_tot_offset(asn->var->resolve), reg_t::STK_PTR));
            } else if (ast::op_asn* op_asn = dynamic_cast<ast::op_asn*>(expr)) {
                if (!op_asn->var->resolve) {
                    throw std::runtime_error("Unresolved assignment target: "+op_asn->var->name);
                }
                int32_t offset = stack_model.get_tot_offset(op_asn->var->resolve);
                if (ast::literal* rhs_lit = dynamic_cast<ast::literal*>(op_asn->expression)) {
                    if (rhs_lit->type == sym_t::INT && supports_imm(op_asn->op)) {
                        insts.push_back(inst::load_64(reg_t::RES, offset, reg_t::STK_PTR));
                        insts.push_back(binary_op_imm(op_asn->op, res_reg, reg_t::RES, std::stoi(rhs_lit->value)));
                        insts.push_back(inst::store_64(res_reg, offset, reg_t::STK_PTR));
                        return;
                    }
                }
                generate_expr(op_asn->expression, insts);
                insts.push_back(inst::load_64(reg_t::T1, offset, reg_t::STK_PTR));
                insts.push_back(binary_op(op_asn->op, res_reg, reg_t::T1, reg_t::RES));
                insts.push_back(inst::store_64(res_reg, offset, reg_t::STK_PTR));
            } else if (ast::op_bin* op_bin = dynamic_cast<ast::op_bin*>(expr)) {
                if (ast::literal* rhs_lit = dynamic_cast<ast::literal*>(op_bin->rhs)) {
                    if (rhs_lit->type == sym_t::INT && supports_imm(op_bin->op)) {
                        generate_expr(op_bin->lhs, insts);
                        insts.push_back(binary_op_imm(op_bin->op, res_reg, reg_t::RES, std::stoi(rhs_lit->value)));
                        return;
                    }
                }
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
                insts.push_back(binary_op(op_bin->op, res_reg, reg_t::T1, reg_t::RES));
            } else if (ast::op_un* op_un = dynamic_cast<ast::op_un*>(expr)) {
                generate_expr(op_un->expression, insts);
                insts.push_back(unary_op(op_un->op, res_reg, reg_t::RES));
            } else {
                throw std::runtime_error("Unknown case");
            }
        }

        inst binary_op(const string& op, reg_t rd, reg_t rs1, reg_t rs2) const {
            if (op == "+") {return inst::add(rd, rs1, rs2);}
            else if (op == "-") {return inst::sub(rd, rs1, rs2);}
            else if (op == "*") {return inst::mul(rd, rs1, rs2);}
            else if (op == "/") {return inst::div(rd, rs1, rs2);}
            else if (op == "%") {return inst::mod(rd, rs1, rs2);}
            else if (op == "<<") {return inst::b_sl(rd, rs1, rs2);}
            else if (op == ">>") {return inst::b_sr(rd, rs1, rs2);}
            else if (op == "&") {return inst::b_and(rd, rs1, rs2);}
            else if (op == "^") {return inst::b_xor(rd, rs1, rs2);}
            else if (op == "|") {return inst::b_or(rd, rs1, rs2);}
            else if (op == "&&") {return inst::b_and(rd, rs1, rs2);}
            else if (op == "||") {return inst::b_or(rd, rs1, rs2);}
            else if (op == "==") {return inst::eq(rd, rs1, rs2);}
            else if (op == "!=") {return inst::ne(rd, rs1, rs2);}
            else if (op == "<") {return inst::lt(rd, rs1, rs2);}
            else if (op == "<=") {return inst::le(rd, rs1, rs2);}
            else if (op == ">") {return inst::lt(rd, rs2, rs1);}
            else if (op == ">=") {return inst::le(rd, rs2, rs1);}
            else {throw std::runtime_error("Unsupported binary operator: "+op);}
        }
        bool supports_imm(const string& op) const {
            return op == "+" || op == "-" || op == "*" || op == "/" || op == "%"
                || op == "<<" || op == ">>" || op == "&" || op == "|" || op == "^";
        }
        inst binary_op_imm(const string& op, reg_t rd, reg_t rs1, int32_t imm) const {
            if (op == "+") {return inst::addi(rd, rs1, imm);}
            else if (op == "-") {return inst::addi(rd, rs1, -imm);}
            else if (op == "*") {return inst::muli(rd, rs1, imm);}
            else if (op == "/") {return inst::divi(rd, rs1, imm);}
            else if (op == "%") {return inst::modi(rd, rs1, imm);}
            else if (op == "<<") {return inst::b_sli(rd, rs1, imm);}
            else if (op == ">>") {return inst::b_sri(rd, rs1, imm);}
            else if (op == "&") {return inst::b_andi(rd, rs1, imm);}
            else if (op == "|") {return inst::b_ori(rd, rs1, imm);}
            else if (op == "^") {return inst::b_xori(rd, rs1, imm);}
            else {throw std::runtime_error("Unsupported binary immediate operator: "+op);}
        }

        inst unary_op(const string& op, reg_t rd, reg_t rs1) const {
            if (op == "-") {return inst::sub(rd, reg_t::ZERO, rs1);}
            else if (op == "~") {return inst::b_not(rd, rs1);}
            else if (op == "!") {return inst::eq(rd, rs1, reg_t::ZERO);}
            else {throw std::runtime_error("Unsupported unary operator: "+op);}
        }
    };
}