#include <iostream>

double file1_mul(double a, int b)
{
    double answer = 0;
    if (b < 0)
    {
        std::cerr << "Error! Negative values not supported.\n";
        return 0;
    }

    for (int i = 0; i < b; i++)
    {
        answer += a;
    }
    return answer;
}

double fil1_add(double a, double b)
{   
    double sum = a+b;
    return sum;
}

int main()
{
    int a = 4;
    int b = 6;
    std::cout << fil1_add(a,b);
}
