//compile with: g++ -o fit_onde_stazionarie_r fit_onde_stazionarie_r.cpp `root-config --cflags --glibs`
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

double fit_formula(double *x, double *par) //V_0/(r^2)
{
    return par[0] / (x[0]*x[0]) + par[1]/x[0];
}

int main(int argc, char const *argv[])
{
    std::vector<std::vector<double>*> data; //contains: rs_max,rs_max_err,V_max,V_max_err
    readCSVFile("../data/onde_stazionarie/ondestazionarie_r_massimi_elab.csv", data);
    std::vector<double> rs_max = *data[0];
    std::vector<double> rs_max_err = *data[1];
    std::vector<double> V_max = *data[2];
    std::vector<double> V_max_err = *data[3];

    //fitting
    TF1 *fit = new TF1("fit", fit_formula, 0.45, 0.75, 2); //parameters of TF1: name, function, xmin, xmax, npar
    TGraphErrors *graph = new TGraphErrors(rs_max.size(), &rs_max[0], &V_max[0], &rs_max_err[0], &V_max_err[0]);

    std::vector<double> fit_par = {1.2, 1.2};
    fit = BFOMTY_fixed(graph, fit, BFOMTY_iters, fit_par);
    //print fit parameters
    std::cout << "Fit Results:" << std::endl;
    std::cout << "p_0 = " << fit->GetParameter(0) << " +- " << fit->GetParError(0) << std::endl;
    std::cout << "p_1 = " << fit->GetParameter(1) << " +- " << fit->GetParError(1) << std::endl;
    //plot
    TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000, 1000, 700);
    graph->SetTitle("Tensione di massima oscillazione in funzione del raggio");
    graph->GetXaxis()->SetTitle("Raggio [m]");
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
    pt->AddText(Form("#p_0 = %.3f #pm %.3f", fit->GetParameter(0), fit->GetParError(0)));
    pt->AddText(Form("#p_1 = %.3f #pm %.3f", fit->GetParameter(1), fit->GetParError(1)));
    pt->AddText(Form("#chi^{2}/ndf= %.3f / %i = %.3f", fit->GetChisquare(), fit->GetNDF(), fit->GetChisquare() / fit->GetNDF()));
    pt->AddText(Form("p-value = %.3f", fit->GetProb()));
    pt->Draw();
    c1->SaveAs("onde_stazionarie_r.png");


    return 0;
}
