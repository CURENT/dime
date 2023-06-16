# "mex" command, usually just "mex"
MEX ?= mex

# Matlab user path, varies depending on the system:
# * %UserProfile%/Documents/MATLAB on Windows
# * $home/Documents/MATLAB on macOS and Linux
MATLABPATH ?= ${HOME}/Documents/MATLAB

# File extension for binaries compiled with mex, varies depending on the system:
# * mexw64 for Windows
# * mexmaci64 for macOS
# * mexa64 for Linux
EXT = mexa64
