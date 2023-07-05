#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

//function to wrtite a vector of vectors of duble to a csv file
void writeCSV(std::vector<std::vector<double>> data, std::vector<std::string>colums_names,std::string filename)
{
    std::ofstream file(filename);
    //write the column names
    for (int i = 0; i < colums_names.size(); i++)
    {
        file << colums_names[i];
        if (i != colums_names.size() - 1)
        {
            file << ",";
        }
    }
    file << "\n";
    //write the data
    for (int i = 0; i < data.size(); i++)
    {
        for (int j = 0; j < data[i].size(); j++)
        {
            file << data[i][j];
            if (j != data[i].size() - 1)
            {
                file << ",";
            }
        }
        file << "\n";
    }
    file.close();
}

void writeCSV(std::vector<double> data, std::vector<std::string>colums_names,std::string filename)
{
    std::ofstream file(filename);
    //write the column names
    for (int i = 0; i < colums_names.size(); i++)
    {
        file << colums_names[i];
        if (i != colums_names.size() - 1)
        {
            file << ",";
        }
    }
    file << "\n";
    //write the data
    for (int i = 0; i < data.size(); i++)
    {
        file << data[i];
        if (i != data.size() - 1)
        {
            file << ",";
        }
    }
    file << "\n";
    file.close();
}