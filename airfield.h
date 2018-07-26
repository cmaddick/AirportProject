#pragma once

#include "runway.h"

typedef struct _airfield
{
	int status;
	int ticksLeftUntilEnabled;
	runway* runways[10];
	int numRunways;
} airfield;