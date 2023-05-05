#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

void readCSVFile(const std::string& filename, std::vector<std::vector<double>*>& data)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file " << filename << std::endl;
        return;
    }

    std::vector<std::vector<double>> temp_data;

    // Skip the first line
    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<double> row;
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            try {
                row.push_back(std::stod(cell));
            }
            catch (const std::invalid_argument&) {
                std::cerr << "Invalid value: " << cell << std::endl;
            }
        }
        temp_data.push_back(row);
    }

    // Transpose the temporary data to get the columns
    const size_t num_cols = temp_data[0].size();
    const size_t num_rows = temp_data.size();
    for (size_t i = 0; i < num_cols; ++i) {
        auto* col = new std::vector<double>(num_rows);
        for (size_t j = 0; j < num_rows; ++j) {
            (*col)[j] = temp_data[j][i];
        }
        data.push_back(col);
    }
}