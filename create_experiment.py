import os
import sys
import yaml
import nbformat
from nbformat.v4 import new_notebook, new_markdown_cell, new_code_cell

experiment_name = sys.argv[1]
path_to_data_structure = None

if len(sys.argv) > 2:
    path_to_data_structure = sys.argv[2]

# create main experiment directory
os.makedirs(experiment_name)

# create subdirectories
os.makedirs(os.path.join(experiment_name, 'data'))
os.makedirs(os.path.join(experiment_name, 'imgs'))
os.makedirs(os.path.join(experiment_name, 'fitting_root'))
os.makedirs(os.path.join(experiment_name, 'diario_di_bordo'))

# create notebook
nb = new_notebook()  # Create a new notebook object
frist_cell = new_markdown_cell("## " + experiment_name)  # Create a new Markdown cell
importstr = 'import numpy as np\nimport pandas as pd\nimport sys\nsys.path.append("D:/Progetti/LabProgram")\nfrom Routine import Routine\nfrom Funnel import Funnel \nimport lab_utilities as lu'
import_cell = new_code_cell(importstr)
nb['cells'] = [frist_cell, import_cell]  # Add the cell to the notebook object
# Save the notebook to a file
file_path = experiment_name + "/notebook_" + experiment_name + ".ipynb"
with open(file_path, 'w') as f:
    nbformat.write(nb, f)

# create subdirectories based on YAML file
if path_to_data_structure:
    with open(path_to_data_structure) as f:
        data_structure = yaml.load(f, Loader=yaml.FullLoader)

    for subdirectory in data_structure['data']:
        os.makedirs(os.path.join(experiment_name, 'data', subdirectory))

#create an md file for the diary
with open(experiment_name + "/diario_di_bordo/diario_di_bordo.md", 'w') as f:
    f.write("## " + experiment_name)

