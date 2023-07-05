//compile with: g++ -o fit_brewster fit_brewster.cpp `root-config --cflags --glibs`
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TGraphErrors.h"
#include "TPaveText.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include "TFitResult.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include "../include/reader.h"
#include "../include/utils.h"
#include "../include/fitting.h"
#include "../include/writer.h"

#define BFOMTY_iters 1000

double max_parabola(double *x, double *par)
{
    return par[0]*x[0]*x[0] + par[1]*x[0] + par[2];
}

int main(int argc, char const *argv[])
{
    std::vector<std::vector<double> *> data_max1; //contains: theta_e,V,err_V
    readCSVFile("../data/brewster/trasmissione/max_1_elab.csv", data_max1);
    std::vector<double> theta_e_max1 = *data_max1[0];
    std::vector<double> V_max1 = *data_max1[1];
    std::vector<double> err_V_max1 = *data_max1[2];

    std::vector<std::vector<double> *> data_max2; //contains: theta_e,V,err_V
    readCSVFile("../data/brewster/trasmissione/max_2_elab.csv", data_max2);
    std::vector<double> theta_e_max2 = *data_max2[0];
    std::vector<double> V_max2 = *data_max2[1];
    std::vector<double> err_V_max2 = *data_max2[2];

    //error vector for theta_i
    std::vector<double> err_theta_e_max1 = std::vector<double>(theta_e_max1.size(), 0.5);
    std::vector<double> err_theta_e_max2 = std::vector<double>(theta_e_max2.size(), 0.5);

    //fitting
    TF1 *fit_max1 = new TF1("fit_max1", max_parabola, 0, 70, 3); //parameters of TF1: name, function, xmin, xmax, npar
    TGraphErrors *graph_max1 = new TGraphErrors(theta_e_max1.size(), &theta_e_max1[0], &V_max1[0], &err_theta_e_max1[0], &err_V_max1[0]);
    std::vector<double> fit_max1_pars = {-0.01, 1, 3.0};
    TFitResultPtr fit_max1_res = BFOMTY_fixed_complete(graph_max1, fit_max1, BFOMTY_iters, fit_max1_pars);

    TF1 *fit_max2 = new TF1("fit_max2", max_parabola, 0, 70, 3); //parameters of TF1: name, function, xmin, xmax, npar
    TGraphErrors *graph_max2 = new TGraphErrors(theta_e_max2.size(), &theta_e_max2[0], &V_max2[0], &err_theta_e_max2[0], &err_V_max2[0]);
    std::vector<double> fit_max2_pars = {-0.01, 1, 2.8};
    TFitResultPtr fit_max2_res = BFOMTY_fixed_complete(graph_max2, fit_max2, BFOMTY_iters, fit_max2_pars);

    //plotting (use multigraph)
    TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000, 1000, 700);
    TMultiGraph *mg = new TMultiGraph();
    graph_max1->SetMarkerStyle(20);
    graph_max1->SetMarkerSize(0.5);
    graph_max1->SetMarkerColor(kRed);
    graph_max1->SetTitle("Massimo 1");
    graph_max1->GetXaxis()->SetTitle("#theta_{i} [Gradi]");
    graph_max1->GetYaxis()->SetTitle("Tensione [V]");
    graph_max1->Draw("AP");
    fit_max1->SetLineColor(kRed);
    graph_max1->GetFunction("fit_max1")->SetLineColor(kRed);
    mg->Add(graph_max1);

    graph_max2->SetMarkerStyle(20);
    graph_max2->SetMarkerSize(0.5);
    graph_max2->SetMarkerColor(kBlue);
    graph_max2->SetTitle("Massimo 2");
    graph_max2->GetXaxis()->SetTitle("#theta_{i} [Gradi]");
    graph_max2->GetYaxis()->SetTitle("Tensione [V]");
    graph_max2->Draw("AP");
    fit_max2->SetLineColor(kBlue);
    graph_max2->GetFunction("fit_max2")->SetLineColor(kBlue);
    //the fit of max2 should be drawn in blue
    mg->Add(graph_max2);

    mg->Draw("AP");
    mg->GetXaxis()->SetTitle("#theta_{i} [Gradi]");
    mg->GetYaxis()->SetTitle("Tensione [V]");
    mg->SetTitle("Massimi");
    mg->Draw("AP");

    //add TpaveText
    TPaveText *pt = new TPaveText(0.12, 0.7, 0.35, 0.9, "brNDC");
    pt->SetBorderSize(1);
    pt->SetTextAlign(12);
    pt->SetTextSize(0.03);
    //pt should be semi-transparent
    pt->SetFillColorAlpha(0, 0.4); // 0 is the color code for black, 0.4 for 60% transparency
    pt->SetTextFont(42);
    //calculate vertixes of the two parabolas
    double x1 = -fit_max1->GetParameter(1) / (2 * fit_max1->GetParameter(0));
    double x2 = -fit_max2->GetParameter(1) / (2 * fit_max2->GetParameter(0));
    //error on x1 and x2
    TVectorD D1(3);
    D1[1] = -1 / (2 * fit_max1->GetParameter(0));
    D1[0] = (fit_max1->GetParameter(1) / (2 * pow(fit_max1->GetParameter(0), 2)));
    D1[2] = 0.0;

    TVectorD D2(3);
    D2[1] = -1 / (2 * fit_max2->GetParameter(0));
    D2[0] = (fit_max2->GetParameter(1) / (2 * pow(fit_max2->GetParameter(0), 2)));
    D2[2] = 0.0;
    //double err_x1 = sqrt(pow(fit_max1->GetParError(1) / (2 * fit_max1->GetParameter(0)), 2) + pow((fit_max1->GetParameter(1) * fit_max1->GetParError(0) / (2 * pow(fit_max1->GetParameter(0), 2))),2));
    //double err_x2 = sqrt(pow(fit_max2->GetParError(1) / (2 * fit_max2->GetParameter(0)), 2) + pow((fit_max2->GetParameter(1) * fit_max2->GetParError(0) / (2 * pow(fit_max2->GetParameter(0), 2))),2));
    //add covariances
    //get covariance matrixes
    TMatrixD cov1 = fit_max1_res->GetCovarianceMatrix();
    TMatrixD cov2 = fit_max2_res->GetCovarianceMatrix();
    //print covariances matrixes
    cov1.Print();
    D1.Print();
    cov2.Print();
    D2.Print();
    //err_x1 += 2 *(-fit_max1->GetParameter(1) * cov1(0, 1) / (4 * pow(fit_max1->GetParameter(0), 3)));
    //err_x2 += 2 *(-fit_max2->GetParameter(1) * cov2(0, 1) / (4 * pow(fit_max2->GetParameter(0), 3)));
    TVectorD t_D1_C = cov1 * D1;
    TVectorD t_D2_C = cov2 * D2;
    double err_x1 = t_D1_C * D1;
    double err_x2 = t_D2_C * D2;

    std::cout << "err_x1: " << err_x1 << std::endl;
    std::cout << "err_x2: " << err_x2 << std::endl;

    pt->AddText(Form("#theta_{M1} = (%.2f #pm %.2f)", x1, err_x1));
    pt->AddText(Form("#theta_{M2} = (%.2f #pm %.2f)", x2, err_x2));
    //add the two chi2/ndf and p-values
    pt->AddText(Form("M1 #chi^{2}/ndf: %.2f/%d", fit_max1->GetChisquare(), fit_max1->GetNDF()));
    pt->AddText(Form("M1 p-value: %.2f", fit_max1->GetProb()));
    pt->AddText(Form("M2 #chi^{2}/ndf: %.2f/%d", fit_max2->GetChisquare(), fit_max2->GetNDF()));
    pt->AddText(Form("M2 p-value: %.2f", fit_max2->GetProb()));
    pt->Draw();

    c1->SaveAs("brewster_massimi.png");

    return 0;
}
