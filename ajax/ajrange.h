#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajrange_h
#define ajrange_h

/* @data AjPRange *************************************************************
**
** Data structure for AJAX sequence range specifications
**
** @alias AjORange
** @alias AjSRange
**
** @new ajRangeNewI Default constructor for range objects
** @delete ajRangeDel Default destructor for range objects
** @mod ajRangeGet Load a range object
** @mod ajRangeFile Load a range object from a file
** @mod ajRangeChange Set the values of a start and end in a range element
** @mod ajRangeBegin Sets the range values relative to the Begin value
** @use ajRangeNumber Return the number of ranges in a range object
** @use ajRangeValues Return (as parameters) start and end values in a range
** @use ajRangeText Return (as parameters) text value of a range
** @use ajRangeStrExtractList PushApp substrings defined by range onto list
** @use ajRangeStrExtract Extract substrings defined by range
** @use ajRangeStrStuff The opposite of ajRangeStrExtract
** @use ajRangeStrMask Mask the range in a String
** @use ajRangeOverlapSingle Detect an overlap of a range to a sequence region
** @use ajRangeOverlaps Detect overlaps of a set of ranges to a seq region
** @use ajRangeOrdered Test if ranges are in ascending non-overlapping order
** @use ajRangeDefault Test if the default range has been set
** @@
******************************************************************************/


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
ajint     ajRangeNumber(AjPRange thys);
AjBool    ajRangeText(AjPRange thys, ajint element, AjPStr * text);
AjBool    ajRangeValues(AjPRange thys, ajint element,
			ajint *start, ajint *end);
AjBool    ajRangeChange(AjPRange thys, ajint element, ajint start, ajint end);

AjBool    ajRangeBegin (AjPRange thys, ajint begin);

AjBool    ajRangeStrExtractList (AjPList outliststr, AjPRange thys,
				 AjPStr instr);
AjBool    ajRangeStrExtract (AjPStr *outstr, AjPRange thys, AjPStr instr);
AjBool    ajRangeStrStuff (AjPStr *outstr, AjPRange thys, AjPStr instr);
AjBool    ajRangeStrMask (AjPStr *str, AjPRange thys, AjPStr maskchar);
ajint	  ajRangeOverlapSingle (ajint start, ajint end,
				ajint pos, ajint length);
ajint	  ajRangeOverlaps (AjPRange thys, ajint pos, ajint length);
AjBool    ajRangeOrdered (AjPRange thys);

#endif

#ifdef __cplusplus
}
#endif
