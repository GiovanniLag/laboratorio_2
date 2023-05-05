//compile with: g++ -o RL_fit RL_fit.cpp `root-config --cflags --glibs`

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

#define fitIters 10

using namespace std;

int main(int argc, char const *argv[])
{
    //read data from csv file
    vector<vector<double>*> data_charge;
    readCSVFile("../data/RL/charge_clust.csv", data_charge);
    vector<double> t_charge = *data_charge[0];
    vector<double> V_charge = *data_charge[1];
    vector<double> t_charge_err = *data_charge[2];
    vector<double> V_charge_err = *data_charge[3];

    vector<vector<double>*> data_discharge;
    readCSVFile("../data/RL/discharge_clust.csv", data_discharge);
    vector<double> t_discharge = *data_discharge[0];
    vector<double> V_discharge = *data_discharge[1];
    vector<double> t_discharge_err = *data_discharge[2];
    vector<double> V_discharge_err = *data_discharge[3];
    
    //create graphs with error bars
    TGraphErrors *charge_graph = new TGraphErrors(t_charge.size(), &t_charge[0], &V_charge[0], &t_charge_err[0], &V_charge_err[0]);
    TGraphErrors *discharge_graph = new TGraphErrors(t_discharge.size(), &t_discharge[0], &V_discharge[0], &t_discharge_err[0], &V_discharge_err[0]);
    charge_graph->SetTitle("RL Carica");
    discharge_graph->SetTitle("RL Scarica");

    //fit graphs
    TF1 *charge_fit = new TF1("charge_fit", "[0]*(1-exp(-x/[1]))", 0, t_charge.at(t_charge.size()-1)); //range from 0 to last t value of charge
    TF1 *discharge_fit = new TF1("discharge_fit", "[0]*(exp(-x/[1]))", 0, t_discharge.at(t_discharge.size()-1));
    charge_fit->SetParameter(0, 2);
    charge_fit->SetParameter(1, 0.0000145);
    discharge_fit->SetParameter(0, 2);
    discharge_fit->SetParameter(1, 0.0000145);

    std::vector<double> intial_params = {1.8, 0.0000145};

    //charge_fit = BFOMTY_fixed(charge_graph, charge_fit, fitIters, intial_params);
    //discharge_fit = BFOMTY_fixed(discharge_graph, discharge_fit, fitIters, intial_params);

    std::cout << "Fit charge" << std::endl;
    charge_graph->Fit(charge_fit, "R");
    std::cout << "V_g = " << charge_fit->GetParameter(0) << " +- " << charge_fit->GetParError(0) << std::endl;
    std::cout << "tau = " << charge_fit->GetParameter(1) << " +- " << charge_fit->GetParError(1) << std::endl;

    std::cout << "Fit discharge" << std::endl;
    discharge_graph->Fit(discharge_fit, "R");
    std::cout << "V_g = " << discharge_fit->GetParameter(0) << " +- " << discharge_fit->GetParError(0) << std::endl;
    std::cout << "tau = " << discharge_fit->GetParameter(1) << " +- " << discharge_fit->GetParError(1) << std::endl;

    //create canvases
    TCanvas *c_charge = new TCanvas("c1", "c1", 800, 600);
    TCanvas *c_discharge = new TCanvas("c2", "c2", 800, 600);

    //draw graphs with fit info
    //red for charge -> in c_charge canvas
    c_charge->cd();
    charge_graph->SetMarkerColor(2);
    charge_graph->SetMarkerStyle(20);
    charge_graph->SetMarkerSize(0.5);
    charge_graph->GetXaxis()->SetTitle("t [s]");
    charge_graph->GetYaxis()->SetTitle("V [V]");
    charge_graph->Draw("AP");
    charge_fit->Draw("same");
    //create TPaveText to display fit result information
    TPaveText *fitInfo_charge = new TPaveText(0.58, 0.14, 0.88, 0.34, "NDC");
    fitInfo_charge->SetFillColor(0);
    fitInfo_charge->SetTextAlign(12);
    fitInfo_charge->AddText(Form("Risultati Fit:"));
    fitInfo_charge->AddText(Form("   Chi2/dof = %.2f / %d = %.2f", charge_fit->GetChisquare(), charge_fit->GetNDF(), charge_fit->GetChisquare()/charge_fit->GetNDF()));
    fitInfo_charge->AddText(Form("   p-value  = %.3f", charge_fit->GetProb()));
    fitInfo_charge->Draw();
    
    //blue for discharge -> in c_discharge canvas
    c_discharge->cd();
    discharge_graph->SetMarkerColor(4);
    discharge_graph->SetMarkerStyle(20);
    discharge_graph->SetMarkerSize(0.5);
    discharge_graph->GetXaxis()->SetTitle("t [s]");
    discharge_graph->GetYaxis()->SetTitle("V [V]");
    discharge_graph->Draw("AP");
    discharge_fit->Draw("same");
    //create TPaveText to display fit result information
    TPaveText *fitInfo_discharge = new TPaveText(0.58, 0.68, 0.88, 0.88, "NDC");
    fitInfo_discharge->SetFillColor(0);
    fitInfo_discharge->SetTextAlign(12);
    fitInfo_discharge->AddText(Form("Risultati Fit:"));
    fitInfo_discharge->AddText(Form("   Chi2/dof = %.2f / %d = %.2f", discharge_fit->GetChisquare(), discharge_fit->GetNDF(), discharge_fit->GetChisquare()/discharge_fit->GetNDF()));
    fitInfo_discharge->AddText(Form("   p-value  = %.3f", discharge_fit->GetProb()));
    fitInfo_discharge->Draw();

    //save canvas
    c_charge->SaveAs("RL_fit_charge.png");
    c_discharge->SaveAs("RL_fit_discharge.png");
        
    return 0;
}