#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embinit_h
#define embinit_h

AjStatus embInit (const char *pgm, ajint argc, char * const argv[]);
AjStatus embInitP (const char *pgm, ajint argc, char * const argv[],
		   const char *package);

#endif

#ifdef __cplusplus
}
#endif
