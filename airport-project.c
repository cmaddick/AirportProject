#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <limits.h>
#include "airfield.h"
#include "airspace.h"
#include "linkedList.h"
#include "runway.h"
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

void generateLambdaTable(float* tableBuf, float lambda);
void generateCumulativeFreqTable(float* cumulativeTable, float* lambdaTable);
int numSpawnAircraft(float* cumulativeTable);
float factorial(int k);
int ticksHour(int ticks);
int ticksMinute(int ticks);
FILE* createFile(char* filename);
void appendToFile(FILE* fPtr, char* filename, aircraft* aircraft, int msgType, int currentTick);
int aircraftAlreadyExists(aircraft* aircraftToFind, struct node* aircraftInAir, airfield* airport);


// Main file containing main loop
int main(int argc, char* argv[])
{
	int tickLimit = 0;
	int numRunways = 0;
	float lambdaD1 = 0.0;
	float lambdaD2 = 0.0;
	float airLambdaTable[20] = { 0 };
	float airCumulativeTable[20] = { 0 };
	float groundLambdaTable[20] = { 0 };
	float groundCumulativeTable[20] = { 0 };
	char filename[35] = { 0 };
	char tmpFilename[30];
	char fileExtension[5] = ".txt";
	char studentID[10] = "100408079";
	FILE* fPtr;
	int outputEveryTimeUnit = 0;
	int enableStepThrough = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int numAircraft = 0;
	aircraft* tmpCraft = (aircraft*)malloc(sizeof(aircraft));
	struct node* planesInAir = (struct node*)malloc(sizeof(struct node));
	struct node* tmpPlanesInAir = NULL;
	struct node* tmpTakeOffQueue = NULL;
	struct node* tmpLandingQueue = NULL;
	runway* tmpRunway;
	airfield airport;

	if (argc >= 6)
	{
		numRunways = strtoul(argv[1], NULL, 10);
		tickLimit = strtoul(argv[2], NULL, 10);
		lambdaD1 = strtof(argv[3], NULL);
		lambdaD2 = strtof(argv[4], NULL);
		strcpy(&tmpFilename, argv[5]);
		
		if (argc == 8)
		{
			enableStepThrough = strtoul(argv[6], NULL, 10);
			outputEveryTimeUnit = strtoul(argv[7], NULL, 10);			
		}

		// Create file
		strcat(&filename, &studentID);
		strcat(&filename, &tmpFilename);
		strcat(&filename, &fileExtension);
		fPtr = createFile(&filename);

		// Generate tables before main loop start
		generateLambdaTable(&airLambdaTable, lambdaD1);
		generateLambdaTable(&groundLambdaTable, lambdaD2);
		generateCumulativeFreqTable(&airCumulativeTable, &airLambdaTable);
		generateCumulativeFreqTable(&groundCumulativeTable, &groundLambdaTable);

		// Seed random number generator
		srand(time(NULL));

		// Initialisation
		memset(planesInAir, NULL, sizeof(struct node));
		memset(tmpCraft, NULL, sizeof(aircraft));
		memset(&airport, NULL, sizeof(airfield));
		planesInAir = NULL;

		airport.numRunways = numRunways;
		airport.ticksLeftUntilEnabled = 0;
		airport.status = 0;

		for (i = 0; i < airport.numRunways; i++)
		{
			airport.runways[i] = (runway*)malloc(sizeof(runway));
			airport.runways[i]->busy = 0;
			airport.runways[i]->takeOffQueue = NULL;
			airport.runways[i]->landingQueue = NULL;
			airport.runways[i]->takeOffQueueSize = 0;
			airport.runways[i]->landingQueueSize = 0;
		}


		// MAIN LOOP
		for (i = 0; i < tickLimit; i++)
		{
			if (i != 0)
			{
				// Decrement fuel, decrease time to airport
				tmpPlanesInAir = planesInAir;
				while (tmpPlanesInAir != NULL)
				{
					tmpPlanesInAir->aircraft->fuel = tmpPlanesInAir->aircraft->fuel - 1;
					if (tmpPlanesInAir->aircraft->ticksToAirport != 0)
					{
						--tmpPlanesInAir->aircraft->ticksToAirport;
					}
					tmpPlanesInAir = tmpPlanesInAir->next;
				}

				for (j = 0; j < airport.numRunways; j++)
				{
					tmpLandingQueue = airport.runways[j]->landingQueue;
					while (tmpLandingQueue != NULL)
					{
						--tmpLandingQueue->aircraft->fuel;
						++tmpLandingQueue->aircraft->ticksWaiting;
						tmpLandingQueue = tmpLandingQueue->next;
					}
				}

				// Increment time waited for planes waiting to take off (if delayed)
				for (j = 0; j < airport.numRunways; j++)
				{
					tmpTakeOffQueue = airport.runways[j]->takeOffQueue;
					while (tmpTakeOffQueue != NULL)
					{
						if (tmpTakeOffQueue->aircraft->etd <= i)
						{
							++tmpTakeOffQueue->aircraft->ticksWaiting;
						}
						tmpTakeOffQueue = tmpTakeOffQueue->next;
					}
				}

				// Decrement time to being enabled if airport is disabled
				if (airport.ticksLeftUntilEnabled > 0)
				{
					airport.ticksLeftUntilEnabled = airport.ticksLeftUntilEnabled - 1;

					if (airport.ticksLeftUntilEnabled == 0)
					{
						airport.status = 0;
					}
				}

				// Assign new planes at airport to landing runways
				tmpPlanesInAir = planesInAir;
				while (tmpPlanesInAir != NULL)
				{
					if (tmpPlanesInAir->aircraft->ticksToAirport == 0)
					{
						int clearestRunway = NULL;
						int clearestRunwaySize = INT_MAX;
						for (j = 0; j < airport.numRunways; j++)
						{
							tmpRunway = airport.runways[j];
							if (checkSizeOfList(tmpRunway->landingQueue) < clearestRunwaySize)
							{
								clearestRunwaySize = checkSizeOfList(tmpRunway->landingQueue);
								clearestRunway = j;
							}
						}
						addAircraft(tmpPlanesInAir->aircraft, &airport.runways[clearestRunway]->landingQueue);
						deleteAircraft(tmpPlanesInAir, &planesInAir);
					}
					tmpPlanesInAir = tmpPlanesInAir->next;
				}

				// Display verbose detail if enabled
				if (outputEveryTimeUnit == 1)
				{
					printf("\n============= %d hrs, %d mins (tick %d) =============\n\n", ticksHour(i), ticksMinute(i), i);

					for (j = 0; j < airport.numRunways; j++)
					{
						printf("\n-------- RUNWAY %d --------\n\n", j);
						printf("- TAKE OFF QUEUE -\n\n");

						// Take offs
						tmpTakeOffQueue = airport.runways[j]->takeOffQueue;
						while (tmpTakeOffQueue != NULL)
						{
							printf("ID: %s %d DEPARTS: %d %d TICKS WAITED (delayed): %d\n", tmpTakeOffQueue->aircraft->airline,
								tmpTakeOffQueue->aircraft->id, ticksHour(tmpTakeOffQueue->aircraft->etd), ticksMinute(tmpTakeOffQueue->aircraft->etd),
								tmpTakeOffQueue->aircraft->ticksWaiting);

							tmpTakeOffQueue = tmpTakeOffQueue->next;
						}

						printf("\n- LANDING QUEUE -\n\n");

						// Landings
						tmpLandingQueue = airport.runways[j]->landingQueue;
						while (tmpLandingQueue != NULL)
						{
							printf("ID: %s %d TIME WAITING (ticks): %d FUEL REMAINING: %d\n", tmpLandingQueue->aircraft->airline,
								tmpLandingQueue->aircraft->id, tmpLandingQueue->aircraft->ticksWaiting, tmpLandingQueue->aircraft->fuel);
							tmpLandingQueue = tmpLandingQueue->next;
						}
					}

					if (enableStepThrough == 1)
					{
						getchar();
					}
				}

				// Determine if planes should crash
				for (j = 0; j < airport.numRunways; j++)
				{
					tmpLandingQueue = airport.runways[j]->landingQueue;
					while (tmpLandingQueue != NULL)
					{
						if (tmpLandingQueue->aircraft->fuel < 0)
						{
							appendToFile(fPtr, &filename, tmpLandingQueue->aircraft, 2, i);
							deleteAircraft(tmpLandingQueue, &airport.runways[j]->landingQueue);
							airport.status = 1;
							airport.ticksLeftUntilEnabled = 15;
						}
						tmpLandingQueue = tmpLandingQueue->next;
					}
				}

				// Emergency landings
				for (j = 0; j < airport.numRunways; j++)
				{
					tmpLandingQueue = airport.runways[j]->landingQueue;
					while (tmpLandingQueue != NULL)
					{
						if (airport.status == 0)
						{
							if (tmpLandingQueue->aircraft->fuel == 0)
							{
								airport.status = 1;
								airport.ticksLeftUntilEnabled = 1;
								appendToFile(fPtr, &filename, tmpLandingQueue->aircraft, 3, i);
								deleteAircraft(tmpLandingQueue, &airport.runways[j]->landingQueue);
							}
						}
						tmpLandingQueue = tmpLandingQueue->next;
					}
				}

				// Routine take offs and landings
				if (airport.status == 0)
				{
					struct node* aircraftToLand = NULL;
					struct node* aircraftToTakeOff = NULL;
					int currentLeastFuel = INT_MAX;
					int currentMostDelayed = INT_MIN;
					int priority;


					for (j = 0; j < airport.numRunways; j++)
					{
						currentLeastFuel = INT_MAX;
						currentMostDelayed = INT_MIN;
						priority = 0;

						if (airport.runways[j]->busy == 0)
						{
							// Landing
							aircraftToLand == NULL;
							aircraftToTakeOff == NULL;
							tmpLandingQueue = airport.runways[j]->landingQueue;
							tmpTakeOffQueue = airport.runways[j]->takeOffQueue;

							while (tmpLandingQueue != NULL)
							{
								// Find aircraft in queue that is close to running out of fuel. Land them
								// Else if judge depending if a plane needs to take off now.

								if (tmpLandingQueue->aircraft->fuel <= 3)
								{
									aircraftToLand = tmpLandingQueue;
									priority = 1;
								}
								else if (tmpTakeOffQueue == NULL)
								{
									aircraftToLand = tmpLandingQueue;
								}

								tmpLandingQueue = tmpLandingQueue->next;
							}

							while (tmpTakeOffQueue != NULL)
							{
								if (tmpTakeOffQueue->aircraft->etd <= i)
								{
									if (tmpTakeOffQueue->aircraft->ticksWaiting >= 0)
									{
										if (tmpTakeOffQueue->aircraft->ticksWaiting > currentMostDelayed)
										{
											currentMostDelayed = tmpTakeOffQueue->aircraft->ticksWaiting;
											aircraftToTakeOff = tmpTakeOffQueue;
										}
									}
								}
								tmpTakeOffQueue = tmpTakeOffQueue->next;
							}

							// Check if planes are availible to land or take off. Given choice, land.
							if (aircraftToLand != NULL && priority == 1)
							{
								deleteAircraft(aircraftToLand, &airport.runways[j]->landingQueue);
							}
							else if (aircraftToTakeOff != NULL)
							{
								appendToFile(fPtr, &filename, aircraftToTakeOff->aircraft, 0, i);
								deleteAircraft(aircraftToTakeOff, &airport.runways[j]->takeOffQueue);
							}
							else
							{
								deleteAircraft(aircraftToLand, &airport.runways[j]->landingQueue);
							}

						}

					}
				}

				// Generate aircraft every 3 ticks
				if (i % 3 == 0)
				{
					numAircraft = numSpawnAircraft(&airCumulativeTable);
					for (j = 0; j < numAircraft; j++)
					{
						tmpCraft = createAircraft(i, 0);
						if (aircraftAlreadyExists(tmpCraft, planesInAir, &airport) == 0)
						{
							addAircraft(tmpCraft, &planesInAir);
							appendToFile(fPtr, &filename, tmpCraft, 1, i);
						}
						else
						{
							appendToFile(fPtr, &filename, tmpCraft, 5, i);
						}
					}

					numAircraft = numSpawnAircraft(&groundCumulativeTable);
					for (j = 0; j < numAircraft; j++)
					{
						tmpCraft = createAircraft(i, 1);
						for (j = 0; j < numAircraft; j++)
						{
							tmpCraft = createAircraft(i, 1);
							int clearestRunway = NULL;
							int clearestRunwaySize = INT_MAX;
							for (k = 0; k < airport.numRunways; k++)
							{
								tmpRunway = airport.runways[k];
								if (checkSizeOfList(tmpRunway->takeOffQueue) < clearestRunwaySize)
								{
									clearestRunwaySize = checkSizeOfList(tmpRunway->takeOffQueue);
									clearestRunway = k;
								}
							}
							addAircraft(tmpCraft, &airport.runways[clearestRunway]->takeOffQueue);
						}
					}
				}
			}
			else
			{
				// Generate aircraft
				numAircraft = numSpawnAircraft(&airCumulativeTable);
				for (j = 0; j < numAircraft; j++)
				{
					tmpCraft = createAircraft(i, 0);
					if (aircraftAlreadyExists(tmpCraft, planesInAir, &airport) == 0)
					{
						addAircraft(tmpCraft, &planesInAir);
						appendToFile(fPtr, &filename, tmpCraft, 1, i);
					}
					else
					{
						appendToFile(fPtr, &filename, tmpCraft, 5, i);
					}
				}

				numAircraft = numSpawnAircraft(&groundCumulativeTable);
				for (j = 0; j < numAircraft; j++)
				{
					tmpCraft = createAircraft(i, 1);
					int clearestRunway = NULL;
					int clearestRunwaySize = INT_MAX;
					for (k = 0; k < airport.numRunways; k++)
					{
						tmpRunway = airport.runways[k];
						if (checkSizeOfList(tmpRunway->takeOffQueue) < clearestRunwaySize)
						{
							clearestRunwaySize = checkSizeOfList(tmpRunway->takeOffQueue);
							clearestRunway = k;
						}
					}
					addAircraft(tmpCraft, &airport.runways[clearestRunway]->takeOffQueue);
				}
			}
			appendToFile(fPtr, &filename, NULL, 5, NULL);
		}
	}
}

void generateLambdaTable(float* tableBuf, float lambda)
{
	// Fills a float array buffer with probability tables for the given lambda value

	float probability = 1.0;;
	int k = 0;

	while (probability >= 0.0001)
	{
		probability = (pow(lambda, k) * exp(0 - lambda)) / factorial(k);
		if (probability >= 0.0001)
		{
			tableBuf[k] = probability;
		}
		k++;
	}
}

void generateCumulativeFreqTable(float* cumulativeTable, float* lambdaTable)
{
	// Generates the cumulative frequency table from the lambda table
	// This is needed to easily generate a number of planes based on that lambda table with a random float value

	float runningTotal = 0.0;
	float probability = lambdaTable[0];
	int i = 0;

	while (lambdaTable[i] != 0.0)
	{
		runningTotal = runningTotal + lambdaTable[i];
		cumulativeTable[i] = runningTotal;
		i++;
	}
}

int numSpawnAircraft(float* cumulativeTable)
{
	// Gives a random number of aircraft to be spawned based on the given cumulative lambda table
	// Take the number of planes which the random value is closest to in the cumulative table

	float randomValue = 0.0;
	int i = 0;
	int closest = 0;
	float difference = 0.0;
	float closestMargin = FLT_MAX;

	randomValue = (double)rand() / (double)RAND_MAX;

	while (*cumulativeTable != 0.0)
	{
		difference = (float)fabs(*cumulativeTable - randomValue);

		if (difference < closestMargin)
		{
			closestMargin = difference;
			closest = i;
		}

		cumulativeTable++;
		i++;
	}

	return closest;
}

float factorial(int k)
{
	if (k == 0)
	{
		return 1;
	}

	return (k * factorial(k - 1));
}

int ticksHour(int ticks)
{
	return ticks / 60;
}

int ticksMinute(int ticks)
{
	return ticks % 60;
}

FILE* createFile(char* filename)
{
	FILE* fPtr;

	fPtr = fopen(filename, "w");

	if (fPtr == NULL)
	{
		printf("There was an error creating the file\n");
		exit(1);
	}

	return fPtr;
}

void appendToFile(FILE* fPtr, char* filename, aircraft* aircraft, int msgType, int currentTick)
{
	switch (msgType)
	{
	case 0:
		fprintf(fPtr, "TO %s %d %d %d %d\n", aircraft->airline, aircraft->id, ticksHour(aircraft->etd), ticksMinute(aircraft->etd),
			aircraft->numPassengers);
		break;
	case 1:
		fprintf(fPtr, "EA %s %d %d %d %d %d %d\n", aircraft->airline, aircraft->id, ticksHour(aircraft->eta), ticksMinute(aircraft->eta),
			aircraft->numPassengers, ticksHour(aircraft->fuel + currentTick), ticksMinute(aircraft->fuel + currentTick));
		break;
	case 2:
		fprintf(fPtr, "CL %s %d %d %d %d %d %d\n", aircraft->airline, aircraft->id, ticksHour(aircraft->eta), ticksMinute(aircraft->eta),
			ticksHour(currentTick), ticksMinute(currentTick), aircraft->numPassengers);
		break;
	case 3:
		fprintf(fPtr, "EL %s %d %d %d %d %d %d\n", aircraft->airline, aircraft->id, ticksHour(aircraft->eta), ticksMinute(aircraft->eta),
			ticksHour(currentTick), ticksMinute(currentTick), aircraft->numPassengers);
		break;
	case 4:
		fprintf(fPtr, "ER %s %d\n", aircraft->airline, aircraft->id);
		break;
	case 5:
		fprintf(fPtr, "+\n");
		break;
	}
}

int aircraftAlreadyExists(aircraft* aircraftToFind, struct node* aircraftInAir, airfield* airport)
{
	int i;
	struct node* tmpAir = aircraftInAir;
	struct node* tmpTakeOffQueue;
	struct node* tmpLandingQueue;

	while (tmpAir != NULL)
	{
		if (tmpAir->aircraft->id == aircraftToFind->id)
		{
			return 1;
		}

		tmpAir = tmpAir->next;
	}

	for (i = 0; i < airport->numRunways; i++)
	{
		tmpTakeOffQueue = airport->runways[i]->takeOffQueue;
		tmpLandingQueue = airport->runways[i]->landingQueue;

		while (tmpTakeOffQueue != NULL)
		{
			if (tmpTakeOffQueue->aircraft->id == aircraftToFind->id)
			{
				return 1;
			}

			tmpTakeOffQueue = tmpTakeOffQueue->next;
		}

		while (tmpLandingQueue != NULL)
		{
			if (tmpLandingQueue->aircraft->id == aircraftToFind->id)
			{
				return 1;
			}

			tmpLandingQueue = tmpLandingQueue->next;
		}
	}

	return 0;
}