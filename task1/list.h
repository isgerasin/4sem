#define BAD_DATA 0
#define PTRERR 1
#define ELEMERR 2

#define _(ptr) \
		if ((ptr) == NULL) \
			return NULL

#define PL \
	fprintf(stderr, "%d\n", __LINE__)

#define CHECKNULL(ptr)\
		if (!(ptr)) \
			return PTRERR


typedef int ListData;

typedef struct ListElem
{
	ListData data;
	struct ListElem* prev;
	struct ListElem* next;
} ListElem;

typedef struct List
{
	ListElem* head;
	ListElem* tail;
	size_t numbers;
} List;

ListElem* ListElemCtor(ListData data);

int ListElemDtor(ListElem* this, void* contx);

int ListElemOk(ListElem* this, void* contx);

int ListElemDump(ListElem* this, void* contx);

List* ListCtor();

int ForEach(List* this, int (*act)(ListElem* elem, void* ctx), void* ctx);

int ListDtor(List* this);

int ListOk(List* this);

int ListDump(List* this, FILE* output);

int ListAddAfter(List* this, ListElem* elem, ListElem* after);

int ListAddHead(List* this, ListElem* elem);

int ListAddTail(List* this, ListElem* elem);

int ListDelElem(List* this, ListElem* elem);
