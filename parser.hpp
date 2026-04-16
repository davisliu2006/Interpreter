#pragma once

#include "ast.hpp"
#include "symbols.hpp"
#include <climits>

namespace compiler {
    struct Parser {
        vector<Token> tokens;
        vector<int> closing_bracket;
        vector<int> next_semicolon;
        vector<int> next_comma;
        int curr = 0;
        int TOK_SIZE = 0;

        int next_seperator (int begin) {
            int cb = closing_bracket[begin];
            int ns = next_semicolon[begin];
            int nc = next_comma[begin];
            return std::min({cb, ns, nc});
        }

        Parser(vector<Token> tokens): tokens(tokens), TOK_SIZE(tokens.size()) {
            closing_bracket = vector<int>(TOK_SIZE, INT_MAX);
            next_semicolon = vector<int>(TOK_SIZE, INT_MAX);
            next_comma = vector<int>(TOK_SIZE, INT_MAX);
            // closing bracket (pairs first)
            {
                vector<pair<sym_t,int>> stk;
                for (int i = 0; i < TOK_SIZE; i++) {
                    if (is_opening_bracket(tokens[i].type)) {
                        stk.push_back({tokens[i].type, i});
                    } else if (is_closing_bracket(tokens[i].type)) {
                        if (stk.empty()) {
                            throw std::runtime_error("Unmatched right bracket");
                        }
                        int i0 = stk.back().second;
                        if (tokens[i0].type != bracket_pair_type(tokens[i].type)) {
                            throw std::runtime_error("Unmatched right bracket");
                        }
                        closing_bracket[i0] = i;
                        closing_bracket[i] = i;
                        stk.pop_back();
                    }
                }
                if (!stk.empty()) {
                    throw std::runtime_error("Unmatched left bracket");
                }
            }
            // closing bracket (fill in gaps)
            {
                vector<int> stk;
                for (int i = 0; i < TOK_SIZE; i++) {
                    if (is_opening_bracket(tokens[i].type)) {
                        stk.push_back(i);
                    } else if (is_closing_bracket(tokens[i].type)) {
                        stk.pop_back();
                    } else if (!stk.empty()) {
                        closing_bracket[i] = closing_bracket[stk.back()];
                    }
                }
            }
            // next semicolon and comma
            {
                vector<pair<int,int>> stk;
                stk.push_back({INT_MAX, INT_MAX});
                for (int i = TOK_SIZE-1; i >= 0; i--) {
                    if (is_closing_bracket(tokens[i].type)) {
                        stk.push_back({INT_MAX, INT_MAX});
                    } else if (tokens[i].type == sym_t::SEMI) {
                        stk.back().first = i;
                    } else if (tokens[i].type == sym_t::COMMA) {
                        stk.back().second = i;
                    }
                    next_semicolon[i] = stk.back().first;
                    next_comma[i] = stk.back().second;
                    if (is_opening_bracket(tokens[i].type)) {
                        stk.pop_back();
                    } 
                }
            }
        }

        ast::stmt* parse_stmt(int begin, int end) {
            if (tokens[begin].type == sym_t::IF) { // IF
                return parse_c_if(begin, end);
            } else if (tokens[begin].type == sym_t::FOR) { // FOR
                return parse_c_for(begin, end);
            } else if (tokens[begin].type == sym_t::WHILE) { // WHILE
                return parse_c_while(begin, end);
            } else if ((tokens[begin].type == sym_t::ID || tokens[begin].type == sym_t::TYPE)
            && tokens[begin+1].type == sym_t::ID && tokens[begin+2].type == sym_t::ROUND_RIGHT) { // TYPE ID(
                /*
                Note: At this point, we don't know if it's ID or TYPE if it's not a builtin type
                but we can treat it as a type and do typechecking later
                */
                tokens[begin].type = sym_t::TYPE;
                return parse_f_def(begin, end);
            } else {
                return parse_simple_stmt(begin, end);
            }
        }

        ast::simple_stmt* parse_simple_stmt(int begin, int end) {
            if (tokens[begin].type == sym_t::ID
            && tokens[begin+1].type == sym_t::OPERATOR && tokens[begin+1].text == "=") { // ID =
                return parse_asn(begin, end);
            } else if (tokens[begin].type == sym_t::ID
            && tokens[begin+1].type == sym_t::ROUND_LEFT) { // ID(
                return parse_f_call(begin, end);
            } else if ((tokens[begin].type == sym_t::ID || tokens[begin].type == sym_t::TYPE)
            && tokens[begin+1].type == sym_t::ID) { // TYPE ID
                /*
                Note: At this point, we don't know if it's ID or TYPE if it's not a builtin type
                but we can treat it as a type and do typechecking later
                */
                tokens[begin].type = sym_t::TYPE;
                return parse_var_decl(begin, end);
            } else {
                return parse_expr(begin, end);
            }
        }

        ast::expr* parse_expr(int begin, int end) {
            if (begin >= end) {
                throw std::runtime_error("Invalid expression");
            }
            if (begin+1 == end) {
                if (tokens[begin].type == sym_t::ID) {
                    return new ast::var_ref(tokens[begin].text);
                } else if (is_literal(tokens[begin].type)) {
                    return new ast::literal(tokens[begin].type, tokens[begin].text);
                } else {
                    cout << "Invalid expression token: "
                        << tokens[begin].type << ' ' << tokens[begin].text << '\n';
                    throw std::runtime_error("Invalid expression");
                }
            }
            if (tokens[begin].type == sym_t::ROUND_LEFT
            && closing_bracket[begin] == end-1) { // (expr)
                return parse_expr(begin+1, end-1);
            }
            int op_pos = -1;
            int op_prec = INT_MIN;
            for (int i = begin; i < end; i++) {
                if (is_opening_bracket(tokens[i].type)) {
                    i = closing_bracket[i];
                } else if (tokens[i].type == sym_t::OPERATOR) {
                    int prec = operator_precedence.find(tokens[i].text.c_str(), tokens[i].text.size())->val;
                    if (prec > op_prec) {
                        op_prec = prec;
                        op_pos = i;
                    }
                }
            }
            if (op_pos == -1) {
                throw std::runtime_error("Invalid expression, no operator");
            } else {
                return parse_expr_helper(begin, op_pos, end);
            }
        }

        ast::block* parse_block(int begin, int end) {
            return NULL;
        }

        ast::var_decl* parse_var_decl(int begin, int end) { // TYPE ID
            if (tokens[begin].type != sym_t::TYPE) {
                throw std::runtime_error("Invalid variable declaration");
            }
            if (tokens[begin+1].type != sym_t::ID) {
                throw std::runtime_error("Invalid variable declaration");
            }
            if (tokens[begin+2].type == sym_t::OPERATOR && tokens[begin+2].text == "=") { // TYPE ID =
                return parse_var_def(begin, end);
            }
            string type = tokens[begin].text;
            string var = tokens[begin+1].text;
            curr = begin+2;
            return new ast::var_decl(type, var);
        }

        ast::var_def* parse_var_def(int begin, int end) { // TYPE ID = expr
            string type = tokens[begin].text;
            string var = tokens[begin+1].text;
            ast::expr* expression = parse_expr(begin+3, end);
            return NULL;
        }

        ast::asn* parse_asn(int begin, int end) { // ID = expr
            if (tokens[begin].type != sym_t::ID) {
                throw std::runtime_error("Invalid assignment");
            }
            if (tokens[begin+1].type != sym_t::OPERATOR || tokens[begin+1].text != "=") {
                throw std::runtime_error("Invalid assignment");
            }
            string var = tokens[begin].text;
            ast::expr* expression = parse_expr(begin+2, end);
            return new ast::asn(var, expression);
        }

        ast::op_asn* parse_op_asn(int begin, int end) { // ID OP= expr
            return NULL;
        }

        ast::op_bin* parse_op_bin(int begin, int end) { // expr OP expr
            return NULL;
        }

        ast::f_def* parse_f_def(int begin, int end) { // TYPE ID(params) {body}
            if (tokens[begin].type != sym_t::TYPE) {
                throw std::runtime_error("Invalid function definition");
            }
            if (tokens[begin+1].type != sym_t::ID) {
                throw std::runtime_error("Invalid function definition");
            }
            if (tokens[begin+2].type != sym_t::ROUND_LEFT) {
                throw std::runtime_error("Invalid function definition");
            }
            string ret_type = tokens[begin].text;
            string name = tokens[begin+1].text;
            vector<ast::var_decl*> params;
            int param_begin = begin+3;
            int param_end;
            while (true) {
                param_end = next_seperator(param_begin);
                if (tokens[param_begin].type != sym_t::TYPE) {
                    throw std::runtime_error("Invalid function definition");
                }
                if (tokens[param_begin+1].type != sym_t::ID) {
                    throw std::runtime_error("Invalid function definition");
                }
                params.push_back(new ast::var_decl(
                    tokens[param_begin].text,
                    tokens[param_begin+1].text
                ));
                if (tokens[param_end].type == sym_t::ROUND_RIGHT) {
                    break;
                }
            }
            if (tokens[param_end+1].type != sym_t::CURLY_LEFT) {
                throw std::runtime_error("Invalid function definition");
            }
            int block_begin = param_end+2;
            int block_end = closing_bracket[block_begin];
            ast::block* body = parse_block(block_begin, block_end);
            curr = block_end+1;
            return new ast::f_def(ret_type, name, params, body);
        }

        ast::f_call* parse_f_call(int begin, int end) { // ID(args)
            if (tokens[begin].type != sym_t::ID) {
                throw std::runtime_error("Invalid function call");
            }
            if (tokens[begin+1].type != sym_t::ROUND_LEFT) {
                throw std::runtime_error("Invalid function call");
            }
            vector<ast::expr*> args;
            int arg_begin = begin+2;
            int arg_end;
            while (true) {
                arg_end = next_seperator(arg_begin);
                ast::expr* arg = parse_expr(arg_begin, arg_end);
                args.push_back(arg);
                arg_begin = arg_end+1;
                if (tokens[arg_end].type == sym_t::ROUND_RIGHT) {
                    break;
                }
            }
            curr = arg_end+1;
            return new ast::f_call(tokens[begin].text, args);
        }

        ast::c_if* parse_c_if(int begin, int end) { // IF (cond) {body}
            if (tokens[begin].type != sym_t::IF) {
                throw std::runtime_error("Invalid if statement");
            }
            if (tokens[begin+1].type != sym_t::ROUND_LEFT) {
                throw std::runtime_error("Invalid if statement");
            }
            int cond_begin = begin+2;
            int cond_end = closing_bracket[cond_begin];
            ast::expr* cond = parse_expr(cond_begin, cond_end);
            if (tokens[cond_end+1].type != sym_t::CURLY_LEFT) {
                throw std::runtime_error("Invalid if statement");
            }
            int then_begin = cond_end+2;
            int then_end = closing_bracket[then_begin];
            ast::block* then_body = parse_block(then_begin, then_end);
            vector<ast::if_branch*> branches;
            branches.push_back(new ast::if_branch(cond, then_body));
            while (tokens[then_end+1].type == sym_t::ELSE) { // ELSE ...
                if (tokens[then_end+2].type == sym_t::IF) { // ELSE IF (cond) {body}
                    if (tokens[then_end+3].type != sym_t::ROUND_LEFT) {
                        throw std::runtime_error("Invalid if statement");
                    }
                    cond_begin = then_end+4;
                    cond_end = closing_bracket[cond_begin];
                    ast::expr* cond = parse_expr(cond_begin, cond_end);
                    if (tokens[cond_end+1].type != sym_t::CURLY_LEFT) {
                        throw std::runtime_error("Invalid if statement");
                    }
                    then_begin = cond_end+2;
                    then_end = closing_bracket[then_begin];
                    ast::block* then_body = parse_block(then_begin, then_end);
                    branches.push_back(new ast::if_branch(cond, then_body));
                } else { // ELSE {body}
                    if (tokens[then_end+2].type != sym_t::CURLY_LEFT) {
                        throw std::runtime_error("Invalid if statement");
                    }
                    int else_begin = then_end+3;
                    int else_end = closing_bracket[else_begin];
                    ast::block* else_body = parse_block(else_begin, else_end);
                    branches.push_back(new ast::if_branch(NULL, else_body));
                    break;
                }
            }
            curr = then_end+1;
            return new ast::c_if(branches);
        }

        ast::c_for* parse_c_for(int begin, int end) { // FOR (init; cond; upd) {body}
            if (tokens[begin].type != sym_t::FOR) {
                throw std::runtime_error("Invalid for statement");
            }
            if (tokens[begin+1].type != sym_t::ROUND_LEFT) {
                throw std::runtime_error("Invalid for statement");
            }
            int init_begin = begin+2;
            int init_end = next_semicolon[init_begin];
            ast::stmt* init = parse_stmt(init_begin, init_end);
            int cond_begin = init_end+1;
            int cond_end = next_semicolon[cond_begin];
            ast::expr* cond = parse_expr(cond_begin, cond_end);
            int upd_begin = cond_end+1;
            int upd_end = closing_bracket[upd_begin];
            ast::stmt* upd = parse_stmt(upd_begin, upd_end);
            if (tokens[upd_end+1].type != sym_t::CURLY_LEFT) {
                throw std::runtime_error("Invalid for statement");
            }
            int body_begin = upd_end+2;
            int body_end = closing_bracket[body_begin];
            ast::block* body = parse_block(body_begin, body_end);
            curr = body_end+1;
            return new ast::c_for(init, cond, upd, body);
        }

        ast::c_while* parse_c_while(int begin, int end) { // WHILE (cond) {body}
            if (tokens[begin].type != sym_t::WHILE) {
                throw std::runtime_error("Invalid while statement");
            }
            if (tokens[begin+1].type != sym_t::ROUND_LEFT) {
                throw std::runtime_error("Invalid while statement");
            }
            int cond_begin = begin+2;
            int cond_end = closing_bracket[cond_begin];
            ast::expr* cond = parse_expr(cond_begin, cond_end);
            if (tokens[cond_end+1].type != sym_t::CURLY_LEFT) {
                throw std::runtime_error("Invalid while statement");
            }
            int body_begin = cond_end+2;
            int body_end = closing_bracket[body_begin];
            ast::block* body = parse_block(body_begin, body_end);
            curr = body_end+1;
            return new ast::c_while(cond, body);
        }

        ast::expr* parse_expr_helper(int begin, int op_pos, int end) {
            // TODO: support and clean up expr cases
            if (op_pos > begin && op_pos < end-1) {
                ast::expr* lhs = parse_expr(begin, op_pos);
                ast::expr* rhs = parse_expr(op_pos+1, end);
                return new ast::op_bin(tokens[op_pos].text, lhs, rhs);
            } else if (op_pos == begin) { 
                if (tokens[op_pos].text == "++") {
                    return new ast::op_asn(
                        tokens[op_pos+1].text, "+",
                        new ast::literal(sym_t::INT, "1")
                    );
                } else if (tokens[op_pos].text == "--") {
                    return new ast::op_asn(
                        tokens[op_pos+1].text, "-",
                        new ast::literal(sym_t::INT, "1")
                    );
                } else {
                    cout << "Invalid expression operator: " << tokens[op_pos].text << '\n';
                    throw std::runtime_error("Invalid expression");
                }
            } else {
                assert(op_pos == end-1);
                if (tokens[op_pos].text == "++") {
                    return new ast::op_asn(
                        tokens[op_pos-1].text, "+",
                        new ast::literal(sym_t::INT, "1")
                    );
                } else if (tokens[op_pos].text == "--") {
                    return new ast::op_asn(
                        tokens[op_pos-1].text, "-",
                        new ast::literal(sym_t::INT, "1")
                    );
                } else {
                    cout << "Invalid expression operator: " << tokens[op_pos].text << '\n';
                    throw std::runtime_error("Invalid expression");
                }
            }
        }
    };
}