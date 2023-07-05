//compile with: g++ -o fit_trans fit_trans.cpp `root-config --cflags --glibs`
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
//RC ---------------------------------------------
double RC_H1_module(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double C = par[1];
    double H1 = (1.)/TMath::Sqrt(1+4*TMath::Power(TMath::Pi()*freq, 2)*TMath::Power(R*C, 2));
    return H1;
}
double RC_H2_module(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double C = par[1];
    double H1 = (2*TMath::Pi()*freq*R*C)/TMath::Sqrt(1+4*TMath::Power(TMath::Pi()*freq, 2)*TMath::Power(R*C, 2));
    return H1;
}

//H1_arg = -arctan(2\pi\nuRC)
double RC_H1_arg(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double C = par[1];
    double H1 = -TMath::ATan(2*TMath::Pi()*freq*R*C);
    return H1;
}
double RC_H2_arg(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double C = par[1];
    double H1 = TMath::ATan(1/(2*TMath::Pi()*freq*R*C));
    return H1;
}
// ---------------------------------------------
//RL ---------------------------------------------
double RL_H1_module(double * x, double * par)
{
    double freq = x[0];
    //double R = par[0];
    //double L = par[1];
    double tau = par[0];
    //double H1 = (2*TMath::Pi()*freq*L)/TMath::Sqrt(R*R+4*TMath::Power(TMath::Pi()*freq*L, 2));
    double H1 = (2*TMath::Pi()*freq)/TMath::Sqrt(TMath::Power(1/tau, 2) + 4 * TMath::Power(TMath::Pi()*freq, 2));
    return H1;   
}
double RL_H2_module(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double L = par[1];
    double H1 = R/TMath::Sqrt(R*R + TMath::Power(2 * TMath::Pi()*freq*L, 2));
    //double H1 = 1 / TMath::Sqrt(1 + TMath::Power((2 * TMath::Pi() * freq * L) / R, 2));
    return H1;
}


double RL_H1_arg(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double L = par[1];
    double H1 = TMath::Pi()/2 - TMath::ATan(2*TMath::Pi()*freq*L/R);
    return H1;
}
double RL_H2_arg(double * x, double * par)
{
    double freq = x[0];
    double R = par[0];
    double L = par[1];
    double H1 = TMath::ATan(2*TMath::Pi()*freq*L/R);
    return H1;
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
    TF1 *H2_module_fit;
    TF1 *H1_arg_fit;
    TF1 *H2_arg_fit;
    double R_fit = 9.9e3;
    double Z_fit;
    if (path.find("RC") != std::string::npos)
    {
        H1_module_fit = new TF1("H1_module_fit", RC_H1_module, min(freq), max(freq), 2);
        H1_arg_fit = new TF1("H1_arg_fit", RC_H1_arg, min(freq), max(freq), 2);
        H2_module_fit = new TF1("H2_module_fit", RC_H2_module, min(freq), max(freq), 2);
        H2_arg_fit = new TF1("H2_arg_fit", RC_H2_arg, min(freq), max(freq), 2);
        Z_fit = 47e-9;
    }
    else if (path.find("RL") != std::string::npos)
    {
        H1_module_fit = new TF1("H1_module_fit", RL_H1_module, min(freq), max(freq), 1);
        H1_arg_fit = new TF1("H1_arg_fit", RL_H1_arg, min(freq), max(freq), 2);
        H2_module_fit = new TF1("H2_module_fit", RL_H2_module, min(freq), max(freq), 2);
        H2_arg_fit = new TF1("H2_arg_fit", RL_H2_arg, min(freq), max(freq), 2);
        Z_fit = 0.06;
    }
    else
    {
        std::cout << "Error: no fit function found for this kind of circuit" << std::endl;
        return 1;
    }
    

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

    TGraphErrors *amp_Vz_Vg_graph = new TGraphErrors(freq.size(), &freq[0], &amp_mth_ch1[0], &freq_err_vector[0], &amp_mth_ch1_err[0]);
    TGraphErrors *amp_Vgz_Vg_graph = new TGraphErrors(freq.size(), &freq[0], &amp_ch2_ch1[0], &freq_err_vector[0], &amp_ch2_ch1_err[0]);
    TGraphErrors *phase_diff_Vz_Vg_graph = new TGraphErrors(freq.size(), &freq[0], &phase_diff_ch1_mth[0], &freq_err_vector[0], &phase_diff_err_ch1_mth[0]);
    TGraphErrors *phase_diff_Vgz_Vg_graph = new TGraphErrors(freq.size(), &freq[0], &phase_diff_ch1_ch2[0], &freq_err_vector[0], &phase_diff_err_ch1_ch2[0]);
    
    //fitting ----------------------------------------------------
    std::vector<double> init_pars = {R_fit, Z_fit};
    //H1
    //module (amplitude)
    //H1_module_fit->SetParameters(R_fit, Z_fit);
    //amp_Vz_Vg_graph->Fit("H1_module_fit", "R");
    H1_module_fit = BFOMTY_fixed(amp_Vz_Vg_graph, H1_module_fit, BFOMTY_iters, init_pars);
    double R = H1_module_fit->GetParameter(0);
    double R_err = H1_module_fit->GetParError(0);
    double Z = H1_module_fit->GetParameter(1);
    double Z_err = H1_module_fit->GetParError(1);
    std::cout << "H1 Module" << std::endl;
    std::cout << "R = " << R << " +- " << R_err << std::endl;
    std::cout << "Z(C or L) = " << Z << " +- " << Z_err << std::endl;
    std::cout << std::endl;
    //argument (phase diff)
    //H1_arg_fit->SetParameters(R_fit, Z_fit);
    //phase_diff_Vz_Vg_graph->Fit("H1_arg_fit", "R");
    H1_arg_fit = BFOMTY_fixed(phase_diff_Vz_Vg_graph, H1_arg_fit, BFOMTY_iters, init_pars);
    R = H1_arg_fit->GetParameter(0);
    R_err = H1_arg_fit->GetParError(0);
    Z = H1_arg_fit->GetParameter(1);
    Z_err = H1_arg_fit->GetParError(1);
    std::cout << "H1 Argument" << std::endl;
    std::cout << "R = " << R << " +- " << R_err << std::endl;
    std::cout << "Z(C or L) = " << Z << " +- " << Z_err << std::endl;
    std::cout << std::endl;
    //H2
    //module (amplitude)
    //H2_module_fit->SetParameters(R_fit, Z_fit);
    //amp_Vgz_Vg_graph->Fit("H2_module_fit", "R");
    H2_module_fit = BFOMTY_fixed(amp_Vgz_Vg_graph, H2_module_fit, BFOMTY_iters, init_pars);
    R = H2_module_fit->GetParameter(0);
    R_err = H2_module_fit->GetParError(0);
    Z = H2_module_fit->GetParameter(1);
    Z_err = H2_module_fit->GetParError(1);
    std::cout << "H2 Module" << std::endl;
    std::cout << "R = " << R << " +- " << R_err << std::endl;
    std::cout << "Z(C or L) = " << Z << " +- " << Z_err << std::endl;
    std::cout << std::endl;
    //argument (phase diff)
    //H2_arg_fit->SetParameters(R_fit, Z_fit);
    //phase_diff_Vgz_Vg_graph->Fit("H2_arg_fit", "R");
    H2_arg_fit = BFOMTY_fixed(phase_diff_Vgz_Vg_graph, H2_arg_fit, BFOMTY_iters, init_pars);
    R = H2_arg_fit->GetParameter(0);
    R_err = H2_arg_fit->GetParError(0);
    Z = H2_arg_fit->GetParameter(1);
    Z_err = H2_arg_fit->GetParError(1);
    std::cout << "H2 Argument" << std::endl;
    std::cout << "R = " << R << " +- " << R_err << std::endl;
    std::cout << "Z(C or L) = " << Z << " +- " << Z_err << std::endl;
    std::cout << std::endl;
    //------------------------------------------------------------


    //plotting ----------------------------------------------------
    //H1
    //module (amplitude)
    TCanvas *c_amp_Vz_Vg = new TCanvas("c1", "c1", 1200, 1200);
    c_amp_Vz_Vg->SetLeftMargin(0.12);
    c_amp_Vz_Vg->cd();
    if (path.find("RC") != std::string::npos)
    {
        amp_Vz_Vg_graph->SetTitle("\\text{Modulo } V_{C} \\rightarrow V_{g}");
        amp_Vz_Vg_graph->GetYaxis()->SetTitle("\\frac{V_{C}}{V_{g}}");
    }
    else if (path.find("RL") != std::string::npos)
    {
        amp_Vz_Vg_graph->SetTitle("\\text{Modulo } V_{L} \\rightarrow V_{L}");
        amp_Vz_Vg_graph->GetYaxis()->SetTitle("\\frac{V_{L}}{V_{g}}");
    }
    
    amp_Vz_Vg_graph->GetXaxis()->SetTitle("Frequenza [Hz]");
    amp_Vz_Vg_graph->SetMarkerStyle(20);
    amp_Vz_Vg_graph->SetMarkerSize(0.5);
    amp_Vz_Vg_graph->Draw("AP");
    //add TPaveText with fit parameters (use scientific notation)
    TPaveText *pt_amp_Vz_Vg;
    if (path.find("RC") != std::string::npos){
        pt_amp_Vz_Vg = new TPaveText(0.6, 0.7, 0.9, 0.9, "brNDC");
    }
    else if (path.find("RL") != std::string::npos){
        pt_amp_Vz_Vg = new TPaveText(0.6, 0.15, 0.9, 0.35, "brNDC");
    }
    pt_amp_Vz_Vg->SetBorderSize(1);
    pt_amp_Vz_Vg->SetTextAlign(12);
    pt_amp_Vz_Vg->SetTextSize(0.02);
    pt_amp_Vz_Vg->SetFillColor(0);
    pt_amp_Vz_Vg->SetTextFont(42);
    pt_amp_Vz_Vg->AddText("Fit parameters:");
    pt_amp_Vz_Vg->AddText(Form("R = (%.2e #pm %.2e) #Omega", H1_module_fit->GetParameter(0), H1_module_fit->GetParError(0)));
    if (path.find("RC") != std::string::npos) {
        pt_amp_Vz_Vg->AddText(Form("C = (%.2e #pm %.2e) F", H1_module_fit->GetParameter(1), H1_module_fit->GetParError(1)));
    } else if (path.find("RL") != std::string::npos) {
        //pt_amp_Vz_Vg->AddText(Form("L = (%.2e #pm %.2e) H", H1_module_fit->GetParameter(1), H1_module_fit->GetParError(1)));
    }
    //add chi square / ndf to TPaveText
    pt_amp_Vz_Vg->AddText(Form("#chi^{2} / ndf = %.2f / %d = %.2f", H1_module_fit->GetChisquare(), H1_module_fit->GetNDF(), H1_module_fit->GetChisquare() / H1_module_fit->GetNDF()));
    //add p-value to TPaveText
    pt_amp_Vz_Vg->AddText(Form("p-value = %.2f", TMath::Prob(H1_module_fit->GetChisquare(), H1_module_fit->GetNDF())));
    pt_amp_Vz_Vg->Draw();
    if (path.find("RC") != std::string::npos)
    {
        c_amp_Vz_Vg->SaveAs("RC_amp_Vz_Vg.png");
    }
    else if (path.find("RL") != std::string::npos)
    {
        c_amp_Vz_Vg->SaveAs("RL_amp_Vz_Vg.png");
    }
    
    //argument (phase diff)
    TCanvas *c_phase_diff_Vz_Vg = new TCanvas("c2", "c2", 1200, 1200);
    c_phase_diff_Vz_Vg->SetLeftMargin(0.12);
    c_phase_diff_Vz_Vg->cd();
    if (path.find("RC") != std::string::npos)
    {
        phase_diff_Vz_Vg_graph->SetTitle("\\text{Differenza di fase } V_{C} \\rightarrow V_{g}");
    }
    else if (path.find("RL") != std::string::npos)
    {
        phase_diff_Vz_Vg_graph->SetTitle("\\text{Differenza di fase } V_{L} \\rightarrow V_{g}");
    }
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
    pt_phase_diff_Vz_Vg->AddText("Parametri fit:");
    pt_phase_diff_Vz_Vg->AddText(Form("R = (%.2e #pm %.2e) #Omega", H1_arg_fit->GetParameter(0), H1_arg_fit->GetParError(0)));
    if (path.find("RC") != std::string::npos) {
        pt_phase_diff_Vz_Vg->AddText(Form("C = (%.2e #pm %.2e) F", H1_arg_fit->GetParameter(1), H1_arg_fit->GetParError(1)));
    } else if (path.find("RL") != std::string::npos) {
        pt_phase_diff_Vz_Vg->AddText(Form("L = (%.2e #pm %.2e) H", H1_arg_fit->GetParameter(1), H1_arg_fit->GetParError(1)));
    }
    //add chi square / ndf to TPaveText
    pt_phase_diff_Vz_Vg->AddText(Form("#chi^{2} / ndf = %.2f / %d = %.2f", H1_arg_fit->GetChisquare(), H1_arg_fit->GetNDF(), H1_arg_fit->GetChisquare() / H1_arg_fit->GetNDF()));
    //add p-value to TPaveText
    pt_phase_diff_Vz_Vg->AddText(Form("p-value = %.2f", TMath::Prob(H1_arg_fit->GetChisquare(), H1_arg_fit->GetNDF())));
    pt_phase_diff_Vz_Vg->Draw();
    if (path.find("RC") != std::string::npos)
    {
        c_phase_diff_Vz_Vg->SaveAs("RC_phase_diff_Vz_Vg.png");
    }
    else if (path.find("RL") != std::string::npos)
    {
        c_phase_diff_Vz_Vg->SaveAs("RL_phase_diff_Vz_Vg.png");
    }

    //H2
    //module (amplitude)
    TCanvas *c_amp_Vgz_Vg = new TCanvas("c2", "c2", 1200, 1200);
    c_amp_Vgz_Vg->SetLeftMargin(0.12);
    c_amp_Vgz_Vg->cd();
    if (path.find("RC") != std::string::npos)
    {
        amp_Vgz_Vg_graph->SetTitle("\\text{Modulo } V_{C} \\rightarrow V_{g}");
        amp_Vgz_Vg_graph->GetYaxis()->SetTitle("\\frac{V_{C}}{V_{g}}");
    }
    else if (path.find("RL") != std::string::npos)
    {
        amp_Vgz_Vg_graph->SetTitle("\\modulo{Modulo } V_{L} \\rightarrow V_{g}");
        amp_Vgz_Vg_graph->GetYaxis()->SetTitle("\\frac{V_{L}}{V_{g}}");
    }
    amp_Vgz_Vg_graph->GetXaxis()->SetTitle("Frequenza [Hz]");
    amp_Vgz_Vg_graph->SetMarkerStyle(20);
    amp_Vgz_Vg_graph->SetMarkerSize(0.5);
    //set y axis range
    amp_Vgz_Vg_graph->Draw("AP");
    //add TPaveText with fit parameters (use scientific notation)
    TPaveText *pt_amp_Vgz_Vg;
    if(path.find("RC") != std::string::npos){
        pt_amp_Vgz_Vg = new TPaveText(0.6, 0.35, 0.9, 0.15, "brNDC");
    }else{
        pt_amp_Vgz_Vg = new TPaveText(0.6, 0.7, 0.9, 0.9, "brNDC");
    }
    pt_amp_Vgz_Vg->SetBorderSize(1);
    pt_amp_Vgz_Vg->SetTextAlign(12);
    pt_amp_Vgz_Vg->SetTextSize(0.02);
    pt_amp_Vgz_Vg->SetFillColor(0);
    pt_amp_Vgz_Vg->SetTextFont(42);
    pt_amp_Vgz_Vg->AddText("Parametri fit:");
    pt_amp_Vgz_Vg->AddText(Form("R = (%.2e #pm %.2e) #Omega", H2_module_fit->GetParameter(0), H2_module_fit->GetParError(0)));
    if (path.find("RC") != std::string::npos) {
        pt_amp_Vgz_Vg->AddText(Form("C = (%.2e #pm %.2e) F", H2_module_fit->GetParameter(1), H2_module_fit->GetParError(1)));
    } else if (path.find("RL") != std::string::npos) {
        pt_amp_Vgz_Vg->AddText(Form("L = (%.2e #pm %.2e) H", H2_module_fit->GetParameter(1), H2_module_fit->GetParError(1)));
    }
    //add chi square / ndf to TPaveText
    pt_amp_Vgz_Vg->AddText(Form("#chi^{2} / ndf = %.2f / %d = %.2f", H2_module_fit->GetChisquare(), H2_module_fit->GetNDF(), H2_module_fit->GetChisquare() / H2_module_fit->GetNDF()));
    //add p-value to TPaveText
    pt_amp_Vgz_Vg->AddText(Form("p-value = %.2f", TMath::Prob(H2_module_fit->GetChisquare(), H2_module_fit->GetNDF())));
    pt_amp_Vgz_Vg->Draw();
    if (path.find("RC") != std::string::npos)
    {
        c_amp_Vgz_Vg->SaveAs("RC_amp_Vgz_Vg.png");
    }
    else if (path.find("RL") != std::string::npos)
    {
        c_amp_Vgz_Vg->SaveAs("RL_amp_Vgz_Vg.png");
    }
    
    //argument (phase diff)
    TCanvas *c_phase_diff_Vgz_Vg = new TCanvas("c2", "c2", 1200, 1200);
    c_phase_diff_Vgz_Vg->SetLeftMargin(0.12);
    c_phase_diff_Vgz_Vg->cd();
    if (path.find("RC") != std::string::npos)
    {
        phase_diff_Vgz_Vg_graph->SetTitle("\\text{Differenza di fase } V_{C} \\rightarrow V_{g}");
    }
    else if (path.find("RL") != std::string::npos)
    {
        phase_diff_Vgz_Vg_graph->SetTitle("\\text{Differenza di fase } V_{L} \\rightarrow V_{g}");
    }
    phase_diff_Vgz_Vg_graph->GetXaxis()->SetTitle("Frequenza [Hz]");
    phase_diff_Vgz_Vg_graph->GetYaxis()->SetTitle("#Delta#phi [rad]");
    phase_diff_Vgz_Vg_graph->SetMarkerStyle(20);
    phase_diff_Vgz_Vg_graph->SetMarkerSize(0.5);
    phase_diff_Vgz_Vg_graph->Draw("AP");
    //add TPaveText with fit parameters (use scientific notation)
    TPaveText *pt_phase_diff_Vgz_Vg = new TPaveText(0.6, 0.7, 0.9, 0.9, "brNDC");
    pt_phase_diff_Vgz_Vg->SetBorderSize(1);
    pt_phase_diff_Vgz_Vg->SetTextAlign(12);
    pt_phase_diff_Vgz_Vg->SetTextSize(0.02);
    pt_phase_diff_Vgz_Vg->SetFillColor(0);
    pt_phase_diff_Vgz_Vg->SetTextFont(42);
    pt_phase_diff_Vgz_Vg->AddText("Parametri fit:");
    pt_phase_diff_Vgz_Vg->AddText(Form("R = (%.2e #pm %.2e) #Omega", H2_arg_fit->GetParameter(0), H2_arg_fit->GetParError(0)));
    if (path.find("RC") != std::string::npos) {
        pt_phase_diff_Vgz_Vg->AddText(Form("C = (%.2e #pm %.2e) F", H2_arg_fit->GetParameter(1), H2_arg_fit->GetParError(1)));
    } else if (path.find("RL") != std::string::npos) {
        pt_phase_diff_Vgz_Vg->AddText(Form("L = (%.2e #pm %.2e) H", H2_arg_fit->GetParameter(1), H2_arg_fit->GetParError(1)));
    }
    //add chi square / ndf to TPaveText
    pt_phase_diff_Vgz_Vg->AddText(Form("#chi^{2} / ndf = %.2f / %d = %.2f", H2_arg_fit->GetChisquare(), H2_arg_fit->GetNDF(), H2_arg_fit->GetChisquare() / H2_arg_fit->GetNDF()));
    //add p-value to TPaveText
    pt_phase_diff_Vgz_Vg->AddText(Form("p-value = %.2f", TMath::Prob(H2_arg_fit->GetChisquare(), H2_arg_fit->GetNDF())));
    pt_phase_diff_Vgz_Vg->Draw();
    if (path.find("RC") != std::string::npos)
    {
        c_phase_diff_Vgz_Vg->SaveAs("RC_phase_diff_Vgz_Vg.png");
    }
    else if (path.find("RL") != std::string::npos)
    {
        c_phase_diff_Vgz_Vg->SaveAs("RL_phase_diff_Vgz_Vg.png");
    }
    //------------------------------------------------------------

    
    return 0;
}
