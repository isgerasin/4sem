#include <malloc.h>
#include <stdio.h>
#include "list.h"

ListElem* ListElemCtor(ListData data)
{
	ListElem* this = NULL;
	this = (ListElem*) calloc(1, sizeof(ListElem));
	if (this == NULL)
		return this;
	this->next = NULL;
	this->prev = NULL;
	this->data = data;
	return this;
}

int ListElemDtor(ListElem* this, void* contx)
{
	if (!this)
		return PTRERR;
	this->next = NULL;
	this->prev = NULL;
	this->data = BAD_DATA;
	free(this);
	return 0;
}

int ListElemOk(ListElem* this, void* contx)
{
	return !(this &&
			this->next &&
			this->prev);
}

int ListElemDump(ListElem* this, void* contx)
{
	if (!contx)
		return PTRERR;
	FILE* output = (FILE*) contx;
	fprintf(output, "=====ListElem Dump[%p]=======\n", this);
	
	if (ListElemOk(this, NULL) == 0)
		fprintf(output, "ok\n");
	else
		fprintf(output, "ERROR!!!\n=================\n");
	if (this == NULL)
		return PTRERR;
	fprintf(output, "\tdata = %i\n"
					"\tprev = %p\n"
					"\tnext = %p\n"
					"=============================\n",
					this->data, this->prev, this->next);
	return 0;
}

List* ListCtor()
{
	List* this = (List*) calloc(1, sizeof(List));
	if (this == NULL)
		return this;
	this->head = NULL;
	this->tail = NULL;
	this->numbers = 0;
	return this;
}

int ForEach(List* this, int (*act)(ListElem* elem, void* ctx), void* ctx)
{
	if (!(this && act))
		return PTRERR;

	ListElem* elem = this->head;
	ListElem* nextElem = NULL;
	while(this->numbers && nextElem != this->head)
	{
		nextElem = elem->next;
		int res = act(elem, ctx);
		if (res)
			return res;
		elem = nextElem;
	}
	return 0;
}

int ListDtor(List* this)
{
	if (!this)
		return PTRERR;
	int res = ForEach(this, &ListElemDtor, NULL);
	this->head = NULL;
	this->tail = NULL;
	this->numbers = 0;
	free(this);
	return res;
}

int ListOk(List* this)
{
	return this == NULL &&
		   ForEach(this, ListElemOk, NULL);
}

int ListDump(List* this, FILE* output)
{
	if (!output)
		return PTRERR;

	fprintf(output, "------List %p-------\n", this);

	if (ListOk(this) == 0)
		fprintf(output, "ok\n");
	else
		fprintf(output, "ERROR!!!\n------------------\n");
	if (this == NULL)
		return PTRERR;
	fprintf(output, "\thead = %p\n"
					"\ttail = %p\n"
					"\tnumbers = %lu\n{\n",
					this->head, this->tail, this->numbers);
	int res = ForEach(this, ListElemDump, (void*) output);
	fprintf(output, "}\n---------------------\n");

	return res;
}

int ListAddAfter(List* this, ListElem* elem, ListElem* after)
{
	if (!(this && elem && after))
		return PTRERR;
	ListElem* before = after->next;
	elem->next = before;
	elem->prev = after;
	after->next = elem;
	before->prev = elem;
	this->numbers++;
	return 0;
}

int ListAddTail(List* this, ListElem* elem)
{
	if (!(this && elem))
		return PTRERR;
	if(!this->numbers)
	{
		this->head = elem;
		elem->next = elem;
		elem->prev = elem;
		this->numbers++;
	}
	else
		ListAddAfter(this, elem, this->tail);
	this->tail = elem;

	return 0;
}

int ListAddHead(List* this, ListElem* elem)
{
	if (!(this && elem))
		return PTRERR;

	if(!this->numbers)
	{
		this->tail = elem;
		elem->next = elem;
		elem->prev = elem;
		this->numbers++;
	}
	else
		ListAddAfter(this, elem, this->tail);
	this->head = elem;

	return 0;
}


int ListDelElem(List* this, ListElem* elem)
{
	if (!(this && elem))
		return PTRERR;
	
	if (this->numbers == 1)
	{
		this->tail = NULL;
		this->head = NULL;
	}
	else
	{
		ListElem* before = elem->prev;
		before->next = elem->next;
		elem->next->prev = before;
		if (this->head == elem)
			this->head = elem->next;
		else if (this->tail == elem)
			this->tail = elem->prev;
	}

	elem->prev = NULL;
	elem->next = NULL;
	this->numbers--;

	return 0;
}
