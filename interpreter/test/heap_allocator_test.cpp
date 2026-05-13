#include <cassert>
#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../heap_allocator.hpp"

using namespace std;
using namespace interpreter;

vector<Test> tests = {
    Test("simple_alloc", []() {
        HeapAllocator heap_allocator(1024*16);
        int8_t* block = heap_allocator.simple_alloc(1024);
        cout << heap_allocator.to_debug_string() << '\n';
        heap_allocator.simple_free(block);
        cout << heap_allocator.to_debug_string() << '\n';
    })
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    cout << setprecision(15);

    for (const Test& test: tests) {
        test.run();
    }

    return 0;
}