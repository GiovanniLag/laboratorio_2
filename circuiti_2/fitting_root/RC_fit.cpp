//compile with: g++ -o RC_fit RC_fit.cpp `root-config --cflags --glibs`

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

#define fitIters 100

using namespace std;

int main(int argc, char const *argv[])
{
    //read data from csv file
    vector<vector<double>*> data_charge;
    readCSVFile("../data/RC/charge_clust.csv", data_charge);
    vector<double> t_charge = *data_charge[0];
    vector<double> V_charge = *data_charge[1];
    vector<double> t_charge_err = *data_charge[2];
    vector<double> V_charge_err = *data_charge[3];

    vector<vector<double>*> data_discharge;
    readCSVFile("../data/RC/discharge_clust.csv", data_discharge);
    vector<double> t_discharge = *data_discharge[0];
    vector<double> V_discharge = *data_discharge[1];
    vector<double> t_discharge_err = *data_discharge[2];
    vector<double> V_discharge_err = *data_discharge[3];
    
    //create graphs with error bars
    vector<double> intial_params_charge = {2, 0.0000423};
    vector<double> intial_params_discharge = {2, 0.0000423};
    TGraphErrors *charge_graph = new TGraphErrors(t_charge.size(), &t_charge[0], &V_charge[0], &t_charge_err[0], &V_charge_err[0]);
    TGraphErrors *discharge_graph = new TGraphErrors(t_discharge.size(), &t_discharge[0], &V_discharge[0], &t_discharge_err[0], &V_discharge_err[0]);
    charge_graph->SetTitle("RC Carica");
    discharge_graph->SetTitle("RC Scarica");

    //fit graphs
    TF1 *charge_fit = new TF1("charge_fit", "[0]*(1-exp(-x/[1]))", 0, t_charge.at(t_charge.size()-1)); //range from 0 to last t value of charge
    TF1 *discharge_fit = new TF1("discharge_fit", "[0]*(exp(-x/[1]))", 0, t_discharge.at(t_discharge.size()-1));
    charge_fit->SetParameter(0, 2);
    charge_fit->SetParameter(1, 0.0000423);
    discharge_fit->SetParameter(0, 2);
    discharge_fit->SetParameter(1, 0.0000423);

    std::cout << "Fit charge" << std::endl;
    charge_graph->Fit(charge_fit, "R");
    //charge_fit = BFOMTY_fixed(charge_graph, charge_fit, fitIters, intial_params_charge);
    std::cout << "V_g = " << charge_fit->GetParameter(0) << " +- " << charge_fit->GetParError(0) << std::endl;
    std::cout << "tau = " << charge_fit->GetParameter(1) << " +- " << charge_fit->GetParError(1) << std::endl;

    std::cout << "Fit discharge" << std::endl;
    discharge_graph->Fit(discharge_fit, "R");
    //discharge_fit = BFOMTY_fixed(discharge_graph, discharge_fit, fitIters, intial_params_discharge, 0.5);
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
    charge_graph->GetXaxis()->SetTitle("Tempo [s]");
    charge_graph->GetYaxis()->SetTitle("Tensione [V]");
    charge_graph->Draw("AP");
    charge_fit->Draw("same");
    //create TPaveText to display fit result information
    TPaveText *fitInfo_charge = new TPaveText(0.58, 0.14, 0.88, 0.34, "NDC");
    fitInfo_charge->SetFillColor(0);
    fitInfo_charge->SetTextAlign(12);
    fitInfo_charge->AddText(Form("Risultati Fit:"));
    fitInfo_charge->AddText(Form("   Chi2/dof = %.2f / %d = %.2f", charge_fit->GetChisquare(), charge_fit->GetNDF(), charge_fit->GetChisquare()/charge_fit->GetNDF()));
    fitInfo_charge->AddText(Form("   p-value  = %.3f", charge_fit->GetProb()));
    fitInfo_charge->AddText(Form("   V_{C} = V_{g} #upoint (1-exp(-t/tau))"));
    //multiply by 10^5 to display in micro seconds
    fitInfo_charge->AddText(Form("   tau = (%.3f +- %.3f) #upoint 10^{-5}", charge_fit->GetParameter(1)*100000, charge_fit->GetParError(1)*100000));
    fitInfo_charge->AddText(Form("   V_{g} = (%.3f +- %.3f) V", charge_fit->GetParameter(0), charge_fit->GetParError(0)));
    fitInfo_charge->Draw();
    
    //blue for discharge -> in c_discharge canvas
    c_discharge->cd();
    discharge_graph->SetMarkerColor(4);
    discharge_graph->SetMarkerStyle(20);
    discharge_graph->SetMarkerSize(0.5);
    discharge_graph->GetXaxis()->SetTitle("Tempo [s]");
    discharge_graph->GetYaxis()->SetTitle("Tensione [V]");
    discharge_graph->Draw("AP");
    discharge_fit->Draw("same");
    //create TPaveText to display fit result information
    TPaveText *fitInfo_discharge = new TPaveText(0.58, 0.68, 0.88, 0.88, "NDC");
    fitInfo_discharge->SetFillColor(0);
    fitInfo_discharge->SetTextAlign(12);
    fitInfo_discharge->AddText(Form("Risultati Fit:"));
    fitInfo_discharge->AddText(Form("   Chi2/dof = %.2f / %d = %.2f", discharge_fit->GetChisquare(), discharge_fit->GetNDF(), discharge_fit->GetChisquare()/discharge_fit->GetNDF()));
    fitInfo_discharge->AddText(Form("   p-value  = %.3f", discharge_fit->GetProb()));
    fitInfo_discharge->AddText(Form("   V_{C} = V_{g} #upoint (exp(-t/tau))"));
    //multiply by 10^5 to display in micro seconds
    fitInfo_discharge->AddText(Form("   tau = (%.3f +- %.3f) #upoint 10^{-5}", discharge_fit->GetParameter(1)*100000, discharge_fit->GetParError(1)*100000));
    fitInfo_discharge->AddText(Form("   V_{g} = (%.3f +- %.3f) V", discharge_fit->GetParameter(0), discharge_fit->GetParError(0)));
    fitInfo_discharge->Draw();

    //save canvas
    c_charge->SaveAs("RC_fit_charge.png");
    c_discharge->SaveAs("RC_fit_discharge.png");
        
    return 0;
}
