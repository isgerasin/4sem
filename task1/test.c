#include <stdio.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <unistd.h>

#include "list.h"

#define PRINT_RES(res, tar) \
		if ((res) tar) \
			fprintf(stderr, "OK %d\n", __LINE__); \
		else \
			fprintf(stderr, "ERROR at \n%4d |%s\n",__LINE__, #res)

int True(ListElem* elem, void* ctx)
{
	return 1;
}

int main(int argc, char const *argv[])
{
	struct rlimit tmpold, tmpnew;
	getrlimit(RLIMIT_AS, &tmpold);
	tmpnew.rlim_cur = 0;
	tmpnew.rlim_max = tmpold.rlim_max;
//--------------
/*
	setrlimit(RLIMIT_AS, &tmpnew);

	ListElem* elemC = ListElemCtor(88);
	PRINT_RES(elemC, == NULL);
	List* listC = ListCtor();
	PRINT_RES(listC, == NULL);

	setrlimit(RLIMIT_AS, &tmpold);
*/
//-----------------------------
	FILE* out = fopen("2.dump", "w");
	PRINT_RES(out, != NULL);

	ListElem* elem = ListElemCtor(45);
	PRINT_RES(elem, != NULL);
	ListElem* elemA = ListElemCtor(19);
	PRINT_RES(elemA, != NULL);
	ListElem* elemB = ListElemCtor(-98);
	PRINT_RES(elemB, != NULL);
	List* list = ListCtor();
	PRINT_RES(list, != NULL);

	PRINT_RES(ListElemDump(elem, out), == 0);
	PRINT_RES(ListElemDump(NULL, out), == PTRERR);
	PRINT_RES(ListElemDump(elem, NULL), == PTRERR);
	PRINT_RES(ListDump(list, out), == 0);
	PRINT_RES(ListDump(NULL, out), == PTRERR);
	PRINT_RES(ForEach(NULL, NULL, NULL), == PTRERR);
	PRINT_RES(ForEach(list, True, NULL), == 0);
	PRINT_RES(ListAddHead(NULL, NULL), == PTRERR );
	PRINT_RES(ListAddTail(NULL, NULL), == PTRERR );
	PRINT_RES(ListAddAfter(NULL, NULL, NULL), == PTRERR );
	PRINT_RES(ListDelElem(list, NULL), == PTRERR) ;
	PRINT_RES(ListAddHead(list, elem), == 0);
	PRINT_RES(ListDelElem(list, elem), == 0);
	PRINT_RES(ListAddTail(list, elem), == 0);
	PRINT_RES(ListAddHead(list, elemA), == 0);
	PRINT_RES(ListAddTail(list, elemB), == 0);
	PRINT_RES(ListDelElem(list, elemB), == 0);
	PRINT_RES(ListAddTail(list, elemB), == 0);
	PRINT_RES(ListDelElem(list, elemA), == 0);
	PRINT_RES(ListAddHead(list, elemA), == 0);
	PRINT_RES(ListDelElem(list, elem), == 0);
	PRINT_RES(ListElemDtor(elem, NULL), == 0);
	PRINT_RES(ListElemDtor(NULL, NULL), == PTRERR);
	PRINT_RES(ListDump(list, out), == 0);
	PRINT_RES(ListDump(list, NULL), == PTRERR);
	PRINT_RES(ForEach(list, True, NULL), == 1);
	PRINT_RES(ListDtor(NULL), == PTRERR);
	PRINT_RES(ListDtor(list), == 0);
	PRINT_RES(fclose(out), == 0);

	return 0;
}