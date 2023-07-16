# Data and Programs for the Lab (2) Course at Unimib Physics

## Data
Data from each experiment can be found in the subfolder "data" within each experiment's main folder. All data is saved in CSV files, except for some constants that have been directly written in the Python notebook.

## Programs
For each experiment, there is a Python notebook that contains the code for data preparation and analysis. Additionally, each experiment has a folder named "fitting_root" where there are C++ scripts to fit the data using ROOT. Images resulting from these fits are saved in the same folder.
The libraries used in C++ are saved in the "include" folder for each experiment. I forgot to place the Python utilities library in the appropriate folder, and I used to include it from my local disk. As a result, the include paths are incorrect. However, it's still present in the main folder.

#### Note
Everything is a bit messy.
