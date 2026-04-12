#include <bits/stdc++.h>
#include "tokenizer.hpp"

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cout << setprecision(15);

    compiler::tokenizer::init();
    auto tokens = compiler::tokenizer::tokenize("int x = x0 0 x;");
    for (const auto& [type, token]: tokens) {
        cout << type << ' ' << token << '\n';
    }

    return 0;
}