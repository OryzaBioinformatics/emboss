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
** @new ajRangeGet Create a range object from a string
** @new ajRangeFile Create a range object from a file
** @delete ajRangeDel Default destructor for range objects
** @modify ajRangeChange Set the values of a start and end in a range element
** @modify ajRangeBegin Sets the range values relative to the Begin value
** @use ajRangeNumber Return the number of ranges in a range object
** @use ajRangeValues Return (as parameters) start and end values in a range
** @use ajRangeText Return (as parameters) text value of a range
** @use ajRangeStrExtractList PushApp substrings defined by range onto list
** @use ajRangeStrExtract Extract substrings defined by range
** @use ajRangeStrStuff The opposite of ajRangeStrExtract
** @use ajRangeStrMask Mask the range in a String
** @use ajRangeStrToLower Change to lower-case the range in a String
** @use ajRangeOverlaps Detect overlaps of a set of ranges to a seq region
** @use ajRangeOrdered Test if ranges are in ascending non-overlapping order
** @use ajRangeDefault Test if the default range has been set
**
** @attr n [ajint] Number of ranges
** @attr start [ajint*] From positions
** @attr end [ajint*] End positions
** @attr text [AjPStr*] Associated text for each range
** @@
******************************************************************************/


typedef struct AjSRange
{
    ajint n;
    ajint *start;
    ajint *end;
    AjPStr *text;
} AjORange;
#define AjPRange AjORange*


AjBool    ajRangeBegin (AjPRange thys, ajint begin);
AjBool    ajRangeChange(AjPRange thys, ajint element, ajint start, ajint end);
AjBool    ajRangeDefault(const AjPRange thys, const AjPStr s);
void      ajRangeDel(AjPRange *thys);
AjPRange  ajRangeFile(const AjPStr name);
AjPRange  ajRangeGet(const AjPStr s);
AjPRange  ajRangeNewI(ajint n);
ajint     ajRangeNumber(const AjPRange thys);
AjBool    ajRangeOrdered (const AjPRange thys);
ajint	  ajRangeOverlaps (const AjPRange thys, ajint pos, ajint length);
ajint	  ajRangeOverlapSingle (ajint start, ajint end,
				ajint pos, ajint length);
AjBool    ajRangeStrExtract (const AjPRange thys,
			     const AjPStr instr, AjPStr *outstr);
AjBool    ajRangeStrExtractList (const AjPRange thys,
				 const AjPStr instr, AjPList outliststr);
AjBool    ajRangeStrMask (const AjPRange thys,
			  const AjPStr maskchar, AjPStr *str);
AjBool    ajRangeStrStuff (const AjPRange thys,
			   const AjPStr instr, AjPStr *outstr);
AjBool    ajRangeStrToLower (const AjPRange thys, AjPStr *str);
AjBool    ajRangeText(const AjPRange thys, ajint element, AjPStr * text);
AjBool    ajRangeValues(const AjPRange thys, ajint element,
			ajint *start, ajint *end);

#endif

#ifdef __cplusplus
}
#endif
