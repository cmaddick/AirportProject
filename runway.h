#pragma once

#include "aircraft.h"
#include "linkedList.h"

typedef struct _runway
{
	int busy;
	int takeOffQueueSize;
	int landingQueueSize;
	struct node* takeOffQueue;
	struct node* landingQueue;
} runway;