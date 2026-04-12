#pragma once

#include "ast.hpp"
#include "tokenizer.hpp"

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
            next_semicolon = vector<int>(TOK_SIZE+1, INT_MAX);
            next_comma = vector<int>(TOK_SIZE+1, INT_MAX);
            // bracket pairs
            vector<pair<sym_t,int>> stk;
            for (int i = 0; i < TOK_SIZE; i++) {
                if (tokens[i].type == sym_t::ROUND_LEFT) {
                    stk.push_back({sym_t::ROUND_LEFT, i});
                } else if (tokens[i].type == sym_t::SQUARE_LEFT) {
                    stk.push_back({sym_t::SQUARE_LEFT, i});
                } else if (tokens[i].type == sym_t::CURLY_LEFT) {
                    stk.push_back({sym_t::CURLY_LEFT, i});
                } else if (tokens[i].type == sym_t::ROUND_RIGHT) {
                    if (stk.empty()) {
                        throw std::runtime_error("Unmatched right bracket");
                    }
                    int i0 = stk.back().second;
                    if (tokens[i0].type != sym_t::ROUND_LEFT) {
                        throw std::runtime_error("Unmatched right bracket");
                    }
                    closing_bracket[i0] = i;
                    closing_bracket[i] = i;
                    stk.pop_back();
                } else if (tokens[i].type == sym_t::SQUARE_RIGHT) {
                    if (stk.empty()) {
                        throw std::runtime_error("Unmatched right bracket");
                    }
                    int i0 = stk.back().second;
                    if (tokens[i0].type != sym_t::SQUARE_LEFT) {
                        throw std::runtime_error("Unmatched right bracket");
                    }
                    closing_bracket[i0] = i;
                    closing_bracket[i] = i;
                    stk.pop_back();
                } else if (tokens[i].type == sym_t::CURLY_RIGHT) {
                    if (stk.empty()) {
                        throw std::runtime_error("Unmatched right bracket");
                    }
                    int i0 = stk.back().second;
                    if (tokens[i0].type != sym_t::CURLY_LEFT) {
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
            for (int i = 1; i < TOK_SIZE; i++) {
                if (closing_bracket[i] == INT_MAX
                && tokens[i-1].type != sym_t::ROUND_RIGHT
                && tokens[i-1].type != sym_t::SQUARE_RIGHT
                && tokens[i-1].type != sym_t::CURLY_RIGHT) {
                    closing_bracket[i] = closing_bracket[i-1];
                }
            }
            // next semicolon and comma
            if (tokens[TOK_SIZE-1].type == sym_t::SEMI) {
                next_semicolon[TOK_SIZE] = TOK_SIZE-1;
            } else if (tokens[TOK_SIZE-1].type == sym_t::COMMA) {
                next_comma[TOK_SIZE] = TOK_SIZE-1;
            }
            for (int i = TOK_SIZE-1; i >= 0; i--) {
                if (tokens[i].type == sym_t::SEMI) {
                    next_semicolon[i] = i;
                } else {
                    next_semicolon[i] = next_semicolon[i+1];
                }
                if (tokens[i].type == sym_t::COMMA) {
                    next_comma[i] = i;
                } else {
                    next_comma[i] = next_comma[i+1];
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
                return NULL; // TODO: handle expr
                cout << "Invalid statement token: "
                    << tokens[begin].type << ' ' << tokens[begin].text << '\n';
                throw std::runtime_error("Invalid statement");
            }
        }

        ast::expr* parse_expr(int begin, int end) {
            return NULL;
        }

        ast::block* parse_block(int begin, int end) {
            return NULL;
        }

        ast::var_decl* parse_var_decl(int begin, int end) {
            if (tokens[begin].type != sym_t::TYPE) {
                throw std::runtime_error("Invalid variable declaration");
            }
            if (tokens[begin+1].type != sym_t::ID) {
                throw std::runtime_error("Invalid variable declaration");
            }
            string type = tokens[begin].text;
            string var = tokens[begin+1].text;
            curr = begin+2;
            return new ast::var_decl(type, var);
        }

        ast::var_def* parse_var_def(int begin, int end) {
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

        ast::f_def* parse_f_def(int begin, int end) {
            return NULL;
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
    };
}