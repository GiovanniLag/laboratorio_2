#include <vector>
#include <iostream>
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"

TF1* SingleRandomFit(TGraphErrors* tofit, TF1* fit_function, std::vector<double> params_max, std::vector<double> params_min, std::vector<std::string> order, std::vector<double> fixed_params = {})
{
    //set initial parameters, following the order
    //order of parameters: "r" stand for range meaning that the parameter is given as a range, 
    //"g" stand for guess meaning that the parameter is given as a guess
    double params[params_max.size() + fixed_params.size()];
    int i = 0;
    int j = 0;
    for (int k = 0; k < params_max.size() + fixed_params.size(); k++)
    {
        if (order[k] == "r")
        {
            //if the parameter is a range, generate a random number in the range
            params[k] = params_min[i] + (params_max[i] - params_min[i]) * rand() / (RAND_MAX + 1.0);
            i++;
        }
        else if (order[k] == "g")
        {
            //if the parameter is a guess, use the guess
            params[k] = fixed_params[j];
            j++;
        }
    }
    //set initial parameters
    fit_function->SetParameters(params);
    //set parameter ranges
    for (int k = 0; k < params_max.size() + fixed_params.size(); k++)
    {
        if (order[k] == "r")
        {
            fit_function->SetParLimits(k, params_min[i], params_max[i]);
        }
    }

    //fit the graph
    tofit->Fit(fit_function, "BMQS");
    //return the fit
    return fit_function;
}

TF1* SingleRandomFit_fixed(TGraphErrors* tofit, TF1* fit_function, std::vector<double> fixed_params)
{
    //set initial parameters
    double params[fixed_params.size()];
    int i = 0;
    for (int k = 0; k < fixed_params.size(); k++)
    {
        params[k] = fixed_params[i];
        i++;
    }
    //set initial parameters
    fit_function->SetParameters(params);

    //fit the graph
    tofit->Fit(fit_function, "BMQS");
    //return the fit
    return fit_function;
}

//Best Fit Of Multiple Trys
TF1* BFOMTY(TGraphErrors* tofit, TF1* fit_function, std::vector<double> params_max, std::vector<double> params_min, size_t iterations, std::vector<std::string> order,std::vector<double> fixed_params = {})
{
    TF1* best_fit = nullptr;
    //try to fit the graph multiple times
    double best_chi2 = 0;
    for (int i = 0; i < iterations; i++)
    {
        //get a random fit
        TF1* fit = SingleRandomFit(tofit, fit_function, params_max, params_min, order, fixed_params);

        //check if it is the best fit
        if (i == 0 || fit->GetChisquare() < best_chi2)
        {
            best_fit = fit;
            best_chi2 = fit->GetChisquare();
        }
        //delete fit;
    }
    //return the best fit
    return best_fit;
}

TF1* BFOMTY_fixed(TGraphErrors* tofit, TF1* fit_function, size_t iterations, std::vector<double> fixed_params)
{
    TF1* best_fit = nullptr;
    //try to fit the graph multiple times
    double best_chi2 = 0;
    for (int i = 0; i < iterations; i++)
    {
        //get a random fit
        TF1* fit = SingleRandomFit_fixed(tofit, fit_function, fixed_params);
        //check if it is the best fit
        if (i == 0 || fit->GetChisquare() < best_chi2)
        {
            best_fit = fit;
            best_chi2 = fit->GetChisquare();
        }
        //delete fit;
    }
    //return the best fit
    return best_fit;
}

