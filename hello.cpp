#include <iostream>

extern "C" void say_hello() {
    std::cout << "Hello squirreled" << std::endl;
}

extern "C" void say_hello2() {
    std::cout << "I was added while the program was running" << std::endl;
}
