#include <iostream>

extern "C"
{
    int add(int a, int b)
    {
        std::cerr << "add called, result = " << a + b << std::endl;
        return a + b;
    }

    void print(const char* str)
    {
        std::cerr << "output from print: " << str << std::endl;
    }
}