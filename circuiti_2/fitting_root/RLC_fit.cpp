//compile with: g++ -o RLC_fit RLC_fit.cpp `root-config --cflags --glibs`

#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TgraphErrors.h"
#include "TPaveText.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "../include/reader.h"
#include "../include/fitting.h"
#include "../include/writer.h"

#define fitIters 500

using namespace std;

int main(int argc, char const *argv[])
{
    //read data from csv file
    //sottosmorzato
    vector<vector<double>*> sottosmorzato_data;
    readCSVFile("../data/RLC/sottosmorzato/sottosmorzato_clustered.csv", sottosmorzato_data);
    vector<double> t_sottosmorzato = *sottosmorzato_data[0];
    vector<double> V_sottosmorzato = *sottosmorzato_data[1];
    vector<double> t_sottosmorzato_err = *sottosmorzato_data[2];
    vector<double> V_sottosmorzato_err = *sottosmorzato_data[3];

    //criticamente smorzato
    vector<vector<double>*> crit_smorzato_data;
    readCSVFile("../data/RLC/smorzamentocritico/crit_smorzato_clustered.csv", crit_smorzato_data);
    vector<double> t_crit_smorzato = *crit_smorzato_data[0];
    vector<double> V_crit_smorzato = *crit_smorzato_data[1];
    vector<double> t_crit_smorzato_err = *crit_smorzato_data[2];
    vector<double> V_crit_smorzato_err = *crit_smorzato_data[3];
    
    //create graphs with error bars
    vector<double> intial_params_sottosmorzato = {0.00000395, 4554, 43784};
    TGraphErrors *sottosmorzato_grph = new TGraphErrors(t_sottosmorzato.size(), &t_sottosmorzato[0], &V_sottosmorzato[0], &t_sottosmorzato_err[0], &V_sottosmorzato_err[0]);
    sottosmorzato_grph->SetTitle("Sottosmorzato");

    vector<double> intial_params_crit_smorzato = {7.0e-5, 20000};
    TGraphErrors *crit_smorzato_grph = new TGraphErrors(t_crit_smorzato.size(), &t_crit_smorzato[0], &V_crit_smorzato[0], &t_crit_smorzato_err[0], &V_crit_smorzato_err[0]);
    crit_smorzato_grph->SetTitle("Criticamente smorzato");


    //fit graphs
    TF1 *sottosmorzato_fit = new TF1("sottosmorzato_fit", "[0] * exp(-[1]*x) * (([2]*sin([2]*x)))", 0, t_sottosmorzato.at(t_sottosmorzato.size()-1)); //[0] = I_0, [1] = gamma, [2] = beta

    std::cout << "Fit sottosmorzato" << std::endl;
    sottosmorzato_fit = BFOMTY_fixed(sottosmorzato_grph, sottosmorzato_fit, fitIters, intial_params_sottosmorzato, 0.1);
    std::cout << "I_0 = " << sottosmorzato_fit->GetParameter(0) << " +- " << sottosmorzato_fit->GetParError(0) << std::endl;
    std::cout << "gamma = " << sottosmorzato_fit->GetParameter(1) << " +- " << sottosmorzato_fit->GetParError(1) << std::endl;
    std::cout << "beta = " << sottosmorzato_fit->GetParameter(2) << " +- " << sottosmorzato_fit->GetParError(2) << std::endl;
    std::vector<std::string> colums = {"I_0", "err-I_0", "gamma", "err-gamma", "beta", "err-beta"};
    std::vector<double> values = {sottosmorzato_fit->GetParameter(0), sottosmorzato_fit->GetParError(0), sottosmorzato_fit->GetParameter(1), sottosmorzato_fit->GetParError(1), sottosmorzato_fit->GetParameter(2), sottosmorzato_fit->GetParError(2)};
    //writeCSV(values, colums, "../data/RLC/sottosmorzato/sottosmorzato_fit.csv");


    TF1 *crit_smorzato_fit = new TF1("crit_smorzato_fit", "[0] * [1]*[1] * x * exp(-[1]*x)", 0, t_crit_smorzato.at(t_crit_smorzato.size()-1)); //[0] = xi, [1] = gamma

    std::cout << "Fit crit_smorzato" << std::endl;
    crit_smorzato_fit = BFOMTY_fixed(crit_smorzato_grph, crit_smorzato_fit, fitIters, intial_params_crit_smorzato, 0.1);
    std::cout << "xi = " << crit_smorzato_fit->GetParameter(0) << " +- " << crit_smorzato_fit->GetParError(0) << std::endl;
    std::cout << "gamma = " << crit_smorzato_fit->GetParameter(1) << " +- " << crit_smorzato_fit->GetParError(1) << std::endl;
    std::vector<std::string> colums2 = {"xi", "err-xi", "gamma", "err-gamma"};
    std::vector<double> values2 = {crit_smorzato_fit->GetParameter(0), crit_smorzato_fit->GetParError(0), crit_smorzato_fit->GetParameter(1), crit_smorzato_fit->GetParError(1)};
    writeCSV(values2, colums2, "../data/RLC/crit_smorzato/crit_smorzato_fit.csv");

    //create canvases
    TCanvas *c_sottosmorzato = new TCanvas("c1", "c1", 800, 600);
    TCanvas *c_crit_smorzato = new TCanvas("c2", "c2", 800, 600);

    //draw graphs with fit info
    //red for charge -> in c_charge canvas
    c_sottosmorzato->cd();
    sottosmorzato_grph->SetMarkerColor(2);
    sottosmorzato_grph->SetMarkerStyle(20);
    sottosmorzato_grph->SetMarkerSize(0.5);
    sottosmorzato_grph->GetXaxis()->SetTitle("t [s]");
    sottosmorzato_grph->GetYaxis()->SetTitle("V [V]");
    sottosmorzato_grph->Draw("AP");
    sottosmorzato_fit->Draw("same");
    //create TPaveText to display fit result information
    TPaveText *fitInfo_sottosmorzato = new TPaveText(0.58, 0.58, 0.88, 0.88, "NDC");
    fitInfo_sottosmorzato->SetFillColor(0);
    fitInfo_sottosmorzato->SetTextAlign(12);
    fitInfo_sottosmorzato->AddText(Form("Risultati Fit:"));
    fitInfo_sottosmorzato->AddText(Form("   Chi2/dof = %.2f / %d = %.2f", sottosmorzato_fit->GetChisquare(), sottosmorzato_fit->GetNDF(), sottosmorzato_fit->GetChisquare()/sottosmorzato_fit->GetNDF()));
    fitInfo_sottosmorzato->AddText(Form("   p-value  = %.3f", sottosmorzato_fit->GetProb()));
    fitInfo_sottosmorzato->Draw();
    //save canvas
    c_sottosmorzato->SaveAs("RLC_fit_sottosmorzato.png");


    c_crit_smorzato->cd();
    crit_smorzato_grph->SetMarkerColor(2);
    crit_smorzato_grph->SetMarkerStyle(20);
    crit_smorzato_grph->SetMarkerSize(0.5);
    crit_smorzato_grph->GetXaxis()->SetTitle("t [s]");
    crit_smorzato_grph->GetYaxis()->SetTitle("V [V]");
    crit_smorzato_grph->Draw("AP");
    crit_smorzato_fit->Draw("same");
    //create TPaveText to display fit result information
    TPaveText *fitInfo_crit_smorzato = new TPaveText(0.58, 0.58, 0.88, 0.88, "NDC");
    fitInfo_crit_smorzato->SetFillColor(0);
    fitInfo_crit_smorzato->SetTextAlign(12);
    fitInfo_crit_smorzato->AddText(Form("Risultati Fit:"));
    fitInfo_crit_smorzato->AddText(Form("   Chi2/dof = %.2f / %d = %.2f", crit_smorzato_fit->GetChisquare(), crit_smorzato_fit->GetNDF(), crit_smorzato_fit->GetChisquare()/crit_smorzato_fit->GetNDF()));
    fitInfo_crit_smorzato->AddText(Form("   p-value  = %.3f", crit_smorzato_fit->GetProb()));
    fitInfo_crit_smorzato->Draw();
    //save canvas
    c_crit_smorzato->SaveAs("RLC_fit_crit_smorzato.png");
        
    return 0;
}
