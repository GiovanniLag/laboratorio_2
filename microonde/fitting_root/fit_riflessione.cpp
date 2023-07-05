//compile with: g++ -o fit_riflessione fit_riflessione.cpp `root-config --cflags --glibs`
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

double func_riflessione(double *x, double *par)
{
    return par[0]*x[0];
}

int main(int argc, char const *argv[])
{
    std::vector<std::vector<double> *> data; //contains: theta_i,theta_r
    readCSVFile("../data/riflessione/riflessione_elab.csv", data);
    std::vector<double> theta_i = *data[0];
    std::vector<double> theta_r = *data[1];
    std::vector<double> theta_err = std::vector<double>(theta_i.size(), 0.5); //use for both theta_i and theta_r

    //fitting
    TF1 *fit = new TF1("fit", func_riflessione, 0, 90, 1); //parameters of TF1: name, function, xmin, xmax, npar
    TGraphErrors *graph = new TGraphErrors(theta_i.size(), &theta_i[0], &theta_r[0], &theta_err[0], &theta_err[0]);

    std::vector<double> fit_par = {1};
    fit = BFOMTY_fixed(graph, fit, BFOMTY_iters, fit_par);
    //print fit parameters
    std::cout << "Fit Results:" << std::endl;
    std::cout << "m = " << fit->GetParameter(0) << " +- " << fit->GetParError(0) << std::endl;

    //plot
    TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000, 1000, 700);
    graph->SetTitle("Angolo di riflessione in funzione dell'angolo di incidenza");
    graph->GetXaxis()->SetTitle("Angolo di incidenza [gradi]");
    graph->GetYaxis()->SetTitle("Angolo di riflessione [gradi]");
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.5);
    graph->SetMarkerColor(kBlue);
    graph->Draw("AP");
    fit->Draw("same");

    //add TpaveText
    TPaveText *pt = new TPaveText(0.6, 0.15, 0.9, 0.35, "brNDC");
    pt->SetBorderSize(1);
    pt->SetTextAlign(12);
    pt->SetTextSize(0.03);
    pt->SetFillColor(0);
    pt->SetTextFont(42);
    pt->AddText(Form("m = %.3f #pm %.3f", fit->GetParameter(0), fit->GetParError(0)));
    pt->AddText(Form("#chi^{2}/ndf = %.3f/%i = %.3f", fit->GetChisquare(), fit->GetNDF(), fit->GetChisquare() / fit->GetNDF()));
    pt->AddText(Form("p-value = %.3f", fit->GetProb()));
    pt->Draw();

    c1->SaveAs("riflessione.png");
}
