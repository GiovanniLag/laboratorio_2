#include <vector>
#include <stdlib.h>
#include <math.h>

//Mean
double mean(std::vector<double> data)
{
    double sum = 0;
    for (int i = 0; i < data.size(); i++)
    {
        sum += data[i];
    }
    return sum / data.size();
}

//standard deviation
double standardDeviation(std::vector<double> data)
{
    double sum = 0;
    double m = mean(data);
    for (int i = 0; i < data.size(); i++)
    {
        sum += pow(data[i] - m, 2);
    }
    return sqrt(sum / (data.size() - 1));
}