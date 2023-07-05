//compile with: g++ -o fit_legge_di_malus fit_legge_di_malus.cpp `root-config --cflags --glibs`
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

double legge_di_malus(double *x, double *par)
{
    return par[0] * pow(cos(x[0]), 2) + par[1] * TMath::Abs(cos(x[0]));
}

int main(int argc, char const *argv[])
{
    std::vector<std::vector<double> *> data; //contains: alpha,alpha_err,I,V_err
    readCSVFile("../data/polarizzazione/polarizzazione_alpha.csv", data);
    //angle must be converted from degrees to radians
    std::vector<double> alpha = *data[0];
    for (int i = 0; i < alpha.size(); i++)
    {
        alpha[i] = alpha[i] * M_PI / 180;
    }
    double alpha_err_single = 0.5 * M_PI / 180;
    std::vector<double> alpha_err = std::vector<double>(alpha.size(), alpha_err_single);
    std::vector<double> V = *data[1];
    std::vector<double> V_err = *data[2];

    //fitting
    TF1 *fit = new TF1("fit", legge_di_malus, 0, 90, 2); //parameters of TF1: name, function, xmin, xmax, npar
    TGraphErrors *graph = new TGraphErrors(alpha.size(), &alpha[0], &V[0], &alpha_err[0], &V_err[0]);

    std::vector<double> fit_par = {1.2, 1.2};
    fit = BFOMTY_fixed(graph, fit, BFOMTY_iters, fit_par);
    //print fit parameters
    std::cout << "Fit Results:" << std::endl;
    std::cout << "p0 = " << fit->GetParameter(0) << " +- " << fit->GetParError(0) << std::endl;
    std::cout << "p1 = " << fit->GetParameter(1) << " +- " << fit->GetParError(1) << std::endl;

    //plot
    TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000, 1000, 700);
    graph->SetTitle("Tensione in funzione dell'angolo");
    graph->GetXaxis()->SetTitle("Angolo [gradi]");
    graph->GetYaxis()->SetTitle("Tensione' [V]");
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
    pt->AddText(Form("#V_0 = %.3f #pm %.3f", fit->GetParameter(0), fit->GetParError(0)));
    pt->AddText(Form("#p_1 = %.3f #pm %.3f", fit->GetParameter(1), fit->GetParError(1)));
    pt->AddText(Form("#chi^{2}/ndf = %.3f/%i = %.3f", fit->GetChisquare(), fit->GetNDF(), fit->GetChisquare() / fit->GetNDF()));
    pt->AddText(Form("p-value = %.3f", fit->GetProb()));
    pt->Draw();
    c1->SaveAs("legge_di_malus.png");
}