#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajbase_h
#define ajbase_h

#include "ajax.h"
#include <ctype.h>

typedef struct AjIUB AjIUB;
struct AjIUB
{
    AjPStr code;
    AjPStr list;
};


extern AjBool aj_base_I;
extern float aj_base_prob[][32];


extern void ajBaseInit(void);
extern int  ajAZToInt(int c);
extern int  ajAZToBin(int c);
extern char ajAZToBinC(char c);
extern int  ajIntToAZ(int n);

#endif


#ifdef __cplusplus
}
#endif
