#pragma once

#include "aircraft.h"

struct node
{
	aircraft* aircraft;
	struct node* next;
};

void addAircraft(aircraft* aircraft, struct node** head);
void deleteAircraft(struct node* aircraftToDelete, struct node** head);
int checkSizeOfList(struct node* head);