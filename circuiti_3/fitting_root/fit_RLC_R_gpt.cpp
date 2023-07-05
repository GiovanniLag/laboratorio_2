//compile with: g++ -o fit_RLC_R fit_RLC_R.cpp `root-config --cflags --glibs`
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
#include <vector>
#include "../include/reader.h"
#include "../include/utils.h"
#include "../include/fitting.h"
#include "../include/writer.h"

#define BFOMTY_iters 1000

/*definition of functions for fitting*/
//RLC ---------------------------------------------
double RLC_HR_module(double * x, double * par)
{
    double freq = x[0];
    double tau = par[0];
    double nu = freq/(2*TMath::Pi());
    double HR = (2*TMath::Pi()*freq)/TMath::Sqrt(4*TMath::Power(TMath::Pi()*nu*tau, 2) + TMath::Power(1 - 4*TMath::Power(TMath::Pi()*nu*tau, 2), 2));
    return HR;
}

double RLC_HR_arg(double * x, double * par)
{
    double freq = x[0];
    double tau = par[0];
    double nu = freq/(2*TMath::Pi());
    double HR = -TMath::ATan((1/(2*TMath::Pi()*nu*tau)) - 2*TMath::Pi()*nu*tau);
    return HR;
}
// ---------------------------------------------

int main(int argc, char const *argv[])
{
    std::string path = argv[1];
    double freq_err = atof(argv[2]);
    std::vector<std::vector<double>*> table;
    readCSVFile(path, table);
    std::vector<double> freq = *table[0];
    std::vector<double> amp_ch1 = *table[1];
    std::vector<double> amp_err_ch1 = *table[2];
    std::vector<double> amp_ch2 = *table[3];
    std::vector<double> amp_err_ch2 = *table[4];
    std::vector<double> phase_diff_ch1_ch2 = *table[5];
    std::vector<double> phase_diff_err_ch1_ch2 = *table[6];

    std::vector<double> freq_err_vector = std::vector<double>(freq.size(), freq_err);

    //make two kind of fits, one for amplitude and one for phase diff
    //amplitude
    TF1 *HR_module_fit;
    TF1 *HR_arg_fit;
    double tau_fit;
    if (path.find("RLC") != std::string::npos)
    {
        HR_module_fit = new TF1("HR_module_fit", RLC_HR_module, min(freq), max(freq), 1);
        HR_arg_fit = new TF1("HR_arg_fit", RLC_HR_arg, min(freq), max(freq), 1);
        tau_fit = 4.653e-4; // This is an example value, you should replace it with your initial guess for tau
    }
    else
    {
        std::cout << "Error: no fit function found for this kind of circuit" << std::endl;
        return 1;
    }

    TGraphErrors *amp_Vz_Vg_graph = new TGraphErrors(freq.size(), &freq[0], &amp_ch1[0], &freq_err_vector[0], &amp_err_ch1[0]);
    TGraphErrors *phase_diff_Vz_Vg_graph = new TGraphErrors(freq.size(), &freq[0], &phase_diff_ch1_ch2[0], &freq_err_vector[0], &phase_diff_err_ch1_ch2[0]);
    
    //fitting ----------------------------------------------------
    std::vector<double> init_pars = {tau_fit};
    //HR
    //module (amplitude)
    HR_module_fit = BFOMTY_fixed(amp_Vz_Vg_graph, HR_module_fit, BFOMTY_iters, init_pars);
    double tau = HR_module_fit->GetParameter(0);
    double tau_err = HR_module_fit->GetParError(0);
    std::cout << "HR Module" << std::endl;
    std::cout << "tau = " << tau << " +- " << tau_err << std::endl;
    std::cout << std::endl;
    //argument (phase diff)
    HR_arg_fit = BFOMTY_fixed(phase_diff_Vz_Vg_graph, HR_arg_fit, BFOMTY_iters, init_pars);
    double tau_arg = HR_arg_fit->GetParameter(0);
    double tau_arg_err = HR_arg_fit->GetParError(0);
    std::cout << "HR Argument" << std::endl;
    std::cout << "tau = " << tau_arg << " +- " << tau_arg_err << std::endl;
    std::cout << std::endl;
    //------------------------------------------------------------

    //plotting ----------------------------------------------------
    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    TMultiGraph *mg = new TMultiGraph();

    amp_Vz_Vg_graph->SetMarkerColor(kRed);
    amp_Vz_Vg_graph->SetLineColor(kRed);
    amp_Vz_Vg_graph->SetMarkerStyle(20);
    amp_Vz_Vg_graph->SetMarkerSize(0.8);
    amp_Vz_Vg_graph->SetTitle("Amplitude");
    mg->Add(amp_Vz_Vg_graph);

    HR_module_fit->SetLineColor(kRed);
    HR_module_fit->SetLineStyle(2);
    HR_module_fit->Draw("same");

    phase_diff_Vz_Vg_graph->SetMarkerColor(kBlue);
    phase_diff_Vz_Vg_graph->SetLineColor(kBlue);
    phase_diff_Vz_Vg_graph->SetMarkerStyle(20);
    phase_diff_Vz_Vg_graph->SetMarkerSize(0.8);
    phase_diff_Vz_Vg_graph->SetTitle("Phase difference");
    mg->Add(phase_diff_Vz_Vg_graph);

    HR_arg_fit->SetLineColor(kBlue);
    HR_arg_fit->SetLineStyle(2);
    HR_arg_fit->Draw("same");

    mg->Draw("AP");
    mg->GetXaxis()->SetTitle("Frequency (Hz)");
    mg->GetYaxis()->SetTitle("Amplitude / Phase difference");

    TLegend *leg = new TLegend(0.1,0.7,0.3,0.9);
    leg->SetHeader("Legend","C"); // option "C" allows to center the header
    leg->AddEntry(amp_Vz_Vg_graph,"Amplitude","lep");
    leg->AddEntry(HR_module_fit,"Fit Amplitude","l");
    leg->AddEntry(phase_diff_Vz_Vg_graph,"Phase difference","lep");
    leg->AddEntry(HR_arg_fit,"Fit Phase difference","l");
    leg->Draw();

    c1->SaveAs("RLC_R_fit.png");
    //------------------------------------------------------------

    return 0;
}
