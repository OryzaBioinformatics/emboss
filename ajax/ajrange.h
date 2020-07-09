#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajrange_h
#define ajrange_h

typedef struct AjSRange
{
    ajint n;			/* Number of ranges */
    ajint *start;			/* From positions   */
    ajint *end;                   /* End positions    */
    AjPStr *text;		/* Associated text for each range */
} AjORange,*AjPRange;


AjPRange  ajRangeNewI(ajint n);
AjBool    ajRangeDefault(AjPRange thys, AjPStr s);

void      ajRangeDel(AjPRange *thys);

AjBool    ajRangeGet(AjPRange *r, AjPStr s);
AjBool    ajRangeFile(AjPRange *r, AjPStr name);
ajint       ajRangeNumber(AjPRange thys);
AjBool    ajRangeText(AjPRange thys, ajint element, AjPStr * text);
AjBool    ajRangeValues(AjPRange thys, ajint element, ajint *start, ajint *end);
AjBool    ajRangeChange(AjPRange thys, ajint element, ajint start, ajint end);

AjBool    ajRangeBegin (AjPRange thys, ajint begin);

AjBool    ajRangeStrExtractList (AjPList outliststr, AjPRange thys, AjPStr instr);
AjBool    ajRangeStrExtract (AjPStr *outstr, AjPRange thys, AjPStr instr);
AjBool    ajRangeStrStuff (AjPStr *outstr, AjPRange thys, AjPStr instr);
AjBool    ajRangeStrMask (AjPStr *str, AjPRange thys, AjPStr maskchar);
ajint	  ajRangeOverlapSingle (ajint start, ajint end, ajint pos, ajint length);
ajint	  ajRangeOverlaps (AjPRange thys, ajint pos, ajint length);
AjBool    ajRangeOrdered (AjPRange thys);

#endif

#ifdef __cplusplus
}
#endif
