#pragma once

#include "ast.hpp"
#include "../ds/trie_map.hpp"

namespace compiler {
    struct Scope {

    };

    struct Resolver {
        ast::block* top_level;
        ds::trie_map<ast::var_decl*> var_map;
        ds::trie_map<ast::f_def*> func_map;

        Resolver(ast::block* top_level): top_level(top_level) {

        }

        void resolve_block(ast::block* block) {
            for (ast::stmt* stmt: block->stmts) {
                resolve_stmt(stmt);
            }
        }

        void resolve_stmt(ast::stmt* stmt) {
            ast::var_decl* var_decl;
            ast::f_def* f_def;
            ast::c_if* c_if;
            ast::c_for* c_for;
            ast::c_while* c_while;
            ast::expr* expr;
            if ((var_decl = dynamic_cast<ast::var_decl*>(stmt))) {
                var_map.insert(var_decl->var.c_str())->val = var_decl;
            } else if ((f_def = dynamic_cast<ast::f_def*>(stmt))) {
                func_map.insert(f_def->name.c_str())->val = f_def;
            } else if ((c_if = dynamic_cast<ast::c_if*>(stmt))) {
                for (ast::if_branch* branch: c_if->branches) {
                    resolve_expr(branch->cond);
                    resolve_block(branch->body);
                }
            } else if ((c_for = dynamic_cast<ast::c_for*>(stmt))) {
                resolve_stmt(c_for->init);
                resolve_expr(c_for->cond);
                resolve_stmt(c_for->upd);
                resolve_block(c_for->body);
            } else if ((c_while = dynamic_cast<ast::c_while*>(stmt))) {
                resolve_expr(c_while->cond);
                resolve_block(c_while->body);
            } else if ((expr = dynamic_cast<ast::expr*>(stmt))) {
                resolve_expr(expr);
            } else {
                throw std::runtime_error("Unknown case");
            }
        }

        void resolve_expr(ast::expr* expr) {
            ast::var_ref* var_ref;
            ast::f_call* f_call;
            ast::literal* literal;
            ast::asn* asn;
            ast::op_bin* op_bin;
            ast::op_asn* op_asn;
            if ((var_ref = dynamic_cast<ast::var_ref*>(expr))) {
                var_ref->resolve = var_map.find(var_ref->name.c_str())->val;
            } else if ((f_call = dynamic_cast<ast::f_call*>(expr))) {
                f_call->resolve = func_map.find(f_call->func.c_str())->val;
            } else if ((literal = dynamic_cast<ast::literal*>(expr))) {
                // do nothing
            } else if ((asn = dynamic_cast<ast::asn*>(expr))) {
                resolve_expr(asn->expression);
            } else if ((op_bin = dynamic_cast<ast::op_bin*>(expr))) {
                resolve_expr(op_bin->lhs);
                resolve_expr(op_bin->rhs);
            } else if ((op_asn = dynamic_cast<ast::op_asn*>(expr))) {
                resolve_expr(op_asn->expression);
            } else {
                throw std::runtime_error("Unknown case");
            }
        }
    };
}