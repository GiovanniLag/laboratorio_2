/*Fits (linerized) data from spectral analisis of light from a mercury lamp to find cauchy parameters of prims*/
//compile with: g++ -o fit_parametri_prisma fit_parametri_prisma.cpp `root-config --cflags --glibs`
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

#define BFOMTY_iters 1000

double cauchy(double *x, double *par)
{
    return par[0] + par[1]*x[0];
}

int main(int argc, char const *argv[])
{
        std::vector<std::vector<double>*> data; //contains: xis,gamma[1/m^2],gamma_err[1/m^2]
        readCSVFile("../data/fit_cauchy_data.csv", data);
        std::vector<double> xis = *data[0];
        std::vector<double> gamma = *data[1];
        std::vector<double> gamma_err = *data[2];
        
        //fitting
        std::vector<double> fit_params = {1, 1};
        TF1 *fit_function = new TF1("fit_function", cauchy, 0, 90, 2);
        TGraphErrors *fit_graph = new TGraphErrors(xis.size(), &xis[0], &gamma[0], 0, &gamma_err[0]);

        TFitResultPtr fit_result = BFOMTY_fixed_complete(fit_graph, fit_function, BFOMTY_iters, fit_params);
        //print fit results
        std::cout << "Fit results:" << std::endl;
        std::cout << "A = " << fit_function->GetParameter(0) << " +- " << fit_function->GetParError(0) << std::endl;
        std::cout << "B = " << fit_function->GetParameter(1) << " +- " << fit_function->GetParError(1) << std::endl;
        //print covariance matrix
        std::cout << "Covariance matrix:" << std::endl;
        std::cout << fit_result->GetCovarianceMatrix()[0][0] << " " << fit_result->GetCovarianceMatrix()[0][1] << std::endl;
        std::cout << fit_result->GetCovarianceMatrix()[1][0] << " " << fit_result->GetCovarianceMatrix()[1][1] << std::endl;
        //print chi2/ndf and p-value
        std::cout << "Chi2/ndf = " << fit_function->GetChisquare() / fit_function->GetNDF() << std::endl;
        std::cout << "p-value = " << fit_function->GetProb() << std::endl;

        //plotting
        TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000, 1000, 700);
        c1->SetGrid();
        fit_graph->SetTitle("n in funzione di 1/#lambda^2");
        fit_graph->GetXaxis()->SetTitle("1/#lambda^2 [1/m^2]");
        fit_graph->GetYaxis()->SetTitle("n");
        fit_graph->SetMarkerStyle(20);
        fit_graph->SetMarkerSize(0.5);
        fit_graph->Draw("AP");
        fit_function->Draw("same");
        //add TPaveText
        TPaveText *pt = new TPaveText(0.1, 0.7, 0.35, 0.9, "brNDC");
        pt->SetBorderSize(1);
        pt->SetTextAlign(12);
        pt->SetTextSize(0.03);
        pt->SetFillColor(0);
        pt->SetTextFont(42);
        //write parameter with error and chi2/ndf (with result) and p-value
        pt->AddText(Form("A = %.3f #pm %.3f", fit_function->GetParameter(0), fit_function->GetParError(0)));
        pt->AddText(Form("B = %.3fe-15 #pm %.3fe-15", fit_function->GetParameter(1)*1e15, fit_function->GetParError(1)*1e15));
        pt->AddText(Form("#chi^{2}/ndf =%.3f/%i =%.3f", fit_function->GetChisquare(), fit_function->GetNDF(), fit_function->GetChisquare() / fit_function->GetNDF()));
        pt->AddText(Form("p-value = %.3f", fit_function->GetProb()));
        pt->Draw();
        c1->SaveAs("fit_parametri_prisma.png");

    return 0;
}

