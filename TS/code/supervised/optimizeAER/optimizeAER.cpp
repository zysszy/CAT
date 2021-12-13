#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <string>
#include <cmath>
#include <ctime>
#include <algorithm>

using namespace std;

/************************************************
  version
************************************************/
void version()
{
	cout << "-------------------------------------------\n"
		    " optimizeAER v0.1\n"
			" 2014/08/26 - 2014/08/26\n"
			" (c) 2014 Yang Liu\n"
			"-------------------------------------------\n";
}

/************************************************
  main function
************************************************/
int main(int argc, char** argv)
{
	// display version
	version();

	if (argc != 5)
	{
		cerr << "Usage: optimizeAER initFile candsFile featsFile lambdaFile\n";
		exit(1);
	}

	ofstream out(argv[4]);

	vector<vector<double> > ALLDATA;  // feats.opt
	vector<vector<double> > CANDS;    // cands.opt
	vector<vector<double> > INIT;     // init.opt
	int i, j;

	ifstream in1(argv[3]);

	if (!in1)
	{
		cerr << "\nERROR: cannot open file \""
			 << argv[3]
			 << "\"!\n";

		exit(1);
	}

	string line;

	while (getline(in1, line))
	{
		vector<double> v;
		istringstream buf(line.c_str());
		string w;

		while (buf >> w)
		{
			double temp = atof(w.c_str());
			v.push_back(temp);
		}

		ALLDATA.push_back(v);
	}

	ifstream in2(argv[2]);

	if (!in2)
	{
		cerr << "\nERROR: cannot open file \""
			 << argv[2]
			 << "\"!\n";

		exit(1);
	}

	while (getline(in2, line))
	{
		vector<double> v;
		istringstream buf(line.c_str());
		string w;

		while (buf >> w)
		{
			double temp = atof(w.c_str());
			v.push_back(temp);
		}

		CANDS.push_back(v);
	}

	ifstream in3(argv[1]);

	if (!in3)
	{
		cerr << "\nERROR: cannot open file \""
			 << argv[1]
			 << "\"!\n";

		exit(1);
	}

	while (getline(in3, line))
	{
		vector<double> v;
		istringstream buf(line.c_str());
		string w;

		while (buf >> w)
		{
			double temp = atof(w.c_str());
			v.push_back(temp);
		}

		INIT.push_back(v);
	}

	vector<pair<int, int> > startEndIndices;  
	int startIndex = 1,
		endIndex = 0;

	for (i = 0; i < (int)CANDS.size(); i++)
	{
		endIndex = startIndex + CANDS[i].back() - 1;
		startEndIndices.push_back(make_pair(startIndex, endIndex));
		startIndex = endIndex + 1;
	}

	int nlines = INIT.size(),  
		NF = INIT[0].size();   

	vector<vector<double> > ALLFEAT,  
		                    ALLSCORES;  

	for (i = 0; i < (int)ALLDATA.size(); i++)
	{
		vector<double> v1, v2;

		for (int j = 0; j < (int)ALLDATA[i].size(); j++)
		{
			if (j < NF)
			{
				v1.push_back(ALLDATA[i][j]);
			}
			else
			{
				v2.push_back(ALLDATA[i][j]);
			}
		}

		ALLFEAT.push_back(v1);
		ALLSCORES.push_back(v2);
	}
	
	ALLDATA.clear();

	vector<double> leftBound = INIT[0],  
		           rightBound = INIT[1],  
				   startingParams;  

	if (nlines == 3)
	{
		startingParams =INIT[2];
	}

	double ExpansionMargin = 0.0,
		   ExpansionFactor = 1.2,
		   NumRandomTests = 5,  
		   PermutationsEpsilon = 0.001,  
		   JUMP_PERC = 0.1,
		   numSentences = startEndIndices.size();  


	vector<double> initialScore(ALLSCORES[0].size(), 0);

	for (i = 1; i <= numSentences; i++)
	{
		int sentStart = startEndIndices[i - 1].first;  

		for (j = 0; j < (int)ALLSCORES[sentStart - 1].size(); j++)
		{
			initialScore[j] += ALLSCORES[sentStart - 1][j];
		}
	}

	double match_sure = initialScore[0],
		   match_possible = initialScore[1],
		   actual = initialScore[2],
		   sure = initialScore[3];

	double newError = 1.0 - (match_sure + match_possible) / (actual + sure);

	cout << "Initial AER: "
		 << newError
		 << endl;

	vector<vector<double> > FullResults;  

	srand((unsigned)time(NULL));  

	for (int topLevel = 1; topLevel <= NumRandomTests; topLevel++)
	{
		vector<double> LAMBDAS(NF, 0);  
		
		if (topLevel == 1 && nlines == 3)
		{
			LAMBDAS = startingParams;
		}
		else
		{
			for (i = 0; i < NF; i++)
			{
				double r = (double)rand() / RAND_MAX;
				LAMBDAS[i] = (leftBound[i] + ExpansionMargin) + 
					         ((rightBound[i] - ExpansionMargin) - (leftBound[i] + ExpansionMargin)) *
							 r;
			}
		}

		for (i = 0; i < NF; i++)
		{
			double sum = 0;

			for (j = 0; j < (int)ALLFEAT.size(); j++)
			{
				sum += ALLFEAT[j][i];
			}

			double ave = sum / ALLFEAT.size();
			sum = 0;

			for (j = 0; j < (int)ALLFEAT.size(); j++)
			{
				sum += pow(ave - ALLFEAT[j][i], 2);
			}

			if (sum == 0)
			{
				LAMBDAS[i] = 0;
			}
		}

		cout << "Start Values: ";

		for (i = 0; i < (int)LAMBDAS.size(); i++)
		{
			cout << LAMBDAS[i]
				 << " ";
		}

		cout << endl;

		vector<int> activeVec;

		for (i = 0; i < (int)LAMBDAS.size(); i++)
		{
			if (LAMBDAS[i] == 0)
			{
				activeVec.push_back(0);
			}
			else
			{
				activeVec.push_back(1);
			}
		}

		vector<double> lastValue(LAMBDAS.size(), -1);
		double savedError = 1.0,
			   lastError = 1.0;
		int	numIter = 0,  
			ConvergedLimit = 3,  
			converged = 0,  
			IterationLimit = 10;  
		vector<int> oRange;

		for (i = 1; i <= NF; i++)
		{
			oRange.push_back(i);
		}

		while (numIter < IterationLimit && converged <= ConvergedLimit)
		{
			numIter++;

			cout << "Random Test "
				 << topLevel
				 << " | Iteration "
				 << numIter
				 << endl;

			lastValue = LAMBDAS;

			vector<vector<double> > errorGain;  
			vector<double> v(NF, 0);
			errorGain.push_back(v);  
			errorGain.push_back(v);  

			for (int dim = 0; dim < (int)oRange.size(); dim++)
			{
				int optDim = oRange[dim];  
				double savedLambda = LAMBDAS[optDim - 1];

				LAMBDAS[optDim - 1] = 0;  

				vector<vector<double> > totalErrorDelta; 

				for (int s = 1; s <= numSentences; s++)
				{
					int sentStart = startEndIndices[s - 1].first,
						sentEnd = startEndIndices[s - 1].second;

					vector<vector<double> > FEAT,  
						                    SCORES;  

					for (int k = sentStart; k <= sentEnd; k++)
					{
						FEAT.push_back(ALLFEAT[k - 1]);
						SCORES.push_back(ALLSCORES[k - 1]);
					}

					if (FEAT.empty() || SCORES.empty())
					{
						continue;
					}

					vector<double> a,
						           slopes;  
					int x;

					for (x = 0; x < (int)FEAT.size(); x++)
					{
						double temp = 0;

						for (int y = 0; y < (int)FEAT[x].size(); y++)
						{
							temp += FEAT[x][y] * LAMBDAS[y];

							if (optDim - 1 == y)
							{
								slopes.push_back(FEAT[x][y]);
							}
						}

						a.push_back(temp);
					}

					double minLeftValue,  
						   minRightValue;  
					int	minLeftCandidate,  
						minRightCandidate;  

					for (x = 0; x < (int)a.size(); x++)
					{
						double value1 = a[x] + slopes[x] * leftBound[optDim - 1],
							   value2 = a[x] + slopes[x] * rightBound[optDim - 1];

						if (x == 0)
						{
							minLeftValue = value1;
							minLeftCandidate = x;
							minRightValue = value2;
							minRightCandidate = x;
						}
						else
						{
							if (value1 < minLeftValue)
							{
								minLeftValue = value1;
								minLeftCandidate = x;
							}

							if (value2 < minRightValue)
							{
								minRightValue = value2;
								minRightCandidate = x;
							}
						}
					}

					double slopeOfLeftCandidate = slopes[minLeftCandidate],
						   slopeOfRightCandidate = slopes[minRightCandidate];

					vector<int> relevantIndices;  

					for (x = 0; x < (int)slopes.size(); x++)
					{
						if (slopes[x] < slopeOfLeftCandidate &&
							slopes[x] >= slopeOfRightCandidate)
						{
							relevantIndices.push_back(x);
						}
					}

					int numIntersects = 1;
					vector<pair<double, double> > intersectPoints;  
					intersectPoints.push_back(make_pair(leftBound[optDim - 1], minLeftValue));

					vector<vector<double> > errorDeltas;
					vector<double> v;
					v.push_back(intersectPoints[0].first);

					for (x = 0; x < (int)SCORES[minLeftCandidate].size(); x++)
					{
						v.push_back(SCORES[minLeftCandidate][x]);
					}

					errorDeltas.push_back(v);

					vector<double> workingError = SCORES[minLeftCandidate];

					while (relevantIndices.size() > 0)
					{
						double bestL = rightBound[optDim - 1],
							   bestValue = minRightValue,
							   bestSlope = slopeOfLeftCandidate,
							   newMinCandidate = minLeftCandidate;

						for (int c = 0; c < (int)relevantIndices.size(); c++)
						{
							int x = relevantIndices[c];

							double top = a[x] - a[minLeftCandidate],
								   bottom = slopes[minLeftCandidate] - slopes[x];

							if (bottom != 0)
							{
								double newL = top / bottom;  

								if (newL < bestL)
								{
									double candidateSlope = slopes[x];
									bestL = newL;
									bestValue = a[x] + newL * candidateSlope;
									newMinCandidate = c;
									bestSlope = candidateSlope;
								}

								if (newL == bestL &&
									slopes[x] < bestSlope)
								{
									double candidateSlope = slopes[x];
									bestL = newL;
									bestValue = a[x] + newL * candidateSlope;
									newMinCandidate = c;
									bestSlope = candidateSlope;
								}
							}
						}  

						numIntersects++;
						intersectPoints.push_back(make_pair(bestL, bestValue));

						vector<double> newSlopes,
							           newA;
						vector<vector<double> > newSCORES;
						int index;

						for (index = 0; index < (int)relevantIndices.size(); index++)
						{
							newSlopes.push_back(slopes[relevantIndices[index]]);
							newA.push_back(a[relevantIndices[index]]);
						}

						for (index = 0; index < (int)SCORES.size(); index++)
						{
							bool flag = false;

							for (int i = 0; i < (int)relevantIndices.size(); i++)
							{
								if (index == relevantIndices[i])
								{
									flag = true;
									break;
								}
							}

							if (flag)
							{
								newSCORES.push_back(SCORES[index]);
							}
						}

						slopes = newSlopes;
						a = newA;
						SCORES = newSCORES;
						
						minLeftCandidate = newMinCandidate;
						slopeOfLeftCandidate = slopes[minLeftCandidate];

						vector<double> errorD;
						int k;

						for (k = 0; k < (int)SCORES[minLeftCandidate].size(); k++)
						{
							double temp = SCORES[minLeftCandidate][k] - workingError[k];
							errorD.push_back(temp);
						}

						vector<double> v;
						v.push_back(bestL);

						for (k = 0; k < (int)errorD.size(); k++)
						{
							v.push_back(errorD[k]);
						}

						errorDeltas.push_back(v);
						workingError = SCORES[minLeftCandidate];

						int oldSize = relevantIndices.size();
						relevantIndices.clear();

						for (int x = 0; x < (int)slopes.size(); x++)
						{
							if (slopes[x] < slopeOfLeftCandidate &&
								slopes[x] >= slopeOfRightCandidate)
							{
								relevantIndices.push_back(x);
							}
						}

						if (relevantIndices.size() == oldSize)
						{
							relevantIndices.pop_back();
						}
					}  

					vector<pair<double, double> > newIntersectPoints; 
					vector<vector<double> > newErrorDeltas;
					
					int k;

					for (k = 0; k < numIntersects; k++)
					{
						newIntersectPoints.push_back(intersectPoints[k]);
						newErrorDeltas.push_back(errorDeltas[k]);
					}
					
					intersectPoints = newIntersectPoints;
					errorDeltas = newErrorDeltas;
					
					for (k = 0; k < (int)errorDeltas.size(); k++)
					{
						totalErrorDelta.push_back(errorDeltas[k]);
					}
				}  

				sort(totalErrorDelta.begin(), totalErrorDelta.end()); 

				double lenTotalErrorDelta = totalErrorDelta.size(),
					   errorLineLen = totalErrorDelta[0].size();
				vector<double> zeroline(errorLineLen - 1, 0),
					           tempV;
				tempV.push_back(rightBound[optDim - 1]);

				for (int i = 1; i <= errorLineLen - 1; i++)
				{
					tempV.push_back(0);
				}

				totalErrorDelta.push_back(tempV);

				vector<double> totalBaseError = zeroline;
				double lowestError = 1.0,
					   lowestLambda = leftBound[optDim - 1];

				for (int bIndex = 1; bIndex <= lenTotalErrorDelta; bIndex++)
				{
					bool nonZero = false;
					vector<double> incrementalError;

					for (int k = 1; k < errorLineLen; k++)
					{
						incrementalError.push_back(totalErrorDelta[bIndex - 1][k]);

						if (totalErrorDelta[bIndex - 1][k] != 0)
						{
							nonZero = true;
						}
					}

					if (nonZero)  
					{
						for (int i = 0; i < (int)incrementalError.size(); i++)
						{
							totalBaseError[i] += incrementalError[i];
						}

						if (totalErrorDelta[bIndex - 1][0] !=
							totalErrorDelta[bIndex][0])
						{
							double match_sure = totalBaseError[0],
								   match_possible = totalBaseError[1],
								   actual = totalBaseError[2],
								   sure = totalBaseError[3];

							if (actual + sure == 0)
							{
								continue;
							}

							double newError = 1.0 - (match_sure + match_possible) / (actual + sure);

							if (newError < lowestError)
							{
								lowestError = newError;
								lowestLambda = (totalErrorDelta[bIndex - 1][0] +
									            totalErrorDelta[bIndex][0]) / 2.0;
							}
						}
					}
				}  

				LAMBDAS[optDim - 1] = savedLambda;

				if (activeVec[optDim - 1])
				{
					errorGain[0][optDim - 1] = lowestLambda;
					errorGain[1][optDim - 1] = lowestError;
				}
				else
				{
					errorGain[0][optDim - 1] = 0;
					errorGain[1][optDim - 1] = 1;
				}
						
				savedError = lowestError;
			}  

			double minError,
				   minErrorDim;

			for (int i = 0; i < (int)errorGain[1].size(); i++)
			{
				if (i == 0)
				{
					minError = errorGain[1][i];
					minErrorDim = i + 1;
				}
				else if (errorGain[1][i] < minError)
				{
					minError = errorGain[1][i];
					minErrorDim = i + 1;
				}
			}

			if (minError <= savedError)
			{
				LAMBDAS[minErrorDim - 1] = errorGain[0][minErrorDim - 1];

				cout << "UsedDim ("
					 << minErrorDim
					 << ") ";

				for (int i = 0; i < (int)LAMBDAS.size(); i++)
				{
					if (i == minErrorDim - 1)
					{
						cout << "<"
							 << LAMBDAS[i]
							 << "> ";
					}
					else
					{
						cout << LAMBDAS[i]
							 << " ";
					}
				}

				cout << minError << endl;

				savedError = minError;
			}

			if (fabs(savedError - lastError) < PermutationsEpsilon)
			{
				converged++;

				cout << "Increment Convergence\n";

				vector<double> toAdd = LAMBDAS;
				toAdd.push_back(savedError);
				FullResults.push_back(toAdd);

				vector<double> LAMBDAS_INC(NF, 0);
				int i;

				for (i = 0; i < NF; i++)
				{
					double r = (double)rand() / RAND_MAX;
					LAMBDAS_INC[i] = ((leftBound[i]) + ((rightBound[i]) - (leftBound[i])) * r) -
									  (leftBound[i] + rightBound[i]) / 2;
				}

				for (i = 0; i < NF; i++)
				{
					LAMBDAS_INC[i] *= JUMP_PERC;
				}

				for (i = 0; i < (int)LAMBDAS.size(); i++)
				{
					if (LAMBDAS[i] + LAMBDAS_INC[i] > rightBound[i])
					{
						LAMBDAS[i] = rightBound[i];
					}
					else
					{
						LAMBDAS[i] += LAMBDAS_INC[i];
					}

					if (LAMBDAS[i] < leftBound[i])
					{
						LAMBDAS[i] = leftBound[i];
					}

					double sum = 0;

					for (j = 0; j < (int)ALLFEAT.size(); j++)
					{
						sum += ALLFEAT[j][i];
					}

					double ave = sum / ALLFEAT.size();
					sum = 0;

					for (j = 0; j < (int)ALLFEAT.size(); j++)
					{
						sum += pow(ave - ALLFEAT[j][i], 2);
					}

					if (sum == 0)
					{
						LAMBDAS[i] = 0;
					}
				}
			}
			else
			{
			}

			lastError = savedError;
		}  

		vector<double> toAdd = LAMBDAS;
		toAdd.push_back(savedError);
		FullResults.push_back(toAdd);

		double mval,
			   mRow;

		for (int k = 0; k < (int)FullResults.size(); k++)
		{
			if (k == 0)
			{
				mval = FullResults[k].back();
				mRow = k;
			}
			else if (FullResults[k].back() < mval)
			{
				mval = FullResults[k].back();
				mRow = k;
			}
		}

		cout << "\nIntermediateParameters ";

		for (int k = 0; k < (int)LAMBDAS.size(); k++)
		{
			cout << FullResults[mRow][k]
				 << " ";
		}

		cout << "\nIntermediate AER "
			 << mval
			 << "\n\n";
	}

	double mval,
		   mRow;
	int k;

	for (k = 0; k < (int)FullResults.size(); k++)
	{
		if (k == 0)
		{
			mval = FullResults[k].back();
			mRow = k;
		}
		else if (FullResults[k].back() < mval)
		{
			mval = FullResults[k].back();
			mRow = k;
		}
	}

	cout << "\nFinalParameters ";

	for (k = 0; k < NF; k++)
	{
		cout << FullResults[mRow][k]
			 << " ";
	}

	cout << "\nFinal AER "
		 << mval
		 << endl;

	double sum = 0;

	for (k = 0; k < NF; k++)
	{
		sum += fabs(FullResults[mRow][k]);
	}

	for (k = 0; k < NF; k++)
	{
		FullResults[mRow][k] /= sum;
	}

	out << newError
		<< " -> "
		<< mval
		<< "\n";

	for (k = 0; k < NF; k++)
	{
		out << FullResults[mRow][k]
			<< " ";
	}

	return 0;
}
