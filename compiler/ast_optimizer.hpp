#pragma once

#include "ast.hpp"

namespace compiler {
    struct AstOptimizer {
        private:
        ast::block*& top_level;

        public:
        AstOptimizer(ast::block*& top_level): top_level(top_level) {}

        void optimize() {
            optimize_block(top_level);
        }

        private:
        void optimize_block(ast::block*& block) {
            for (ast::stmt*& stmt: block->stmts) {
                optimize_stmt(stmt);
            }
        }

        void optimize_stmt(ast::stmt*& stmt) {
            if (ast::block* block = dynamic_cast<ast::block*>(stmt)) {
                optimize_block(block);
            } else if (ast::expr* expr = dynamic_cast<ast::expr*>(stmt)) {
                optimize_expr(expr);
            } else if (ast::f_return* f_return = dynamic_cast<ast::f_return*>(stmt)) {
                optimize_expr(f_return->expression);
            } else if (ast::c_if* c_if = dynamic_cast<ast::c_if*>(stmt)) {
                for (ast::if_branch* branch: c_if->branches) {
                    optimize_expr(branch->cond);
                    optimize_block(branch->body);
                }
            } else if (ast::c_for* c_for = dynamic_cast<ast::c_for*>(stmt)) {
                optimize_stmt(c_for->init);
                optimize_expr(c_for->cond);
                optimize_stmt(c_for->upd);
                optimize_block(c_for->body);
            } else if (ast::c_while* c_while = dynamic_cast<ast::c_while*>(stmt)) {
                optimize_expr(c_while->cond);
                optimize_block(c_while->body);
            } else if (ast::var_def* var_def = dynamic_cast<ast::var_def*>(stmt)) {
                optimize_expr(var_def->expression);
            } else if (ast::f_def* f_def = dynamic_cast<ast::f_def*>(stmt)) {
                optimize_block(f_def->body);
            } else {
                throw std::runtime_error("Unknown case");
            }
        }

        void optimize_expr(ast::expr*& expr) {
            if (ast::asn* asn = dynamic_cast<ast::asn*>(expr)) {
                optimize_expr(asn->expression);
            } else if (ast::op_asn* op_asn = dynamic_cast<ast::op_asn*>(expr)) {
                optimize_expr(op_asn->expression);
            } else if (ast::op_bin* op_bin = dynamic_cast<ast::op_bin*>(expr)) {
                optimize_expr(op_bin->lhs);
                optimize_expr(op_bin->rhs);
                ast::literal* lhs_lit = dynamic_cast<ast::literal*>(op_bin->lhs);
                ast::literal* rhs_lit = dynamic_cast<ast::literal*>(op_bin->rhs);
                if (lhs_lit && rhs_lit) {
                    if (lhs_lit->type == sym_t::INT && rhs_lit->type == sym_t::INT) {
                        int lhs_val = std::stoi(lhs_lit->value);
                        int rhs_val = std::stoi(rhs_lit->value);
                        if (op_bin->op == "+") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val+rhs_val));
                            return;
                        } else if (op_bin->op == "-") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val-rhs_val));
                            return;
                        } else if (op_bin->op == "*") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val*rhs_val));
                            return;
                        } else if (op_bin->op == "/") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val/rhs_val));
                            return;
                        } else if (op_bin->op == "%") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val%rhs_val));
                            return;
                        } else if (op_bin->op == "&") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val&rhs_val));
                            return;
                        } else if (op_bin->op == "|") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val|rhs_val));
                            return;
                        } else if (op_bin->op == "^") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val^rhs_val));
                            return;
                        } else if (op_bin->op == "<<") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val<<rhs_val));
                            return;
                        } else if (op_bin->op == ">>") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(lhs_val>>rhs_val));
                            return;
                        }
                    }
                }
            } else if (ast::op_un* op_un = dynamic_cast<ast::op_un*>(expr)) {
                optimize_expr(op_un->expression);
                ast::literal* lit = dynamic_cast<ast::literal*>(op_un->expression);
                if (lit) {
                    if (lit->type == sym_t::INT) {
                        int val = std::stoi(lit->value);
                        if (op_un->op == "-") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(-val));
                            return;
                        } else if (op_un->op == "~") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(~val));
                            return;
                        } else if (op_un->op == "!") {
                            delete expr;
                            expr = new ast::literal(sym_t::INT, std::to_string(!val));
                            return;
                        }
                    }
                }
            }
        }
    };
}