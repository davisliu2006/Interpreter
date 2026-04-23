#pragma once

#include <deque>
#include "ast.hpp"
#include "mem_layout.hpp"
#include "../ds/trie_map.hpp"

namespace compiler {
    struct Scope {
        ds::trie_map<ast::var_decl*> var_map;
        ds::trie_map<ast::f_def*> func_map;
    };

    struct Resolver {
        private:
        ast::block* top_level;
        std::deque<Scope> scopes;
        ast::stmt* scope_stmt = NULL;
        
        public:
        ds::trie_map<ast::var_decl*> var_exports;
        set<ast::var_ref*> var_imports;
        ds::trie_map<ast::f_def*> f_exports;
        set<ast::f_call*> f_imports;
        StackBlockMap stack_blocks;

        Resolver(ast::block* top_level): top_level(top_level) {}

        void resolve() {
            resolve_block(top_level);
            // exports
            for (ast::stmt* stmt: top_level->stmts) {
                if (ast::var_decl* var_decl = dynamic_cast<ast::var_decl*>(stmt)) {
                    var_exports.insert(var_decl->var.c_str())->val = var_decl;
                } else if (ast::f_def* f_def = dynamic_cast<ast::f_def*>(stmt)) {
                    f_exports.insert(f_def->name.c_str())->val = f_def;
                }
            }
        }

        private:
        ast::var_decl* find_var_decl(const char* str) const {
            auto it = scopes.rbegin();
            while (it != scopes.rend()) {
                auto f = it->var_map.find(str);
                if (f) {return f->val;}
                it++;
            }
            return NULL;
        }
        ast::f_def* find_f_def(const char* str) const {
            auto it = scopes.rbegin();
            while (it != scopes.rend()) {
                auto f = it->func_map.find(str);
                if (f) {return f->val;}
                it++;
            }
            return NULL;
        }

        void resolve_block(ast::block* block) {
            scopes.push_back(Scope());
            stack_blocks[scope_stmt = block] = StackBlock();
            resolve_block_append_scope(block);
            scopes.pop_back();
        }

        void resolve_block_append_scope(ast::block* block) {
            for (ast::stmt* stmt: block->stmts) {
                resolve_stmt(stmt);
            }
        }

        void resolve_stmt(ast::stmt* stmt) {
            if (ast::var_decl* var_decl = dynamic_cast<ast::var_decl*>(stmt)) {
                auto f = scopes.back().var_map.find(var_decl->var.c_str());
                if (f) {
                    throw std::runtime_error("Redeclaration of variable: "+var_decl->var);
                }
                scopes.back().var_map.insert(var_decl->var.c_str())->val = var_decl;
                stack_blocks[scope_stmt].add_var(var_decl);
                if (ast::var_def* var_def = dynamic_cast<ast::var_def*>(stmt)) {
                    resolve_expr(var_def->expression);
                }
            } else if (ast::f_def* f_def = dynamic_cast<ast::f_def*>(stmt)) {
                auto f = scopes.back().func_map.find(f_def->name.c_str());
                if (f) {
                    throw std::runtime_error("Redefinition of function: "+f_def->name);
                }
                scopes.back().func_map.insert(f_def->name.c_str())->val = f_def;
                resolve_f_def(f_def);
            } else if (ast::c_if* c_if = dynamic_cast<ast::c_if*>(stmt)) {
                for (ast::if_branch* branch: c_if->branches) {
                    resolve_expr(branch->cond);
                    resolve_block(branch->body);
                }
            } else if (ast::c_for* c_for = dynamic_cast<ast::c_for*>(stmt)) {
                scopes.push_back(Scope());
                stack_blocks[scope_stmt = c_for] = StackBlock();
                resolve_stmt(c_for->init);
                resolve_expr(c_for->cond);
                resolve_stmt(c_for->upd);
                resolve_block_append_scope(c_for->body);
                scopes.pop_back();
            } else if (ast::c_while* c_while = dynamic_cast<ast::c_while*>(stmt)) {
                resolve_expr(c_while->cond);
                resolve_block(c_while->body);
            } else if (ast::expr* expr = dynamic_cast<ast::expr*>(stmt)) {
                resolve_expr(expr);
            } else if (ast::f_return* f_return = dynamic_cast<ast::f_return*>(stmt)) {
                resolve_expr(f_return->expression);
            } else if (ast::block* block = dynamic_cast<ast::block*>(stmt)) {
                resolve_block(block);
            } else {
                throw std::runtime_error("Unknown case");
            }
        }

        void resolve_expr(ast::expr* expr) {
            if (ast::var_ref* var_ref = dynamic_cast<ast::var_ref*>(expr)) {
                var_ref->resolve = find_var_decl(var_ref->name.c_str());
                if (!var_ref->resolve) {
                    var_imports.insert(var_ref);
                }
            } else if (ast::f_call* f_call = dynamic_cast<ast::f_call*>(expr)) {
                resolve_f_call(f_call);
            } else if (ast::literal* literal = dynamic_cast<ast::literal*>(expr)) {
                // do nothing
            } else if (ast::asn* asn = dynamic_cast<ast::asn*>(expr)) {
                asn->var->resolve = find_var_decl(asn->var->name.c_str());
                if (!asn->var->resolve) {
                    var_imports.insert(asn->var);
                }
                resolve_expr(asn->expression);
            } else if (ast::op_bin* op_bin = dynamic_cast<ast::op_bin*>(expr)) {
                resolve_expr(op_bin->lhs);
                resolve_expr(op_bin->rhs);
            } else if (ast::op_asn* op_asn = dynamic_cast<ast::op_asn*>(expr)) {
                op_asn->var->resolve = find_var_decl(op_asn->var->name.c_str());
                if (!op_asn->var->resolve) {
                    var_imports.insert(op_asn->var);
                }
                resolve_expr(op_asn->expression);
            } else {
                throw std::runtime_error("Unknown case");
            }
        }

        void resolve_f_def(ast::f_def* f_def) {
            scopes.push_back(Scope());
            stack_blocks[scope_stmt = f_def] = StackBlock();
            for (ast::var_decl* param: f_def->params) {
                scopes.back().var_map.insert(param->var.c_str())->val = param;
                stack_blocks[scope_stmt].add_var(param);
            }
            resolve_block_append_scope(f_def->body);
            scopes.pop_back();
        }

        void resolve_f_call(ast::f_call* f_call) {
            f_call->resolve = find_f_def(f_call->func.c_str());
            if (!f_call->resolve) {
                f_imports.insert(f_call);
            }
            for (ast::expr* arg: f_call->args) {
                resolve_expr(arg);
            }
        }
    };
}