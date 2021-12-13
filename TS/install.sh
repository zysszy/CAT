#! /bin/bash

# create a directory to store binary executables
echo -n "Creating a directory to store binary executables ... "
mkdir bin
echo "done!"

# compiling
echo -n "Compiling TsinghuaAligner ... "
cd code/aligner/TsinghuaAligner
g++ -O2 -o TsinghuaAligner *.cpp
cp TsinghuaAligner ../../../bin
echo "done!"

echo -n "Compiling waEval ... "
cd ../../misc/waEval
g++ -O2 -o waEval *.cpp
cp waEval ../../../bin
echo "done!"

echo -n "Compiling convertNBestListFormat ... "
cd ../../supervised/convertNBestListFormat
g++ -O2 -o convertNBestListFormat *.cpp
cp convertNBestListFormat ../../../bin
echo "done!"

echo -n "Compiling genIni ... "
cd ../genIni
g++ -O2 -o genIni *.cpp
cp genIni ../../../bin
echo "done!"

echo -n "Compiling mergeNBestList ... "
cd ../mergeNBestList
g++ -O2 -o mergeNBestList *.cpp
cp mergeNBestList ../../../bin
echo "done!"

echo -n "Compiling optimizeAER ... "
cd ../optimizeAER
g++ -O2 -o optimizeAER *.cpp
cp optimizeAER ../../../bin
echo "done!"

echo -n "Chmoding genNoise ... "
cd ../../unsupervised/genNoise
chmod +x genNoise.py
cp genNoise.py ../../../bin
echo "done!"

echo -n "Compiling trainer ... "
cd ../trainer
g++ -O2 -o trainer *.cpp
cp trainer ../../../bin
echo "done!"

echo -n "Chomoding scripts ... "
cd ../../../scripts
chmod +x *.py
echo "done!"

echo "The system is installed successfully."
