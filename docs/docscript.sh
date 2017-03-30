#!/bin/bash
RED='\033[0;31m'
NC='\033[0m' # No Color
echo -ne "${RED}Make sure you are in /docs of the project before running this script!${NC}\n"
read -n 1 -s -p "If you are press enter to continue else CTL+C to abort."
if ! [ -x "$(command -v doxygen)" ]; then
  echo -ne "\nDoxygen is not installed"
  echo -ne "\nInstalling Doxygen..."
  brew install doxygen
else
 echo -ne "\nDoxygen is installed." 
 #exit 1
fi
doxygen -g
(cat Doxyfile;echo "PROJECT_NAME=M2X library Docs";echo "SOURCE_BROWSER=YES";echo "INPUT=../ ../M2XStreamClient/";echo "EXTRACT_ALL=YES";echo "GENERATE_TREEVIEW=YES";echo "GENERATE_LATEX=NO";echo "CLASS_DIAGRAMS=NO" ) | doxygen -
