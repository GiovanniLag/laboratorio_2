# Set the name of the Conda environment
$conda_env = "base"

# Build the command to activate the Conda environment and start a new terminal window
$cmd = "conda activate $conda_env; cd 'C:\Users\giova\Documents\uni\laboratorio_2'; powershell.exe"

# Start a new terminal window with the Conda environment activated
Start-Process powershell.exe -ArgumentList '-NoExit', '-Command', $cmd -WindowStyle Hidden

# Pause the terminal window
Read-Host -Prompt "Press Enter to close this window"