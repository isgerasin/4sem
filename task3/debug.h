
#include <errno.h>

#define _(var) do {errno = 0; \
			if ((var)<0) \
				{ perror(#var); return -1;} }while(0)

#define PL fprintf(stderr, "%d\n", __LINE__);