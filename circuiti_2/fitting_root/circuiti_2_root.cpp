//command to compile: g++ circuiti_2_root.cpp -o circuiti_2_root `root-config --cflags --glibs`

//include cern Root libraries for function analysis
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TgraphErrors.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char const *argv[])
{
    //read data from csv file
    ifstream file("data/RL/F0000CH2.csv");
    vector<string> lines;
    //read from 19th line to end of file and store in lines
    for (int i = 0; i < 18; i++)
    {
        string line;
        getline(file, line);
    }
    while (!file.eof())
    {
        string line;
        getline(file, line);
        if (!line.empty())
            lines.push_back(line);
    }
    file.close();
    //create vectors for t and V values
    vector<double> t;
    vector<double> V;

    //fill vectors with data from csv file
    cout << "lines.size() = " << lines.size() << endl;
    for (int i = 0; i < lines.size(); i++)
    {
        //note that each line starts with three commas that need to be skipped
        string line = lines[i];
        //skip the commas
        line = line.substr(3);
        int pos = line.find(",");
        string t_str = line.substr(0, pos);
        string V_str = line.substr(pos + 1);
        try {
            double t_val = stod(t_str);
            double V_val = stod(V_str);
            t.push_back(t_val);
            V.push_back(V_val);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error parsing value: " << e.what() << std::endl;
            cout << "t_str = " << t_str << endl;
            cout << "V_str = " << V_str << endl;
        }
    }

    //print first 5 values of t and V vectors
    for (int i = 0; i < 5; i++)
    {
        cout << t[i] << " " << V[i] << endl;
    }
    
    //graph V(t) in function of t, without lines connecting the points
    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    TGraph *g1 = new TGraph(t.size(), &t[0], &V[0]);
    //set point color to red
    g1->SetMarkerColor(2);
    g1->SetTitle("V(t) in function of t");
    g1->GetXaxis()->SetTitle("t [s]");
    g1->GetYaxis()->SetTitle("V [V]");
    g1->Draw("APL");
    c1->SaveAs("test.png");

    
}
