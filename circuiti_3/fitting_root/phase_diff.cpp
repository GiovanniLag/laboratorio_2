//compile with (root): g++ -o phase_diff phase_diff.cpp `root-config --cflags --glibs`
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TgraphErrors.h"
#include "TPaveText.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "../include/reader.h"
#include "../include/utils.h"
#include "../include/fitting.h"

#define max_BFOMTY_iters 10

int main(int argc, char const *argv[])
{
    //read list of folders
    std::vector<std::vector<std::string>*> folders;
    readCSVFile("../data/RC/folders.csv", folders);
    std::vector<std::string> folders_pahts = *folders[0];
    std::vector<std::vector<double>*> frequences;
    readCSVFile("../data/RC/frequences.csv", frequences);
    std::vector<double> freqs = *frequences[0];

    std::vector<std::vector<double>*> data_CH1;
    std::vector<double> CH1_time;
    std::vector<double> CH1_voltage;
    std::vector<double> CH1_time_err;
    std::vector<double> CH1_voltage_err;
    std::vector<std::vector<double>*> data_CH2;
    std::vector<double> CH2_time;
    std::vector<double> CH2_voltage;
    std::vector<double> CH2_time_err;
    std::vector<double> CH2_voltage_err;
    std::vector<std::vector<double>*> data_MTH;
    std::vector<double> MTH_time;
    std::vector<double> MTH_voltage;
    std::vector<double> MTH_time_err;
    std::vector<double> MTH_voltage_err;

    //initialize pharameters guess
    double phase_guess_CH1 = 0.0; // provide an initial guess for the phase
    double phase_range_CH1 = 0.1; // set the range of the fit to +/-0.1 radians around the expected phase
    double phase_min_CH1 = phase_guess_CH1 - phase_range_CH1;
    double phase_max_CH1 = phase_guess_CH1 + phase_range_CH1;
    

    //loop trough folders_paths vector
    std::vector<double> phase_CH1;
    std::vector<double> phase_CH2;
    std::vector<double> phase_MTH;
    std::vector<double> phase_err_CH1;
    std::vector<double> phase_err_CH2;
    std::vector<double> phase_err_MTH;
    
    for (int i = 0; i < folders_pahts.size(); i++)
    {
        //read data from files
        std::string pCH1 = folders_pahts[i] + "/CH1_clustered.csv";
        
        readCSVFile(pCH1, data_CH1);
        //fill vectors with data
        CH1_time = *data_CH1[0];
        CH1_voltage = *data_CH1[1];
        CH1_time_err = *data_CH1[2];
        CH1_voltage_err = *data_CH1[3];
        readCSVFile(folders_pahts[i] + "/CH2_clustered.csv", data_CH2);
        CH2_time = *data_CH2[0];
        CH2_voltage = *data_CH2[1];
        CH2_time_err = *data_CH2[2];
        CH2_voltage_err = *data_CH2[3];
        readCSVFile(folders_pahts[i] + "/MTH_clustered.csv", data_MTH);
        MTH_time = *data_MTH[0];
        MTH_voltage = *data_MTH[1];
        MTH_time_err = *data_MTH[2];
        MTH_voltage_err = *data_MTH[3];

        TF1 *sin_fit_CH1 = new TF1("sin_fit_CH1", "[0]*sin([1]*x+[2])", min(CH1_time), max(CH1_time));
        TF1 *sin_fit_CH2 = new TF1("sin_fit_CH2", "[0]*sin([1]*x+[2])", 0, max(CH2_time));
        TF1 *sin_fit_MTH = new TF1("sin_fit_MTH", "[0]*sin([1]*x+[2])", 0, max(MTH_time));
        std::vector<double> phase_min = {0.0};
        std::vector<double> phase_max = {TMath::Pi()};
        std::vector<double> par_gues = {0.5*(max(CH1_voltage)- min(CH1_voltage)), 2 * TMath::Pi() * freqs[i]};
        //order of parameters, "r" stand for range meaning that the parameter is given as a range, 
        //"g" stand for guess meaning that the parameter is given as a guess
        std::vector<std::string> order = {"g", "g", "r"};
        //fit
        TGraphErrors *gr_CH1 = new TGraphErrors(CH1_time.size(), &CH1_time[0], &CH1_voltage[0], &CH1_time_err[0], &CH1_voltage_err[0]);
        sin_fit_CH1 = BFOMTY(gr_CH1, sin_fit_CH1, phase_min, phase_max, max_BFOMTY_iters, order, par_gues);

        TGraphErrors *gr_CH2 = new TGraphErrors(CH2_time.size(), &CH2_time[0], &CH2_voltage[0], &CH2_time_err[0], &CH2_voltage_err[0]);
        sin_fit_CH2 = BFOMTY(gr_CH2, sin_fit_CH2, phase_min, phase_max, max_BFOMTY_iters, order, par_gues);

        TGraphErrors *gr_MTH = new TGraphErrors(MTH_time.size(), &MTH_time[0], &MTH_voltage[0], &MTH_time_err[0], &MTH_voltage_err[0]);
        sin_fit_MTH = BFOMTY(gr_MTH, sin_fit_MTH, phase_min, phase_max, max_BFOMTY_iters, order, par_gues);


        //get phase
        phase_CH1.push_back(sin_fit_CH1->GetParameter(2));
        phase_CH2.push_back(sin_fit_CH2->GetParameter(2));
        phase_MTH.push_back(sin_fit_MTH->GetParameter(2));
        phase_err_CH1.push_back(sin_fit_CH1->GetParError(2));
        phase_err_CH2.push_back(sin_fit_CH2->GetParError(2));
        phase_err_MTH.push_back(sin_fit_MTH->GetParError(2));

        //plot CH1 and save it
        TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
        gr_CH1->SetMarkerStyle(20);
        gr_CH1->SetMarkerSize(0.5);
        gr_CH1->SetMarkerColor(1);
        gr_CH1->Draw("AP");
        std::string save_path_CH1 = "CH1_imgs/CH1_";
        save_path_CH1.append(std::to_string(i));
        save_path_CH1.append(".png");
        c1->SaveAs(save_path_CH1.c_str());

        //plot CH2 and save it
        TCanvas *c2 = new TCanvas("c2", "c2", 1000, 1000);
        gr_CH2->SetMarkerStyle(20);
        gr_CH2->SetMarkerSize(0.5);
        gr_CH2->SetMarkerColor(2);
        gr_CH2->Draw("AP");
        std::string save_path_CH2 = "CH2_imgs/CH2_";
        save_path_CH2.append(std::to_string(i));
        save_path_CH2.append(".png");
        c2->SaveAs(save_path_CH2.c_str());

        //plot MTH and save it
        TCanvas *c3 = new TCanvas("c3", "c3", 1000, 1000);
        gr_MTH->SetMarkerStyle(20);
        gr_MTH->SetMarkerSize(0.5);
        gr_MTH->SetMarkerColor(3);
        gr_MTH->Draw("AP");
        std::string save_path_MTH = "MTH_imgs/MTH_";
        save_path_MTH.append(std::to_string(i));
        save_path_MTH.append(".png");
        c3->SaveAs(save_path_MTH.c_str());
        
        //delete TCanvas object
        delete c1;
        delete c2;
        delete c3;

        //delete TGraphErrors object
        delete gr_CH1;
        delete gr_CH2;
        delete gr_MTH;

        //empty vectors
        CH1_time.clear();
        CH1_voltage.clear();
        CH1_time_err.clear();
        CH1_voltage_err.clear();
        CH2_time.clear();
        CH2_voltage.clear();
        CH2_time_err.clear();
        CH2_voltage_err.clear();
        MTH_time.clear();
        MTH_voltage.clear();
        MTH_time_err.clear();
        MTH_voltage_err.clear();
        //clear data_CH1, data_CH2 and data_MTH
        data_CH1.clear();
        data_CH2.clear();
        data_MTH.clear();

        //delete TF1 object
        delete sin_fit_CH1;
        delete sin_fit_CH2;
        delete sin_fit_MTH;
    }

    //calculate phase difference
    std::vector<double> phase_diff_CH1_CH2;
    std::vector<double> phase_diff_CH1_MTH;

    for (int i = 0; i < phase_CH1.size(); i++)
    {
        phase_diff_CH1_CH2.push_back(phase_CH1[i] - phase_CH2[i]);
        phase_diff_CH1_MTH.push_back(phase_CH1[i] - phase_MTH[i]);
    }

    //freqs_err is vector of "10"
    std::vector<double> freqs_err;
    for (int i = 0; i < freqs.size(); i++)
    {
        freqs_err.push_back(10);
    }
    //plot phases with error bars in a single canvas using TMultiGraph
    TCanvas *c4 = new TCanvas("c4", "c4", 1000, 1000);
    TMultiGraph *mg = new TMultiGraph();
    TGraphErrors *gr_phase_CH1 = new TGraphErrors(phase_CH1.size(), &freqs[0], &phase_CH1[0], &freqs_err[0], &phase_err_CH1[0]);
    gr_phase_CH1->SetMarkerStyle(20);
    gr_phase_CH1->SetMarkerSize(0.9);
    gr_phase_CH1->SetMarkerColor(1);
    gr_phase_CH1->SetTitle("CH1");
    gr_phase_CH1->SetName("CH1");
    gr_phase_CH1->GetXaxis()->SetTitle("Frequency (Hz)");
    gr_phase_CH1->GetYaxis()->SetTitle("Phase (rad)");
    mg->Add(gr_phase_CH1);
    TGraphErrors *gr_phase_CH2 = new TGraphErrors(phase_CH2.size(), &freqs[0], &phase_CH2[0], &freqs_err[0], &phase_err_CH2[0]);
    gr_phase_CH2->SetMarkerStyle(20);
    gr_phase_CH2->SetMarkerSize(0.9);
    gr_phase_CH2->SetMarkerColor(2);
    gr_phase_CH2->SetTitle("CH2");
    gr_phase_CH2->SetName("CH2");
    gr_phase_CH2->GetXaxis()->SetTitle("Frequency (Hz)");
    gr_phase_CH2->GetYaxis()->SetTitle("Phase (rad)");
    mg->Add(gr_phase_CH2);
    TGraphErrors *gr_phase_MTH = new TGraphErrors(phase_MTH.size(), &freqs[0], &phase_MTH[0], &freqs_err[0], &phase_err_MTH[0]);
    gr_phase_MTH->SetMarkerStyle(20);
    gr_phase_MTH->SetMarkerSize(0.9);
    gr_phase_MTH->SetMarkerColor(3);
    gr_phase_MTH->SetTitle("MTH");
    gr_phase_MTH->SetName("MTH");
    gr_phase_MTH->GetXaxis()->SetTitle("Frequency (Hz)");
    gr_phase_MTH->GetYaxis()->SetTitle("Phase (rad)");
    mg->Add(gr_phase_MTH);

    //add legend
    TLegend *leg = new TLegend(0.58, 0.14, 0.88, 0.34);
    leg->AddEntry(gr_phase_CH1, "CH1", "p");
    leg->AddEntry(gr_phase_CH2, "CH2", "p");
    leg->AddEntry(gr_phase_MTH, "MTH", "p");
    leg->Draw();
    
    mg->SetTitle("Phase vs. Frequency");
    mg->GetXaxis()->SetTitle("Frequency (Hz)");
    mg->GetYaxis()->SetTitle("Phase (rad)");
    mg->Draw("AP");
    c4->BuildLegend();
    c4->SaveAs("phase.png");

    //plot phase difference
    TCanvas *c2 = new TCanvas("c2", "c2", 1000, 1000);
    TGraph *gr_phase_diff_CH1_CH2 = new TGraph(phase_diff_CH1_CH2.size(), &freqs[0], &phase_diff_CH1_CH2[0]);
    gr_phase_diff_CH1_CH2->SetMarkerStyle(20);
    gr_phase_diff_CH1_CH2->SetMarkerSize(0.5);
    gr_phase_diff_CH1_CH2->SetMarkerColor(1);
    //axis title
    gr_phase_diff_CH1_CH2->GetXaxis()->SetTitle("Frequency (Hz)");
    gr_phase_diff_CH1_CH2->GetYaxis()->SetTitle("Phase difference (rad)");
    gr_phase_diff_CH1_CH2->Draw("AP");
    c2->SaveAs("phase_diff_CH1_CH2.png");

    TCanvas *c3 = new TCanvas("c3", "c3", 1000, 1000);
    TGraph *gr_phase_diff_CH1_MTH = new TGraph(phase_diff_CH1_MTH.size(), &freqs[0], &phase_diff_CH1_MTH[0]);
    gr_phase_diff_CH1_MTH->SetMarkerStyle(20);
    gr_phase_diff_CH1_MTH->SetMarkerSize(0.5);
    gr_phase_diff_CH1_MTH->SetMarkerColor(1);
    //axis title
    gr_phase_diff_CH1_MTH->GetXaxis()->SetTitle("Frequency (Hz)");
    gr_phase_diff_CH1_MTH->GetYaxis()->SetTitle("Phase difference (rad)");
    gr_phase_diff_CH1_MTH->Draw("AP");
    c3->SaveAs("phase_diff_CH1_MTH.png");



    
    return 0;
}
