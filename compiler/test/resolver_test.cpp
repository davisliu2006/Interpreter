#include <cassert>
#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../tokenizer.hpp"
#include "../parser.hpp"
#include "../resolver.hpp"

using namespace std;
using namespace compiler;

vector<Test> tests = {
    Test("basic", []() {
        try {
            ast::block* top_level = new ast::block({
                new ast::var_decl("float", "x"),
                new ast::var_def("float", "y", new ast::var_ref("x")),
                new ast::f_def("int", "add", {
                    new ast::var_decl("int", "x"), new ast::var_decl("int", "y")
                }, new ast::block({
                    new ast::f_return(new ast::op_bin(
                        "+", new ast::var_ref("x"), new ast::var_ref("y")
                    ))
                }))
            });
            Resolver resolver(top_level);
            resolver.resolve();
            cout << top_level->to_formatted_string() << '\n';
            for (const auto& [key, val]: resolver.stack_blocks) {
                cout << val << '\n';
            }
            delete top_level;
        } catch (const std::runtime_error& e) {
            cout << e.what() << '\n';
        }
    }),
    Test("nested", []() {
        try {
            ast::block* top_level = new ast::block({
                new ast::var_def("float", "PI", new ast::literal(sym_t::FLOAT, "3.1415926535")),
                new ast::f_def("void", "proc", {
                    new ast::var_decl("int", "x"),
                }, new ast::block({
                    new ast::var_def("int", "n", new ast::literal(sym_t::INT, "100")),
                    new ast::f_def("int", "inner", {
                        new ast::var_decl("int", "y"),
                    }, new ast::block({
                        new ast::asn("n", new ast::op_bin("+", new ast::var_ref("y"), new ast::var_ref("PI"))),
                        new ast::f_return(
                            new ast::op_bin("+", new ast::var_ref("x"), new ast::var_ref("n"))
                        )
                    }))
                }))
            });
            Resolver resolver(top_level);
            resolver.resolve();
            cout << top_level->to_formatted_string() << '\n';
            for (const auto& [key, val]: resolver.stack_blocks) {
                cout << val << '\n';
            }
            delete top_level;
        } catch (const std::runtime_error& e) {
            cout << e.what() << '\n';
        }
    }),
    Test("scope", []() {
        try {
            ast::block* top_level = new ast::block({
                new ast::block({
                    new ast::var_def("int", "x", new ast::literal(sym_t::INT, "100")),
                }),
                new ast::var_ref("x"),
                new ast::c_if({
                    new ast::if_branch(
                        new ast::op_bin("==", new ast::var_ref("x"), new ast::literal(sym_t::INT, "0")),
                        new ast::block({
                            new ast::var_ref("x"),
                            new ast::var_decl("int", "x")
                        })
                    ),
                    new ast::if_branch(
                        new ast::op_bin("==", new ast::var_ref("x"), new ast::literal(sym_t::INT, "0")),
                        new ast::block({
                            new ast::var_ref("x")
                        })
                    )
                }),
                new ast::var_ref("x"),
                new ast::c_for(
                    new ast::var_def("int", "x", new ast::literal(sym_t::INT, "0")),
                    new ast::op_bin("<", new ast::var_ref("x"), new ast::literal(sym_t::INT, "10")),
                    new ast::var_ref("x"),
                    new ast::block({
                        new ast::var_ref("x")
                    })
                ),
                new ast::var_ref("x"),
                new ast::f_def("void", "f", {
                    new ast::var_decl("int", "x"),
                }, new ast::block({
                    new ast::var_ref("x")
                })),
                new ast::f_call("f", {
                    new ast::var_ref("x")
                })
            });
            Resolver resolver(top_level);
            resolver.resolve();
            cout << top_level->to_formatted_string() << '\n';
            for (const auto& [key, val]: resolver.stack_blocks) {
                cout << val << '\n';
            }
            delete top_level;
        } catch (const std::runtime_error& e) {
            cout << e.what() << '\n';
        }
    }),
    Test("unary_expr", []() {
        try {
            ast::block* top_level = new ast::block({
                new ast::var_def("int", "x", new ast::literal(sym_t::INT, "100")),
                new ast::var_def("int", "y", new ast::op_un("-", new ast::var_ref("x")))
            });
            Resolver resolver(top_level);
            resolver.resolve();
            ast::var_def* y = dynamic_cast<ast::var_def*>(top_level->stmts[1]);
            assert(y);
            ast::op_un* op_un = dynamic_cast<ast::op_un*>(y->expression);
            assert(op_un && op_un->op == "-");
            ast::var_ref* x = dynamic_cast<ast::var_ref*>(op_un->expression);
            assert(x && x->resolve && x->resolve->var == "x");
            delete top_level;
        } catch (const std::runtime_error& e) {
            cout << e.what() << '\n';
            throw e;
        }
    })
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