//compile with: g++ -o fit_onde_stazionarie_theta fit_onde_stazionarie_theta.cpp `root-config --cflags --glibs`
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TGraphErrors.h"
#include "TPaveText.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include "../include/reader.h"
#include "../include/utils.h"
#include "../include/fitting.h"
#include "../include/writer.h"

#define BFOMTY_iters 1000

double fit_formula(double *x, double *par) // Gaussian function
{
    double mu = 0.0;  // mean
    double A = par[0];  // amplitude
    double sigma = par[1];  // standard deviation
    return A * exp(-1.0 * pow((x[0] - mu), 2) / (2 * sigma * sigma));
}

int main(int argc, char const *argv[])
{
    std::vector<std::vector<double>*> data; //contains: theta,V,err_V
    readCSVFile("../data/onde_stazionarie/ondestazionarie_theta.csv", data);
    std::vector<double> thetas = *data[0];
    std::vector<double> V = *data[1];
    std::vector<double> V_err = *data[2];
    std::vector<double> theta_errs(thetas.size(), 0.5);  // error on theta is 0.5 for all

    //fitting
    TF1 *fit = new TF1("fit", fit_formula, 0, 50, 2); //parameters of TF1: name, function, xmin, xmax, npar
    TGraphErrors *graph = new TGraphErrors(thetas.size(), &thetas[0], &V[0], &theta_errs[0], &V_err[0]);

    std::vector<double> fit_par = {1.2, 1.2};
    fit = BFOMTY_fixed(graph, fit, BFOMTY_iters, fit_par);
    //print fit parameters
    std::cout << "Fit Results:" << std::endl;
    std::cout << "A = " << fit->GetParameter(0) << " +- " << fit->GetParError(0) << std::endl;
    std::cout << "sigma = " << fit->GetParameter(1) << " +- " << fit->GetParError(1) << std::endl;
    //plot
    TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000, 1000, 700);
    graph->SetTitle("Tensione in funzione di theta");
    graph->GetXaxis()->SetTitle("#theta [gradi]");
    graph->GetYaxis()->SetTitle("Tensione [V]");
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.5);
    graph->Draw("AP");
    fit->Draw("same");
    //add TPaveText
    TPaveText *pt = new TPaveText(0.6, 0.7, 0.9, 0.9, "brNDC");
    pt->SetBorderSize(1);
    pt->SetTextAlign(12);
    pt->SetTextSize(0.03);
    pt->SetFillColor(0);
    pt->SetTextFont(42);
    //write parameter with error and chi2/ndf (with result) and p-value
    pt->AddText(Form("A = %.3f #pm %.3f", fit->GetParameter(0), fit->GetParError(0)));
    pt->AddText(Form("#sigma = %.3f #pm %.3f", fit->GetParameter(1), fit->GetParError(1)));
    pt->AddText(Form("#chi^{2}/ndf= %.3f / %i = %.3f", fit->GetChisquare(), fit->GetNDF(), fit->GetChisquare() / fit->GetNDF()));
    pt->AddText(Form("p-value = %.3f", fit->GetProb()));
    pt->Draw();
    c1->SaveAs("onde_stazionarie_theta.png");

    return 0;
}
