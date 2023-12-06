#include <iostream>

double file2_div(double a, double b) 
{
    if (b != 0)
    {
        return a / b;
    } 
    else
    {
        std::cerr << "Error! Division by zero.\n";
        return 0;
    }
}

double file2_add(double a, double b)
{   
    double sum = a+b;
    return sum;
}

int main()
{
    int a = 4;
    int b = 6;
    std::cout << file2_add(a,b);
}
