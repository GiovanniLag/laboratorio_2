//compile with: g++ -o fit_RLC_R fit_RLC_R.cpp `root-config --cflags --glibs`
/*Fits RLC transfer function (module and argument) when the last component of the circuit is R*/
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

#define BFOMTY_iters 10000

/*definition of functions for fitting*/
//RLC ---------------------------------------------
//double RLC_HR_module(double * x, double * par)
//{
//    double freq = x[0];
//    double tau = par[0];
//    double nu = freq/(2*TMath::Pi());
//    double HR = (2*TMath::Pi()*freq)/TMath::Sqrt(4*TMath::Power(TMath::Pi()*nu*tau, 2) + TMath::Power(1 - 4*TMath::Power(TMath::Pi()*nu*tau, 2), 2));
//    return HR;
//}
//
//double RLC_HR_arg(double * x, double * par)
//{
//    double freq = x[0];
//    double tau = par[0];
//    double nu = freq/(2*TMath::Pi());
//    double HR = -TMath::ATan((1/(2*TMath::Pi()*nu*tau)) - 2*TMath::Pi()*nu*tau);
//    return HR;
//}

double RLC_HR_module(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double L = par[1];
    double C = par[2];
    double omega = 2 * M_PI * freq; // Convert frequency to angular frequency
    double numerator = R;
    double denominator = std::sqrt(std::pow(R, 2) + std::pow(omega*L - 1/(omega*C), 2));
    return numerator / denominator;
}

double RLC_HR_arg(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double L = par[1];
    double C = par[2];
    double omega = 2 * M_PI * freq; // Convert frequency to angular frequency
    return -std::atan((omega*L - 1/(omega*C)) / R);
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
    std::vector<double> amp_mth = *table[5];
    std::vector<double> amp_err_mth = *table[6];
    std::vector<double> phase_diff_ch1_ch2 = *table[7];
    std::vector<double> phase_diff_err_ch1_ch2 = *table[8];
    std::vector<double> phase_diff_ch1_mth = *table[9];
    std::vector<double> phase_diff_err_ch1_mth = *table[10];

    std::vector<double> freq_err_vector = std::vector<double>(freq.size(), freq_err);

    //make two kind of fits, one for amplitude and one for phase diff
    //amplitude
    TF1 *H1_module_fit;
    TF1 *H1_arg_fit;
    //double tau_fit = 4.653e-4;
    double R_fit = 9.9e3;
    double L_fit = 1.0e-3;
    double C_fit = 47.0e-9;
    H1_module_fit = new TF1("H1_module_fit", RLC_HR_module, min(freq), max(freq), 3);
    H1_arg_fit = new TF1("H1_arg_fit", RLC_HR_arg, min(freq), max(freq), 3);
    

    //calculate V_(g-z)/V_g (i.e. amp_ch2/amp_ch1)
    std::vector<double> amp_ch2_ch1;
    std::vector<double> amp_ch2_ch1_err;
    for (int i = 0; i < amp_ch1.size(); i++)
    {
        amp_ch2_ch1.push_back(amp_ch2[i]/amp_ch1[i]);
        amp_ch2_ch1_err.push_back(TMath::Sqrt(TMath::Power((amp_err_ch2[i]/amp_ch1[i]), 2) + TMath::Power(((amp_ch2[i]*amp_err_ch1[i])/TMath::Power(amp_ch1[i], 2)), 2)));
    }
    //calculate V_z/V_g (i.e. amp_mth/amp_ch1)
    std::vector<double> amp_mth_ch1;
    std::vector<double> amp_mth_ch1_err;
    for (int i = 0; i < amp_ch1.size(); i++)
    {
        amp_mth_ch1.push_back(amp_mth[i]/amp_ch1[i]);
        //error is (in python) = [np.sqrt((rc_loaded_table["err_amplitude_MTH"].to_list()[i]/rc_loaded_table["amplitude_CH1"].to_list()[i])**2 + (rc_loaded_table["err_amplitude_CH1"].to_list()[i]*rc_loaded_table["amplitude_MTH"].to_list()[i]/(rc_loaded_table["amplitude_CH1"].to_list()[i]**2))**2) for i in range(len(rc_loaded_table["amplitude_CH1"].to_list()))]
        amp_mth_ch1_err.push_back(TMath::Sqrt(TMath::Power((amp_err_mth[i]/amp_ch1[i]), 2) + TMath::Power(((amp_mth[i]*amp_err_ch1[i])/TMath::Power(amp_ch1[i], 2)), 2)));
    }

    //phase diffs have to be inverted (i.e. -phase_diff)
    for (int i = 0; i < phase_diff_ch1_ch2.size(); i++)
    {
        phase_diff_ch1_ch2[i] = -phase_diff_ch1_ch2[i];
        phase_diff_ch1_mth[i] = -phase_diff_ch1_mth[i];
    }

    TGraphErrors *amp_Vz_Vg_graph = new TGraphErrors(freq.size(), &freq[0], &amp_ch2_ch1[0], &freq_err_vector[0], &amp_ch2_ch1_err[0]);
    TGraphErrors *phase_diff_Vz_Vg_graph = new TGraphErrors(freq.size(), &freq[0], &phase_diff_ch1_ch2[0], &freq_err_vector[0], &phase_diff_err_ch1_ch2[0]);

    //fitting ----------------------------------------------------
    std::vector<double> init_pars = {R_fit, L_fit, C_fit};
    H1_module_fit = BFOMTY_fixed(amp_Vz_Vg_graph, H1_module_fit, BFOMTY_iters, init_pars);
    double tau = H1_module_fit->GetParameter(0);
    double tau_err = H1_module_fit->GetParError(0);
    std::cout << "H1 Module" << std::endl;
    std::cout << "tau = " << tau << " +- " << tau_err << std::endl;
    std::cout << std::endl;
    //argument (phase diff)

    H1_arg_fit = BFOMTY_fixed(phase_diff_Vz_Vg_graph, H1_arg_fit, BFOMTY_iters, init_pars);
    double tau_arg = H1_arg_fit->GetParameter(0);
    double tau_arg_err = H1_arg_fit->GetParError(0);
    std::cout << "H1 Argument" << std::endl;
    std::cout << "tau = " << tau_arg << " +- " << tau_arg_err << std::endl;
    std::cout << std::endl;
    //------------------------------------------------------------    

    H1_module_fit->SetRange(0, max(freq) + 5000);
    H1_arg_fit->SetRange(0, max(freq) + 5000);
    
    //plotting ----------------------------------------------------
    //H1
    //module (amplitude)
    TCanvas *c_amp_Vz_Vg = new TCanvas("c1", "c1", 1200, 1200);
    c_amp_Vz_Vg->SetLeftMargin(0.12);
    c_amp_Vz_Vg->cd();
    amp_Vz_Vg_graph->SetTitle("\\text{Modulo } V_{R} \\rightarrow V_{g}");
    amp_Vz_Vg_graph->GetYaxis()->SetTitle("\\frac{V_{R}}{V_{g}}");
    amp_Vz_Vg_graph->GetXaxis()->SetTitle("Frequenza [Hz]");
    amp_Vz_Vg_graph->SetMarkerStyle(20);
    amp_Vz_Vg_graph->SetMarkerSize(0.5);
    amp_Vz_Vg_graph->Draw("AP");
    //add TPaveText with fit parameters (use scientific notation)
    TPaveText *pt_amp_Vz_Vg;
    pt_amp_Vz_Vg = new TPaveText(0.6, 0.7, 0.9, 0.9, "brNDC");
    pt_amp_Vz_Vg->SetBorderSize(1);
    pt_amp_Vz_Vg->SetTextAlign(12);
    pt_amp_Vz_Vg->SetTextSize(0.02);
    pt_amp_Vz_Vg->SetFillColor(0);
    pt_amp_Vz_Vg->SetTextFont(42);
    pt_amp_Vz_Vg->AddText("Parametri fit:");//R, C, L
    pt_amp_Vz_Vg->AddText(Form("R = %.2e #pm %.2e", H1_module_fit->GetParameter(0), H1_module_fit->GetParError(0)));
    pt_amp_Vz_Vg->AddText(Form("C = %.2e #pm %.2e", H1_module_fit->GetParameter(1), H1_module_fit->GetParError(1)));
    pt_amp_Vz_Vg->AddText(Form("L = %.2e #pm %.2e", H1_module_fit->GetParameter(2), H1_module_fit->GetParError(2)));
    //add chi square / ndf to TPaveText
    pt_amp_Vz_Vg->AddText(Form("#chi^{2} / ndf = %.2f / %d = %.2f", H1_module_fit->GetChisquare(), H1_module_fit->GetNDF(), H1_module_fit->GetChisquare() / H1_module_fit->GetNDF()));
    //add p-value to TPaveText
    pt_amp_Vz_Vg->AddText(Form("p-value = %.2e", TMath::Prob(H1_module_fit->GetChisquare(), H1_module_fit->GetNDF())));
    pt_amp_Vz_Vg->Draw();
    c_amp_Vz_Vg->SaveAs("RLC_R_amp_Vz_Vg.png");
    
    //argument (phase diff)
    TCanvas *c_phase_diff_Vz_Vg = new TCanvas("c2", "c2", 1200, 1200);
    c_phase_diff_Vz_Vg->SetLeftMargin(0.12);
    c_phase_diff_Vz_Vg->cd();
    phase_diff_Vz_Vg_graph->SetTitle("\\text{Differenza di fase } V_{R} \\rightarrow V_{g}");
    phase_diff_Vz_Vg_graph->GetXaxis()->SetTitle("Frequenza [Hz]");
    phase_diff_Vz_Vg_graph->GetYaxis()->SetTitle("#Delta#phi [rad]");
    phase_diff_Vz_Vg_graph->SetMarkerStyle(20);
    phase_diff_Vz_Vg_graph->SetMarkerSize(0.5);
    phase_diff_Vz_Vg_graph->Draw("AP");
    //add TPaveText with fit parameters (use scientific notation)
    TPaveText *pt_phase_diff_Vz_Vg = new TPaveText(0.6, 0.7, 0.9, 0.9, "brNDC");
    pt_phase_diff_Vz_Vg->SetBorderSize(1);
    pt_phase_diff_Vz_Vg->SetTextAlign(12);
    pt_phase_diff_Vz_Vg->SetTextSize(0.02);
    pt_phase_diff_Vz_Vg->SetFillColor(0);
    pt_phase_diff_Vz_Vg->SetTextFont(42);
    pt_phase_diff_Vz_Vg->AddText("Parametri fit:"); //R, C, L
    pt_phase_diff_Vz_Vg->AddText(Form("R = (%.2e #pm %.2e) #Omega", H1_arg_fit->GetParameter(0), H1_arg_fit->GetParError(0)));
    pt_phase_diff_Vz_Vg->AddText(Form("C = (%.2e #pm %.2e) F", H1_arg_fit->GetParameter(1), H1_arg_fit->GetParError(1)));
    pt_phase_diff_Vz_Vg->AddText(Form("L = (%.2e #pm %.2e) H", H1_arg_fit->GetParameter(2), H1_arg_fit->GetParError(2)));
    //add chi square / ndf to TPaveText
    pt_phase_diff_Vz_Vg->AddText(Form("#chi^{2} / ndf = %.2f / %d = %.2f", H1_arg_fit->GetChisquare(), H1_arg_fit->GetNDF(), H1_arg_fit->GetChisquare() / H1_arg_fit->GetNDF()));
    //add p-value to TPaveText
    pt_phase_diff_Vz_Vg->AddText(Form("p-value = %.2e", TMath::Prob(H1_arg_fit->GetChisquare(), H1_arg_fit->GetNDF())));
    pt_phase_diff_Vz_Vg->Draw();
    c_phase_diff_Vz_Vg->SaveAs("RLC_R_phase_diff_Vz_Vg.png");
    //------------------------------------------------------------
    return 0;
}
