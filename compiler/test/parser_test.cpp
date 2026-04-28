#include <cassert>
#include <climits>
#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../tokenizer.hpp"
#include "../parser.hpp"

using namespace std;
using namespace compiler;

string display_str(int x) {
    if (x == INT_MAX) {return "INT_MAX";}
    else {return to_string(x);}
}

vector<Test> tests = {
    Test("brackets", []() {
        auto tokens = tokenizer::tokenize(";[(1+2),{3,4,[;;]}],({5,6});");
        Parser parser(tokens);
        // --------------------------------0123456789012345678901234567
        // --------------------------------0         1         2
        cout << "    ";
        for (int i = 0; i < parser.closing_bracket.size(); i++) {
            cout << display_str(parser.closing_bracket[i]) << ' ';
        }
        cout << "\n    ";
        for (int i = 0; i < parser.next_comma.size(); i++) {
            cout << display_str(parser.next_comma[i]) << ' ';
        }
        cout << "\n    ";;
        for (int i = 0; i < parser.next_semicolon.size(); i++) {
            cout << display_str(parser.next_semicolon[i]) << ' ';
        }
        cout << '\n';
        vector<int> expected_closing_bracket = vector<int>{
            INT_MAX, 18, 6, 6, 6, 6, 6, 18,
            17, 17, 17, 17, 17, 16, 16, 16, 16, 17, 18, INT_MAX,
            26, 25, 25, 25, 25, 25, 26, INT_MAX
        };
        vector<int> expected_next_comma = vector<int>{
            19, 7, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 7,
            10, 10, 10, 12, 12, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX,
            19, INT_MAX, 23, 23, 23, INT_MAX, INT_MAX, INT_MAX, INT_MAX
        };
        vector<int> expected_next_semicolon = vector<int>{
            0, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX,
            INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 14, 14, 15, INT_MAX, INT_MAX, INT_MAX, 27,
            INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 27
        };
        assert(parser.closing_bracket == expected_closing_bracket);
        assert(parser.next_comma == expected_next_comma);
        assert(parser.next_semicolon == expected_next_semicolon);
    }),
    Test("while", []() {
        try {
            auto tokens = tokenizer::tokenize("while (x == 0) {y = 1;}");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            assert(dynamic_cast<ast::c_while*>(ast));
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("for", []() {
        try {
            auto tokens = tokenizer::tokenize("for (int i = 0; i < 10; i++) {}");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            assert(dynamic_cast<ast::c_for*>(ast));
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("if_fallback", []() {
        try {
            auto tokens = tokenizer::tokenize("if (x == 0) {y = 1;}\nelse if (x == 1) {y = 2;}\nelse {y = 3;}");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::c_if* c_if = dynamic_cast<ast::c_if*>(ast);
            assert(c_if);
            assert(c_if->branches.size() == 3);
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("if_no_fallback", []() {
        try {
            auto tokens = tokenizer::tokenize("if (x == 0) {y = 1;}\nelse if (x == 1) {y = 2;}\nelse if (x == 2) {y = 3;}");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::c_if* c_if = dynamic_cast<ast::c_if*>(ast);
            assert(c_if);
            assert(c_if->branches.size() == 3);
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("f_call", []() {
        try {
            auto tokens = tokenizer::tokenize("f(1+2, 3, abc)");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::f_call* f_call = dynamic_cast<ast::f_call*>(ast);
            assert(f_call);
            assert(f_call->args.size() == 3);
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("var_decl", []() {
        try {
            auto tokens = tokenizer::tokenize("int xyz");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            assert(dynamic_cast<ast::var_decl*>(ast));
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("expr_binary", []() {
        try {
            auto tokens = tokenizer::tokenize("(a+b)*(c+d)+f/(g-h)");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::op_bin* expr = dynamic_cast<ast::op_bin*>(ast);
            assert(expr); assert(expr->op == "+");
            ast::op_bin* expr_l = dynamic_cast<ast::op_bin*>(expr->lhs);
            ast::op_bin* expr_r = dynamic_cast<ast::op_bin*>(expr->rhs);
            assert(expr_l && expr_l->op == "*"); assert(expr_r && expr_r->op == "/");
            ast::op_bin* expr_ll = dynamic_cast<ast::op_bin*>(expr_l->lhs);
            ast::op_bin* expr_lr = dynamic_cast<ast::op_bin*>(expr_l->rhs);
            assert(expr_ll && expr_ll->op == "+"); assert(expr_lr && expr_lr->op == "+");
            ast::var_ref* expr_lll = dynamic_cast<ast::var_ref*>(expr_ll->lhs);
            assert(expr_lll && expr_lll->name == "a");
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("expr_with_f_call", []() {
        try {
            auto tokens = tokenizer::tokenize("f(a+b, c*d) + g(e)/f");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::op_bin* expr = dynamic_cast<ast::op_bin*>(ast);
            assert(expr); assert(expr->op == "+");
            ast::f_call* expr_l = dynamic_cast<ast::f_call*>(expr->lhs);
            ast::op_bin* expr_r = dynamic_cast<ast::op_bin*>(expr->rhs);
            assert(expr_l && expr_l->args.size() == 2);
            assert(expr_r && expr_r->op == "/");            
            ast::f_call* expr_rl = dynamic_cast<ast::f_call*>(expr_r->lhs);
            assert(expr_rl); assert(expr_rl->func == "g");
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("expr_asn", []() {
        try {
            auto tokens = tokenizer::tokenize("x = 1+2");
            Parser parser(tokens);
            auto* ast = parser.parse_expr(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::asn* asn = dynamic_cast<ast::asn*>(ast);
            assert(asn);
            assert(asn->var->name == "x");
            ast::op_bin* expr = dynamic_cast<ast::op_bin*>(asn->expression);
            assert(expr && expr->op == "+");
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("expr_op_asn", []() {
        try {
            auto tokens = tokenizer::tokenize("x += y*2");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::op_asn* op_asn = dynamic_cast<ast::op_asn*>(ast);
            assert(op_asn);
            assert(op_asn->var->name == "x");
            assert(op_asn->op == "+");
            ast::op_bin* expr = dynamic_cast<ast::op_bin*>(op_asn->expression);
            assert(expr && expr->op == "*");
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("expr_unary", []() {
        try {
            auto tokens = tokenizer::tokenize("-x+~y*z");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::op_bin* expr = dynamic_cast<ast::op_bin*>(ast);
            assert(expr && expr->op == "+");
            ast::op_un* expr_l = dynamic_cast<ast::op_un*>(expr->lhs);
            assert(expr_l && expr_l->op == "-");
            ast::op_bin* expr_r = dynamic_cast<ast::op_bin*>(expr->rhs);
            assert(expr_r && expr_r->op == "*");
            ast::op_un* expr_rl = dynamic_cast<ast::op_un*>(expr_r->lhs);
            assert(expr_rl && expr_rl->op == "~");
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("block_simple", []() {
        try {
            auto tokens = tokenizer::tokenize("int x = 1; int y = 2;");
            Parser parser(tokens);
            auto* ast = parser.parse_block(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            assert(ast->stmts.size() == 2);
            ast::var_def* stmt1 = dynamic_cast<ast::var_def*>(ast->stmts[0]);
            ast::var_def* stmt2 = dynamic_cast<ast::var_def*>(ast->stmts[1]);
            assert(stmt1); assert(stmt2);
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("f_def", []() {
        try {
            auto tokens = tokenizer::tokenize("int f(int x, int y) {return x+y;}");
            Parser parser(tokens);
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            ast::f_def* f_def = dynamic_cast<ast::f_def*>(ast);
            assert(f_def);
            assert(f_def->params.size() == 2);
            ast::block* body = dynamic_cast<ast::block*>(f_def->body);
            assert(body); assert(body->stmts.size() == 1);
            ast::f_return* f_return = dynamic_cast<ast::f_return*>(body->stmts[0]);
            assert(f_return);
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    cout << setprecision(15);

    tokenizer::init();

    for (const Test& test: tests) {
        test.run();
    }

    return 0;
}