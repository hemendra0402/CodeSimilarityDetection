#include <iostream>
int function2(int a, int b)
{   
    int sub= a-b;
    return sub;
}

int main()
{
    int a = 4;
    int b = 6;
    std::cout<<function2(a,b);
}