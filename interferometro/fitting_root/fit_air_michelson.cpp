//compile with: g++ -o fit_air_michelson fit_air_michelson.cpp `root-config --cflags --glibs`
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TGraphErrors.h"
#include "TPaveText.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include "TFitResultPtr.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include "../include/reader.h"
#include "../include/utils.h"
#include "../include/fitting.h"
#include "../include/writer.h"

#define BFOMTY_iters 100
using namespace std;

double fitFunc(double *x, double *par)
{
    double delta_N = x[0];
    double delta_P = par[0] + par[1] * delta_N;
    return delta_P;
}

int main(int argc, char const *argv[])
{
    // Read data
    vector<vector<double>*> data;
    readCSVFile("../data/michelson/air_fit_data.csv", data);
    vector<double> delta_N = *data[0]; //x
    vector<double> delta_P = *data[1]; //y
    vector<double> delta_P_err = *data[2]; //y_err
    //delta N error is 0.5 for all points
    vector<double> delta_N_err = vector<double>(delta_N.size(), 0.5);

    // Fit
    vector<double> fit_initial_par = {0.0, 1.5};
    TF1 *fit_function = new TF1("fit_function", fitFunc, 0, 16, fit_initial_par.size());
    TGraphErrors *data_graph = new TGraphErrors(delta_N.size(), &delta_N[0], &delta_P[0], &delta_N_err[0], &delta_P_err[0]);
    TFitResultPtr fit_result = BFOMTY_fixed_complete(data_graph, fit_function, BFOMTY_iters, fit_initial_par);


    // Print results
    cout << "Fit results:" << endl;
    cout << "Chi2: " << fit_result->Chi2() << endl;
    cout << "NDF: " << fit_result->Ndf() << endl;
    cout << "Chi2/NDF: " << fit_result->Chi2() / fit_result->Ndf() << endl;
    cout << "P-value: " << fit_result->Prob() << endl;
    cout << "Fit parameters: " << endl;
    for (int i = 0; i < fit_initial_par.size(); i++)
    {
        cout << "par[" << i << "] = " << fit_result->Parameter(i) << " +/- " << fit_result->ParError(i) << endl;
    }
    cout << "Covariance matrix: " << endl;
    for (int i = 0; i < fit_initial_par.size(); i++)
    {
        for (int j = 0; j < fit_initial_par.size(); j++)
        {
            cout << fit_result->CovMatrix(i, j) << " ";
        }
        cout << endl;
    }

    // Plot
    TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000, 1000, 700);
    c1->SetGrid();
    data_graph->SetTitle("#Delta P vs #Delta N");
    data_graph->GetXaxis()->SetTitle("#Delta N");
    data_graph->GetYaxis()->SetTitle("#Delta P");
    data_graph->SetMarkerStyle(20);
    data_graph->SetMarkerSize(0.5);
    data_graph->Draw("AP");
    fit_function->Draw("same");
    //add TPaveText
    TPaveText *pt = new TPaveText(0.1, 0.7, 0.35, 0.9, "brNDC");
    pt->SetBorderSize(1);
    pt->SetTextAlign(12);
    pt->SetTextSize(0.03);
    pt->SetFillColor(0);
    pt->SetTextFont(42);
    pt->AddText(Form("A = (%.3f #pm %.3f) cmHg", fit_function->GetParameter(0), fit_function->GetParError(0)));
    pt->AddText(Form("B = (%.3f #pm %.3f) cmHg", fit_function->GetParameter(1), fit_function->GetParError(1)));
    pt->AddText(Form("#chi^{2}/ndf =%.3f/%i =%.3f", fit_function->GetChisquare(), fit_function->GetNDF(), fit_function->GetChisquare() / fit_function->GetNDF()));
    pt->AddText(Form("p-value = %.3f", fit_function->GetProb()));
    pt->Draw();

    c1->SaveAs("fit_aria_michelson.png");

    return 0;
}
