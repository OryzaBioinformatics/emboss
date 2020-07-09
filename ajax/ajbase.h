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


extern  void ajBaseInit(void);
AjBool  ajBaseAa1ToAa3(char aa1, AjPStr *aa3);

extern ajint  ajAZToInt(ajint c);
extern ajint  ajAZToBin(ajint c);
extern char ajAZToBinC(char c);
extern ajint  ajIntToAZ(ajint n);

#endif


#ifdef __cplusplus
}
#endif
