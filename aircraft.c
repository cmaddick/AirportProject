#include <stdio.h>
#include "aircraft.h"

aircraft* createAircraft(int currentTick, int groundOrAir)
{
	aircraft* newAircraft = (aircraft*)malloc(sizeof(aircraft));
	
	newAircraft->id = 1000 + (rand() % (int)(9999 - 1000 + 1));

	newAircraft->airline[0] = 65 + rand() % 26;
	newAircraft->airline[1] = 65 + rand() % 26;
	newAircraft->airline[2] = '\0';

	if (groundOrAir == 0)
	{
		// Air
		newAircraft->status = 0;
		newAircraft->ticksToAirport = 15 + rand() % 30;
		newAircraft->eta = currentTick + newAircraft->ticksToAirport;
		newAircraft->etd = 0;
		newAircraft->fuel = newAircraft->ticksToAirport + rand() % 40;
		newAircraft->ticksWaiting = 0;
		newAircraft->ticksToDepart = 0;
		newAircraft->numPassengers = 50 + rand() % 200;
	}
	else
	{
		// Ground
		newAircraft->status = 0;
		newAircraft->ticksToAirport = 0;
		newAircraft->ticksToDepart = 5;
		newAircraft->etd = currentTick + newAircraft->ticksToDepart;
		newAircraft->eta = 0;
		newAircraft->fuel = 0;
		newAircraft->ticksWaiting = 0;
		newAircraft->numPassengers = 50 + rand() % 200;
	}

	return newAircraft;
}