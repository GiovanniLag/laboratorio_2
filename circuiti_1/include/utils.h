#include <vector>
#include <iostream>
#include <string>

double min(std::vector<double> data)
{
    double min = data[0];
    for (int i = 1; i < data.size(); i++)
    {
        if (data[i] < min)
        {
            min = data[i];
        }
    }
    return min;
}

double max(std::vector<double> data)
{
    double max = data[0];
    for (int i = 1; i < data.size(); i++)
    {
        if (data[i] > max)
        {
            max = data[i];
        }
    }
    return max;
}