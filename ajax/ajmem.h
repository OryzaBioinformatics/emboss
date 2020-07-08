#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajmem_h
#define ajmem_h

#include "ajexcept.h"

#ifndef HAVE_MEMMOVE
#define ajMemMove(d,s,l) bcopy (s,d,l)
#else
#define ajMemMove(d,s,l) memmove(d,s,l)
#endif

void *ajMemAlloc (long nbytes,
	const char *file, int line);
void *ajMemCalloc(long count, long nbytes,
	const char *file, int line);
void *ajMemCalloc0(long count, long nbytes,
	const char *file, int line);
void ajMemFree(void *ptr,
	const char *file, int line);
void *ajMemResize(void *ptr, long nbytes,
	const char *file, int line);
int *ajMemArrB (size_t size);
int *ajMemArrI (size_t size);
float *ajMemArrF (size_t size);
void ajMemStat (char* title);
void ajMemExit (void);

#define AJALLOC(nbytes) \
	ajMemAlloc((nbytes), __FILE__, __LINE__)
#define AJALLOC0(nbytes) \
	ajMemCalloc0(1, (nbytes), __FILE__, __LINE__)
#define AJCALLOC(count, nbytes) \
	ajMemCalloc((count), (nbytes), __FILE__, __LINE__)
#define AJCALLOC0(count, nbytes) \
	ajMemCalloc0((count), (nbytes), __FILE__, __LINE__)

#define AJNEW(p) ((p) = AJALLOC((long)sizeof *(p)))
#define AJCNEW(p,c) ((p) = AJCALLOC(c, (long)sizeof *(p)))
#define AJNEW0(p) ((p) = AJCALLOC0(1, (long)sizeof *(p)))
#define AJCNEW0(p,c) ((p) = AJCALLOC0(c, (long)sizeof *(p)))

#define AJFREE(ptr) ((void)(ajMemFree((ptr), \
	__FILE__, __LINE__), (ptr) = 0))
#define AJRESIZE(ptr, nbytes) 	((ptr) = ajMemResize((ptr), \
	(nbytes), __FILE__, __LINE__))
#define AJCRESIZE(ptr, nbytes) 	((ptr) = ajMemResize((ptr), \
	(nbytes)*(long)sizeof *(ptr), __FILE__, __LINE__))
#endif

#ifdef __cplusplus
}
#endif
