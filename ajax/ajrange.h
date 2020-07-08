#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajrange_h
#define ajrange_h

typedef struct AjSRange
{
    int n;			/* Number of ranges */
    int *start;			/* From positions   */
    int *end;                   /* End positions    */
    AjPStr *text;		/* Associated text for each range */
} AjORange,*AjPRange;


AjPRange  ajRangeNewI(int n);
AjBool    ajRangeDefault(AjPRange thys, AjPStr s);

void      ajRangeDel(AjPRange *thys);

AjBool    ajRangeGet(AjPRange *r, AjPStr s);
AjBool    ajRangeFile(AjPRange *r, AjPStr name);
int       ajRangeNumber(AjPRange thys);
AjBool    ajRangeText(AjPRange thys, int element, AjPStr * text);
AjBool    ajRangeValues(AjPRange thys, int element, int *start, int *end);
AjBool    ajRangeChange(AjPRange thys, int element, int start, int end);

AjBool    ajRangeBegin (AjPRange thys, int begin);

AjBool    ajRangeStrExtractList (AjPList outliststr, AjPRange thys, AjPStr instr);
AjBool    ajRangeStrExtract (AjPStr *outstr, AjPRange thys, AjPStr instr);
AjBool    ajRangeStrStuff (AjPStr *outstr, AjPRange thys, AjPStr instr);
AjBool    ajRangeStrMask (AjPStr *str, AjPRange thys, AjPStr maskchar);
int	  ajRangeOverlapSingle (int start, int end, int pos, int length);
int	  ajRangeOverlaps (AjPRange thys, int pos, int length);
AjBool    ajRangeOrdered (AjPRange thys);

#endif

#ifdef __cplusplus
}
#endif
