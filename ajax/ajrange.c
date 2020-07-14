/******************************************************************************
** @source AJAX range functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Aug 21 ajb First version
** @modified 7 Sept 1999 GWW - String range edit functions added
** @modified 5 Nov 1999 GWW - store text after pairs of numbers
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */



/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */



/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */




/* @section Range Constructors ************************************************
**
** All constructors return a new object by pointer. It is the responsibility
** of the user to first destroy any previous object. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
******************************************************************************/




/* @func ajRangeNewI **********************************************************
**
** Default constructor for AJAX range objects.
**
** @param [r] n [ajint] number of ranges
**
** @return [AjPRange] Pointer to a range object
** @@
******************************************************************************/

AjPRange ajRangeNewI(ajint n)
{
    AjPRange thys;
    ajint i;

    AJNEW0(thys);

    thys->n = n;

    if(n>0)
    {
	thys->start = AJALLOC(n*sizeof(ajint));
	thys->end   = AJALLOC(n*sizeof(ajint));
	thys->text  = AJALLOC(n*sizeof(AjPStr *));
	for(i=0; i < n; i++)
	    thys->text[i] = NULL;
    }

    return thys;
}




/* @func ajRangeDel ***********************************************************
**
** Default destructor for AJAX range objects.
**
** @param [w] thys [AjPRange *] range structure
**
** @return [void]
** @@
******************************************************************************/

void ajRangeDel(AjPRange *thys)
{
    ajint i;

    if((*thys)->n > 0)
    {
	AJFREE((*thys)->start);
	AJFREE((*thys)->end);
	for(i=0; i < (*thys)->n; i++)
	    ajStrDel(&(*thys)->text[i]);
    }

    AJFREE((*thys)->text);
    AJFREE(*thys);

    return;
}




/* @func ajRangeGet ***********************************************************
**
** Load a range object
**
** @param [w] r [AjPRange *] range object
** @param [r] str [AjPStr] range string
**
** @return [AjBool] true if valid range
** @@
******************************************************************************/

AjBool ajRangeGet(AjPRange *r, AjPStr str)
{
    static AjPStr c1 = NULL;
    static AjPStr c2 = NULL;
    static AjPStr c3 = NULL;
    static AjPStr s   =NULL;
    char *p;
    ajint n;
    ajint e;
    ajint f;
    ajint t;
    ajint i;
    AjBool result  = ajTrue;
    AjBool doneone = ajFalse;

    char *nondigit="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                   " \t\n\r!@#$%^&*()_-+=|\\~`{[}]:;\"'<,>.?/";
    char *digit="0123456789";

    ajStrAssS(&s, str);

    /* clean up the ranges string */
    ajStrChomp(&s);

    /* is this a file of ranges? (does it start with a '@' ?) */
    if(*(ajStrStr(s)) == '@')
    {
	/* knock off the '@' */
	ajStrSub(&s, 1, ajStrLen(s));
	result = ajRangeFile(r, s);
    }
    else
    {
	/* get some copies of the string for parsing with strtok */
	ajStrAssS(&c1, s);
	ajStrAssS(&c2, s);
	ajStrAssS(&c3, s);

	p = ajStrStr(c1);
	p = ajSysStrtok(p, nondigit);

	n = 0;

	if(p)
	{
	    /*
	     *  count the pairs of numbers so that we know the size of
	     *  arrays to create
	     */
	    ++n;
	    while((p=ajSysStrtok(NULL, nondigit)))
		++n;

	    if(n%2)
	    {
		ajWarn("Odd integer(s) in range specification [%d]",n);
		return ajFalse;
	    }
	    *r=ajRangeNewI((e=n>>1));

	    /* get the pairs of numbers and put them in the AjPRange object */
	    p = ajStrStr(c2);
	    p = ajSysStrtok(p, nondigit);
	    if(!sscanf(p,"%d",&f))
	    {
		ajWarn("Bad range value [%s]",p);
		ajRangeDel(r);
		return ajFalse;
	    }

	    p = ajSysStrtok(NULL, nondigit);
	    if(!sscanf(p,"%d",&t))
	    {
		ajWarn("Bad range value [%s]",p);
		ajRangeDel(r);
		return ajFalse;
	    }

	    if(f>t)
	    {
		ajWarn("From range [%d] greater than To range [%d]",f,t);
		ajRangeDel(r);
		return ajFalse;
	    }
	    (*r)->start[0]=f;
	    (*r)->end[0]=t;

	    for(i=1;i<e;++i)
	    {
		p = ajSysStrtok(NULL, nondigit);
		if(!sscanf(p,"%d",&f))
		{
		    ajWarn("Bad range value [%s]",p);
		    ajRangeDel(r);
		    return ajFalse;
		}

		p = ajSysStrtok(NULL, nondigit);
		if(!sscanf(p,"%d",&t))
		{
		    ajWarn("Bad range value [%s]",p);
		    ajRangeDel(r);
		    return ajFalse;
		}

		if(f>t)
		{
		    ajWarn("From range [%d] greater than To range [%d]",f,t);
		    ajRangeDel(r);
		    return ajFalse;
		}
		(*r)->start[i] = f;
		(*r)->end[i]   = t;
	    }

	    /* now get any strings after the pairs of ranges */
	    p = ajStrStr(c3);
	    if(!isdigit((ajint)*p))
	    {
		doneone = ajTrue;
		p = ajSysStrtok(p, digit);
	    }

	    for(i=0; i<e; ++i)
	    {
		/* ignore anything between the two numbers */
		if(!doneone)
		{
		    p = ajSysStrtok(p, digit);
		    doneone = ajTrue;
		}
		else
		    p = ajSysStrtok(NULL, digit);

		/* this must be the text after the pair of numbers */
		/* get the string after the two numbers */
		p = ajSysStrtok(NULL, digit);
		if(p)
		{
		    ajStrAssC(&((*r)->text[i]), p);
		    ajStrChomp(&((*r)->text[i]));
		}
	    }
	}
	else
	    *r=ajRangeNewI(0);

	ajStrDelReuse(&c1);
	ajStrDelReuse(&c2);
	ajStrDelReuse(&c3);
    }

    ajStrDelReuse(&s);

    return result;
}




/* @func ajRangeFile **********************************************************
**
** Load a range object from a file
**
** The format of the range file is:
** Comment lines start with '#' in the first column.
** Comment lines and blank lines are ignored.
** The line may start with white-space.
** There are two positive numbers per line separated by white-space.
** The second number must be greater or equal to the first number.
** There is optional text after the two numbers.
** White-space before or after the text is removed.
**
** e.g.:
**
** # this is my set of ranges
** 12	23
**  4	5	this is like 12-23, but smaller
** 67	10348	interesting region
**
** @param [w] r [AjPRange *] range object
** @param [r] name [AjPStr] range file name
**
** @return [AjBool] true if valid range
** @@
******************************************************************************/

AjBool ajRangeFile(AjPRange *r, AjPStr name)
{
    AjBool result = ajTrue;
    AjPFile infile;
    AjPStr line   = NULL;
    char whiteSpace[] = " \t\n\r";
    char notSpace[]   = "\n\r";
    AjPStrTok tokens;
    ajint n = 0;			/* ranges found so far */
    ajint k;
    ajint numone;
    ajint numtwo;
    
    AjPStr one;
    AjPStr two;
    AjPStr text;
    
    AjPList onelist;
    AjPList twolist;
    AjPList textlist;


    onelist  = ajListstrNew();
    twolist  = ajListstrNew();
    textlist = ajListstrNew();

    
    if((infile = ajFileNewIn(name)) == NULL)
	return ajFalse;

    while(ajFileReadLine(infile, &line))
    {
	ajStrChomp(&line);
	
	if(!ajStrFindC(line, "#"))
	    continue;
	if(!ajStrLen(line))
	    continue;
	
	/*
	 ** parse the numbers out of the line and store in temporary
	 ** list (we may be reading data from stdin, so we can't read
	 ** in once to count the number of ajRange elements, close
	 ** file, open it again and read the data again to populate
	 ** ajRange)
	 */

	tokens = ajStrTokenInit(line, whiteSpace);
	
	one = ajStrNew();
	ajStrToken(&one, &tokens, NULL);
	ajListstrPushApp(onelist, one);
	
	two = ajStrNew();
	ajStrToken(&two, &tokens, NULL);
	if(ajStrLen(two))
	    ajListstrPushApp(twolist, two);
	else
	{
	    ajWarn("Odd integer(s) in range specification:\n%S\n", line);
	    return ajFalse;
	}
	
	/* get any remaining text and store in temporary list */
	text = ajStrNew();
	ajStrToken(&text, &tokens, notSpace);
	ajStrChomp(&text);
	ajListstrPushApp(textlist, text);
	
	ajStrTokenClear( &tokens);	
    }
    
    /* now we know how many pairs of numbers to store, create ajRange object */
    n  = ajListstrLength(onelist);
    *r = ajRangeNewI(n);
    
    /* populate ajRange object from lists and check numbers are valid */
    for(k = 0; k < n; k++)
    {
	ajListstrPop(onelist, &one);
	if(!ajStrToInt(one, &numone))
	{
	    ajWarn("Bad range value [%S]",one);
	    return ajFalse;
	}
	
	ajListstrPop(twolist, &two);
	if(!ajStrToInt(two, &numtwo))
	{
	    ajWarn("Bad range value [%S]",two);
	    return ajFalse;
	}

	ajStrDel(&one);
	ajStrDel(&two);
	
	if(numone > numtwo)
	{
	    ajWarn("From range [%d] greater than To range [%d]",
		   numone, numtwo);
	    return ajFalse;
	}
	
	(*r)->start[k] = numone;
	(*r)->end[k]   = numtwo;

	/* do the text */
	ajListstrPop(textlist, &text);
	(*r)->text[k] = text;
    }


    ajListstrFree(&onelist);
    ajListstrFree(&twolist);
    ajListstrFree(&textlist);

    return result;
}




/* @func ajRangeNumber ********************************************************
**
** Return the number of ranges in a range object
**
** @param [r] thys [AjPRange] range object
**
** @return [ajint] number of ranges
** @@
******************************************************************************/

ajint ajRangeNumber(AjPRange thys)
{
    return thys->n;
}




/* @func ajRangeValues ********************************************************
**
** Return (as parameters) start and end values in a range
**
** @param [r] thys [AjPRange] range object
** @param [r] element [ajint] range element (0 to n-1)
** @param [w] start [ajint *] start value
** @param [w] end [ajint *] end value
**
** @return [AjBool] true if range exists
** @@
******************************************************************************/

AjBool ajRangeValues(AjPRange thys, ajint element, ajint *start, ajint *end)
{
    if(element<0 || element>=thys->n)
	return ajFalse;

    *start = thys->start[element];
    *end   = thys->end[element];

    return ajTrue;
}




/* @func ajRangeText **********************************************************
**
** Return (as parameters) text value of a range
** The text value of a range is any non-digit after the pair of range numbers
** eg. in a pair of range '10-20 potential exon 50-60 repeat'
** the text values of the two ranges are: 'potential exon' and 'repeat'
**
** @param [r] thys [AjPRange] range object
** @param [r] element [ajint] range element (0 to n-1)
** @param [w] text [AjPStr *] text value
**
** @return [AjBool] true if range exists
** @@
** Modified: pmr 21-jan-00 return empty string if there is no text
******************************************************************************/

AjBool ajRangeText(AjPRange thys, ajint element, AjPStr * text)
{
    if(element<0 || element>=thys->n)
	return ajFalse;

    if(thys->text[element])
      *text = ajStrDup(thys->text[element]);
    else
      *text = ajStrNew();

    return ajTrue;
}




/* @func ajRangeChange ********************************************************
**
** Set the values of a start and end in a (preexisting) range element
**
** @param [r] thys [AjPRange] range object
** @param [r] element [ajint] range element (0 to n-1)
** @param [w] start [ajint] start value
** @param [w] end [ajint] end value
**
** @return [AjBool] true if range exists
** @@
******************************************************************************/

AjBool ajRangeChange(AjPRange thys, ajint element, ajint start, ajint end)
{
    if(element<0 || element>=thys->n)
	return ajFalse;

    thys->start[element] = start;
    thys->end[element]   = end;

    return ajTrue;
}




/* @func ajRangeBegin *********************************************************
**
** Sets the range values relative to the Begin value.
** Used when a sequence has -sbegin= and -send= parameters set
** and we have extracted the specified subsequence.
** So if -sbeg 11 has been set and the range is 11-12
** the resulting range is changed to 1-2
**
** @param [u] thys [AjPRange] range object
** @param [r] begin [ajint] begin parameter obtained from ajSeqBegin(seq)
**
** @return [AjBool] true if region values modified
** @@
******************************************************************************/

AjBool ajRangeBegin(AjPRange thys, ajint begin)
{
    ajint nr;
    ajint i;
    ajint st;
    ajint en;
    AjBool result = ajFalse;


    nr = ajRangeNumber(thys);

    for(i=0; i<nr; i++)
    {
	if(begin > 1)
	    result = ajTrue;
	ajRangeValues(thys, i, &st, &en);
	st -= begin-1;
	en -= begin-1;
	ajRangeChange(thys, i, st, en);
    }

    return result;
}




/* @func ajRangeStrExtractList ************************************************
**
** Extract the range from a String and place the resulting text in a
** list of strings.
**
** N.B. the resulting list will be regions of the input string listed
** in the order specified in the set of ranges. If these are not in ascending
** order, the resulting list of strings will not be in ascending order either.
** @param [w] outliststr [AjPList] resulting list of strings
** @param [r] thys [AjPRange] range object
** @param [r] instr [AjPStr] string to extract from
**
** @return [AjBool] true if string modified
** @@
******************************************************************************/

AjBool ajRangeStrExtractList(AjPList outliststr, AjPRange thys,
			     AjPStr instr)
{
    ajint nr;
    ajint i;
    ajint st;
    ajint en;
    AjBool result = ajFalse;
    AjPStr str;

    nr = ajRangeNumber(thys);

    for(i=0; i<nr; i++)
    {
	result = ajTrue;
	ajRangeValues(thys,i,&st,&en);
        str = ajStrNew();
	ajStrAppSub(&str, instr, st-1, en-1);
	ajListstrPushApp(outliststr, str);
    }

    return result;
}




/* @func ajRangeStrExtract ****************************************************
**
** Extract the range from a String (Remove regions not in the range(s))
** N.B. the resulting string will be regions of the input string appended
** in the order specified in the set of ranges. If these are not in ascending
** order, the resulting string will not be in ascending order either.
** @param [w] outstr [AjPStr *] resulting extracted string
** @param [r] thys [AjPRange] range object
** @param [r] instr [AjPStr] string to extract from
**
** @return [AjBool] true if string modified
** @@
******************************************************************************/

AjBool ajRangeStrExtract(AjPStr *outstr, AjPRange thys, AjPStr instr)
{
    ajint nr;
    ajint i;
    ajint st;
    ajint en;
    AjBool result = ajFalse;

    nr = ajRangeNumber(thys);

    for(i=0; i<nr; i++)
    {
	result = ajTrue;
	ajRangeValues(thys,i,&st,&en);
	ajStrAppSub(outstr, instr, st-1, en-1);
    }

    return result;
}




/* @func ajRangeStrStuff ******************************************************
**
** The opposite of ajRangeStrExtract()
** Stuff space characters into a string to pad out to the range.
**
** It takes a string and an ordered, non-overlapping set of ranges and puts
** spaces into the string between the ranges.
** So starting with the string 'abcde' and the ranges 3-5,7-8 it will produce:
** '  abc de'
**
** @param [w] outstr [AjPStr *] resulting stuffed string
** @param [r] thys [AjPRange] range object
** @param [r] instr [AjPStr] string to stuff
**
** @return [AjBool] true if string modified
** @@
******************************************************************************/

AjBool ajRangeStrStuff(AjPStr *outstr, AjPRange thys, AjPStr instr)
{
    ajint nr;
    ajint i;
    ajint j;
    ajint lasten = 0;
    ajint lastst = 0;
    ajint len;
    ajint st;
    ajint en;
    AjBool result = ajFalse;

    nr = ajRangeNumber(thys);

    for(i=0; i<nr; i++)
    {
	result = ajTrue;
	ajRangeValues(thys,i,&st,&en);
	/* change range positions to string positions */
	--st;
	--en;
	len = en-st;

        for(j=lasten; j<st; j++)
	    ajStrAppC(outstr, " ");

	ajStrAppSub(outstr, instr, lastst, lastst+len);
        lastst = lastst+len+1;
        lasten = en+1;
    }

    return result;
}




/* @func ajRangeStrMask *******************************************************
**
** Mask the range in a String
** @param [w] str [AjPStr *] string to be masked
** @param [r] thys [AjPRange] range object
** @param [r] maskchar [AjPStr] character to mask with
**
** @return [AjBool] true if string modified
** @@
******************************************************************************/

AjBool ajRangeStrMask(AjPStr *str, AjPRange thys, AjPStr maskchar)
{
    ajint nr;
    ajint i;
    ajint j;
    ajint st;
    ajint en;
    AjBool result = ajFalse;;

    nr = ajRangeNumber(thys);

    for(i=0; i<nr; ++i)
    {
	result = ajTrue;
	ajRangeValues(thys,i,&st,&en);

	/* change range positions to string positions */
	--st;
	--en;

	/* cut out the region */
	ajStrCut(str, st, en);

	/* replace the region with the mask character */
	for(j=st; j<=en; ++j)
	    ajStrInsert(str, st, maskchar);
    }

    return result;
}




/* @func ajRangeStrToLower *******************************************************
**
** Change the range in a String to lower-case
** @param [w] str [AjPStr *] string to be lower-cased
** @param [r] thys [AjPRange] range object
**
** @return [AjBool] true if string modified
** @@
******************************************************************************/

AjBool ajRangeStrToLower(AjPStr *str, AjPRange thys)
{
    ajint nr;
    ajint i;
    ajint st;
    ajint en;
    AjBool result = ajFalse;
    AjPStr substr;

    substr = ajStrNew();

    nr = ajRangeNumber(thys);

    for(i=0; i<nr; ++i)
    {
	result = ajTrue;
	ajRangeValues(thys,i,&st,&en);

	/* change range positions to string positions */
	--st;
	--en;

	/* extract the region and lowercase */
	ajStrAppSub(&substr, *str, st, en);
	ajStrToLower(&substr);

	/* remove and replace the lowercased region */
	ajStrCut(str, st, en);
        ajStrInsert(str, st, substr);
	ajStrClear(&substr);        
    }

    ajStrDel(&substr);
    
    return result;
}




/* @func ajRangeOverlapSingle *************************************************
**
** Detect an overlap of a single range to a region of a sequence
** @param [r] start [ajint] start of range
** @param [r] end [ajint] end of range
** @param [r] pos [ajint] postion in sequence of start of region of sequence
** @param [r] length [ajint] length of region of sequence
**
** @return [ajint] 0=no overlap 1=internal 2=complete 3=at left 4=at right
** @@
******************************************************************************/

ajint ajRangeOverlapSingle(ajint start, ajint end, ajint pos, ajint length)
{
    ajint posend;

    /* end position of region in sequence */
    posend = pos+length-1;

    /* convert range positions to sequence positions */
    start--;
    end--;

    if(end < pos || start > posend)
	return 0;
    /* no overlap 		~~~~ |--------|	*/
    if(start >= pos && end <= posend)
	return 1;
    /* internal overlap	     |-~~~~~--|		*/
    if(start < pos && end > posend)
	return 2;
    /* complete overlap	~~~~~|~~~~~~~~|~~	*/
    if(start < pos && end >= pos )
	return 3;
    /* overlap at left	~~~~~|~~~-----|		*/
    if(start >= pos && end > posend )
	return 4;
    /* overlap at right	     |----~~~~|~~~	*/

    ajFatal("ajrangeoverlapsingle error");

    return -1;
}




/* @func ajRangeOverlaps ******************************************************
**
** Detect overlaps of a set of ranges to a region of a sequence
** @param [r] thys [AjPRange] range object
** @param [r] pos [ajint] postion in sequence of start of region of sequence
** @param [r] length [ajint] length of region of sequence
**
** @return [ajint] Number of ranges in range object with overlaps to the region
** @@
******************************************************************************/

ajint ajRangeOverlaps(AjPRange thys, ajint pos, ajint length)
{
    ajint nr;
    ajint i;
    ajint st;
    ajint en;
    ajint result = 0;

    nr = ajRangeNumber(thys);

    for(i=0; i<nr; i++)
    {
	ajRangeValues(thys,i,&st,&en);
	if(ajRangeOverlapSingle(st, en, pos, length)) result++;
    }

    return result;
}




/* @func ajRangeOrdered *******************************************************
**
** Tests to see if the set of ranges are in ascending non-overlapping order
** @param [r] thys [AjPRange] range object
**
** @return [AjBool] ajTrue if in ascending non-overlapping order
** @@
******************************************************************************/

AjBool ajRangeOrdered(AjPRange thys)
{
    ajint nr;
    ajint i;
    ajint st;
    ajint en;
    ajint last = -1;

    nr = ajRangeNumber(thys);

    for(i=0; i<nr; i++)
    {
	ajRangeValues(thys,i,&st,&en);
        if(st <= last || en <= st)
	    return ajFalse;
        last = en;
    }

    return ajTrue;
}




/* @func ajRangeDefault *******************************************************
**
** Test whether the default range has been set for a string
**
** IE tests whether the given range is a single range from the start to
** the end of a sequence string.
**
** @param [r] thys [AjPRange] range object
** @param [r] s [AjPStr] string
**
** @return [AjBool] true if default range
** @@
******************************************************************************/

AjBool ajRangeDefault(AjPRange thys, AjPStr s)
{
    if(thys->n==1 && thys->start[0]==1 && thys->end[0]==ajStrLen(s))
	return ajTrue;

    return ajFalse;
}
