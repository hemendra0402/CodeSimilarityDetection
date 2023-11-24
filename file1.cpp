#include <iostream>

int function1(int a, int b)
{   
    int sum= a+b;
    return sum;
}

int main()
{
    int a = 4;
    int b = 6;
    std::cout<<function1(a,b);
}
