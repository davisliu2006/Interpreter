#include <functional>
#include <iostream>
#include <string>

struct Test {
    std::string name;
    std::function<void()> func;

    Test(const std::string& name, const std::function<void()>& func): name(name), func(func) {}

    void run() const {
        std::cout << "Running test: \"" << name << "\"\n" << std::flush;
        func();
        std::cout << "    Test passed\n" << std::flush;
    }
};