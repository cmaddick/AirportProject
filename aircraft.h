#pragma once

typedef struct _aircraft
{
	int id;
	char airline[3];
	int status;
	int fuel;
	int eta;
	int etd;
	int numPassengers;
	int ticksToAirport;
	int ticksWaiting;
	int ticksToDepart;
} aircraft;

aircraft* createAircraft(int currentTick, int groundOrAir);