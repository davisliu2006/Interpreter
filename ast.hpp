#pragma once

#include "base.hpp"
#include "tokenizer.hpp"

namespace compiler {
    struct AST {
        struct node {
            sym_t type;
            vector<node*> children;
            node(sym_t type): type(type) {}
        };
    };

    struct ast {
        struct ASTNode {
            virtual ~ASTNode() = default;

            virtual string to_string() {
                return "ASTNode (unknown)";
            }
        };

        // statements (generic)
        struct stmt: ASTNode {
            virtual string to_string() {
                return "stmt (unknown)";
            }
        };
        struct simple_stmt: stmt {
            virtual string to_string() {
                return "simple_stmt (unknown)";
            }
        };
        struct expr: simple_stmt {
            string ret_type;

            virtual string to_string() {
                return "expr (unknown)";
            }
        };

        // block
        struct block: stmt {
            vector<stmt*> stmts;

            block(vector<stmt*> stmts): stmts(stmts) {}
            virtual ~block() {
                for (stmt* stmt: stmts) {delete stmt;}
            }

            virtual string to_string() {
                string val = "block {\n";
                for (stmt* stmt: stmts) {
                    val += (stmt? stmt->to_string() : "NULL")+"\n";
                }
                val += "}";
                return val;
            }
        };

        // variables
        struct var_decl: simple_stmt {
            string type;
            string var;

            var_decl(string type, string var): type(type), var(var) {}
            virtual ~var_decl() {}

            virtual string to_string() {
                return "var_decl " + type + " " + var;
            }
        };
        struct var_def: var_decl {
            expr* expression;

            var_def(string type, string var, expr* expression):
            var_decl(type, var), expression(expression) {}
            virtual ~var_def() {
                delete expression;
            }
            
            virtual string to_string() {
                return "var_def "+type+" "+var+" = (\n"+(expression? expression->to_string() : "NULL")+"\n)";
            }
        };
        struct asn: expr {
            string var;
            expr* expression;

            asn(string var, expr* expression): var(var), expression(expression) {}
            virtual ~asn() {
                delete expression;
            }

            virtual string to_string() {
                return "asn "+var+" = (\n"+(expression? expression->to_string() : "NULL")+"\n)";
            }
        };

        // functions
        struct f_def: stmt {
            string type;
            string name;
            vector<var_decl*> params;
            block* body;

            f_def(string type, vector<var_decl*> params, block* body):
            type(type), params(params), body(body) {}
            virtual ~f_def() {
                for (var_decl* param: params) {delete param;}
                delete body;
            }

            virtual string to_string() {
                string val = "f_def " + type + " " + name + "(\n";
                for (int i = 0; i < params.size(); i++) {
                    val += params[i]? params[i]->to_string() : "NULL";
                    if (i != params.size()-1) {val += ",\n";}
                }
                val += "\n) {\n"+(body? body->to_string() : "NULL")+"\n}";
                return val;
            }
        };
        struct f_call: expr {
            string func;
            vector<expr*> args;

            f_call(string func, vector<expr*> args): func(func), args(args) {}
            virtual ~f_call() {
                for (expr* arg: args) {delete arg;}
            }

            virtual string to_string() {
                string val = "f_call " + func + "(\n";
                for (int i = 0; i < args.size(); i++) {
                    val += args[i]? args[i]->to_string() : "NULL";
                    if (i != args.size()-1) {val += ",\n";}
                }
                val += "\n)";
                return val;
            }
        };

        // control
        struct if_branch: ASTNode {
            expr* cond;
            block* body;

            if_branch(expr* cond, block* body): cond(cond), body(body) {}
            virtual ~if_branch() {
                delete cond;
                delete body;
            }

            virtual string to_string() {
                return "if_branch (\n"+(cond? cond->to_string() : "NULL")+"\n) {\n"+(body? body->to_string() : "NULL")+"\n}";
            }
        };
        struct c_if: stmt {
            vector<if_branch*> branches;

            c_if(vector<if_branch*> branches): branches(branches) {}
            virtual ~c_if() {
                for (if_branch* branch: branches) {delete branch;}
            }

            virtual string to_string() {
                string val = "c_if";
                for (if_branch* branch: branches) {
                    assert(branch);
                    val += "\n"+branch->to_string();
                }
                return val;
            }
        };
        struct c_for: stmt {
            stmt* init;
            expr* cond;
            stmt* upd;
            block* body;

            c_for(stmt* init, expr* cond, stmt* upd, block* body):
            init(init), cond(cond), upd(upd), body(body) {}
            virtual ~c_for() {
                delete init;
                delete cond;
                delete upd;
                delete body;
            }

            virtual string to_string() {
                return "c_for (\n"+(init? init->to_string() : "NULL")+";\n"+(cond? cond->to_string() : "NULL")+";\n"+(upd? upd->to_string() : "NULL")+"\n) {\n"+(body? body->to_string() : "NULL")+"\n}";
            }
        };
        struct c_while: stmt {
            expr* cond;
            block* body;

            c_while(expr* cond, block* body): cond(cond), body(body) {}
            virtual ~c_while() {
                delete cond;
                delete body;
            }

            virtual string to_string() {
                return "c_while (\n"+(cond? cond->to_string() : "NULL")+"\n) {\n"+(body? body->to_string() : "NULL")+"\n}";
            }
        };
    };
}