#Read a csv file with data from an oscilloscope and elaborates it
import os
import pandas as pd
import numpy as np
import sys

def st_dev(data):
    result_0 = (data - data.mean())
    result_1 = result_0 ** 2
    result_2 = result_1.sum()
    result_2 *= 1/(len(data)-1)
    result = np.sqrt(result_2)
    return result

#clustering function
def simple_sampling(x, y, n_samples):
    # Compute sampling interval
    sampling_interval = len(x) // n_samples
    # check if sampling interval allows to not have the last point alone
    if len(x) % sampling_interval == 1:
        sampling_interval += 1
        print("error: sampling interval does not allow full coverage of data, sampling interval increased by 1")

    # Initialize output lists
    new_x = []
    new_y = []
    new_error_x = []
    new_error_y = []

    # Iterate through data
    i = 0
    while i < len(x):
        # Add data point to output lists
        #use mean of the interval
        new_x.append(np.mean(np.array(x[i:i+sampling_interval])))
        new_y.append(np.mean(np.array(y[i:i+sampling_interval])))
        # check if there are enough points to compute the variance
        if i + sampling_interval < len(x):
            new_error_x.append(st_dev(np.array(x[i:i+sampling_interval])))
            new_error_y.append(st_dev(np.array(y[i:i+sampling_interval])))
        else: # if not, use the variance of the last points
            new_error_x.append(st_dev(np.array(x[i:])))
            new_error_y.append(st_dev(np.array(y[i:])))

        # Update index
        i += sampling_interval

    return new_x, new_y, new_error_x, new_error_y

#main functions that takes path as input
def main(path, n_samples=100):
    data = pd.read_csv(path)
    data = data.iloc[:,[3,4]]
    data.columns = ["time", "voltage"]
    data_t, data_v, data_t_err, data_v_err = simple_sampling(data["time"], data["voltage"], n_samples)
    #create a dataframe with the data
    data_elab = pd.DataFrame({"time": data_t, "voltage": data_v, "time_err": data_t_err, "voltage_err": data_v_err})
    #save the dataframe as a csv file
    data_elab.to_csv(path[:-4] + "_elab.csv", index=False)

if __name__ == "__main__":
    #get the number of samples as the first argument
    if len(sys.argv) > 1:
        n_samples = int(sys.argv[1])
    else:
        n_samples = 100
    #get the path of the file to elaborate
    path = input("Insert path of the file to elaborate: ")
    main(path, n_samples)


#one shall use the following command to run the script:
#python elab_single.py
#and then insert the path of the file to elaborate when prompted and to specify the nu