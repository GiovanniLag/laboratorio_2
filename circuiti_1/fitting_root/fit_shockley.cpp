//compile with: g++ -o fit_shockley fit_shockley.cpp `root-config --cflags --glibs`
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

using namespace std;

double fitFunc(double *x, double *par)
{
    //Shockley equation: I = I0 * (exp(qV/(k*g*T)) - 1)
    double V = x[0];
    double I = par[0] * (exp(par[1]*V/(par[2])) - 1);
    return I;
}

int main(int argc, char const *argv[])
{
    // Read data
    vector<vector<double>*> data;
    readCSVFile("../data/parte_tre/fit_diodo.csv", data); //V,I,sigma_V,sigma_I
    vector<double> V = *data[0]; //x
    vector<double> err_V = *data[2]; //x_err
    vector<double> I = *data[1]; //y
    vector<double> err_I = *data[3]; //y_err

    // Fit (use normal ROOT fit)
    vector<double> fit_initial_par = {3.1641995673263943e-10,1.5839110396286393,300};
    TF1 *fit_function = new TF1("fit_function", fitFunc, 0, 3, fit_initial_par.size());
    //set initial parameters
    for (int i = 0; i < fit_initial_par.size(); i++)
    {
        fit_function->SetParameter(i, fit_initial_par[i]);
    }
    TGraphErrors *data_graph = new TGraphErrors(V.size(), &V[0], &I[0], &err_V[0], &err_I[0]);
    TFitResultPtr fit_result = data_graph->Fit(fit_function, "S");

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
    TCanvas *c1 = new TCanvas("c1", "c1", 1200, 1000);
    c1->SetLeftMargin(0.15);
    c1->SetFillColor(0);
    c1->SetGrid();
    data_graph->SetMarkerStyle(20);
    data_graph->SetMarkerSize(1);
    data_graph->SetMarkerColor(kBlue);
    //title \cos(\theta) vs N [use latex syntax]
    data_graph->SetTitle("Tensione vs Corrente");
    data_graph->GetXaxis()->SetTitle("Tensione [V]");
    data_graph->GetYaxis()->SetTitle("Corrente [A]");
    data_graph->Draw("AP");
    fit_function->SetLineColor(kRed);
    fit_function->Draw("same");
    TPaveText *pt = new TPaveText(0.17, 0.6, 0.5, 0.9, "NDC"); // Change coordinates to move to the right and make larger
    pt->SetTextSize(0.025); // Set a smaller text size to create more line spacing
    pt->SetBorderSize(1);
    pt->SetTextAlign(12);
    pt->SetTextSize(0.03);
    pt->SetFillColor(0);
    pt->SetTextFont(42);
    pt->AddText(Form("#chi^{2}/NDF= %.2f/ %i = %.2f", fit_result->Chi2(), fit_result->Ndf(), fit_result->Chi2() / fit_result->Ndf()));
    pt->AddText(Form("P-value = %.4f", fit_result->Prob()));
    pt->AddText(Form("I = I_{0} * (e^{q*V/k*g*T} - 1)"));
    pt->AddText(Form("I_{0} = (%.3e #pm %.3e) A", fit_result->Parameter(0), fit_result->ParError(0)));
    pt->AddText(Form("g = %.3f #pm %.3f", fit_result->Parameter(1), fit_result->ParError(1)));
    pt->AddText(Form("T = %.3f #pm %.3f K", fit_result->Parameter(2), fit_result->ParError(2)));
    pt->Draw();
    c1->SaveAs("fit_shockley.png");
}
