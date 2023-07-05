//compile with: g++ -o fit_cos_fp fit_cos_fp.cpp `root-config --cflags --glibs`
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
    //a*x + b
    double cos_theta = x[0];
    double N = par[0] + par[1] * cos_theta;
    return N;
}

int main(int argc, char const *argv[])
{
    // Read data
    vector<vector<double>*> data;
    readCSVFile("../data/fabry-perot/cos_fit.csv", data);
    vector<double> cos_theta = *data[0]; //x
    vector<double> err_cos_theta = *data[1]; //y
    vector<double> N = *data[2]; //y_err
    
    // Fit (use normal ROOT fit)
    vector<double> fit_initial_par = {1, 5e-5};
    TF1 *fit_function = new TF1("fit_function", fitFunc, 0, 3, fit_initial_par.size());
    TGraphErrors *data_graph = new TGraphErrors(cos_theta.size(), &N[0], &cos_theta[0], 0, &err_cos_theta[0]);
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
    data_graph->SetTitle("cos(#theta) vs N");
    data_graph->GetXaxis()->SetTitle("N");
    data_graph->GetYaxis()->SetTitle("cos(#theta)");
    data_graph->Draw("AP");
    fit_function->SetLineColor(kRed);
    fit_function->Draw("same");
    TPaveText *pt = new TPaveText(0.58, 0.12, 0.9, 0.32, "NDC");
    pt->SetBorderSize(1);
    pt->SetTextAlign(12);
    pt->SetTextSize(0.03);
    pt->SetFillColor(0);
    pt->SetTextFont(42);
    pt->AddText(Form("#chi^{2}/NDF= %.2f/ %i = %.2f", fit_result->Chi2(), fit_result->Ndf(), fit_result->Chi2() / fit_result->Ndf()));
    pt->AddText(Form("P-value = %.2f", fit_result->Prob()));
    pt->AddText(Form("N = a + b*cos(#theta)"));
    pt->AddText(Form("a = %.6f #pm %.6f", fit_result->Parameter(0), fit_result->ParError(0)));
    pt->AddText(Form("b = (%.3f #pm %.3f) #upoint 10^{-5}", fit_result->Parameter(1)*1e5, fit_result->ParError(1)*1e5));
    pt->Draw();

    // Save plot
    c1->SaveAs("cos_fit.png");

}
