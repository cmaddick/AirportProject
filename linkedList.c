#include <stdio.h>
#include "aircraft.h"
#include "linkedList.h"

void addAircraft(aircraft* aircraft, struct node** head)
{
	struct node* newNode = (struct node*)malloc(sizeof(struct node));
	newNode->aircraft = aircraft;
	newNode->next = NULL;

	if (*head == NULL)
	{
		*head = newNode;
	}
	else
	{
		struct node* current = *head;
		while (current->next)
		{
			current = current->next;
		}
		current->next = newNode;
	}
}

void deleteAircraft(struct node* aircraftToDelete, struct node** head)
{
	struct node* current = *head;
	struct node** oldHead;
	struct node* newHead;
	struct node* previous = NULL;

	while (current != NULL)
	{
		if (current == aircraftToDelete)
		{
			if (previous == NULL && current->next == NULL)
			{
				 // Delete something that is a single element list
				*head = NULL;
				current = NULL;

			}
			else if (previous == NULL)
			{
				// Delete something at the start of the list. Set the head pointer to the new start
				newHead = current->next;
				*head = newHead;
				current = NULL;
			}
			else if (current->next == NULL)
			{
				// Delete something at the end of the list
				previous->next = NULL;
				current = NULL;
			}
			else
			{
				// Delete something in the middle
				previous->next = current->next;
				current = NULL;
			}
		}
		else
		{
			previous = current;
			current = current->next;
		}
	}
}

int checkSizeOfList(struct node* head)
{
	int i = 0;
	struct node* current = NULL;

	current = head;

	while (current != NULL)
	{
		++i;
		current = current->next;
	}

	return i;
}