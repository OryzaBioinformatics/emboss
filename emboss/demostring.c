/* @source demostring application
**
** Demomnstration of how the string functions should be used.
** @author: Copyright (C) Peter Rice (pmr@sanger.ac.uk)
** @@
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"

/* @prog demostring ***********************************************************
**
** Testing
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPStr instr;
    AjPStr nustr;
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    AjPStr maskstr = NULL;
    AjPStr masktest = NULL;
    ajint maskbegin = 0;
    ajint maskend = 0;

    AjPTime today;

    embInit ("demostring", argc, argv);

    /*
     *  the ajStrTrace function writes the string contents out to the
     *  debug file demostring.dbg when you put -debug on the command line
     */

    /*
     *  instr is defined in the ACD processing so we just need to pick it
     *  up and should *not* be calling a constructor for instr. Think of
     *  ajAcdGetString as a constructor. Whatever you provide as the value when
     *  prompted will be stored in the instr variable.
     */

    instr = ajAcdGetString("instring");

    ajDebug ("\n\n============ demostring output ========\n\n");
    ajDebug ("\nTrace of instr:\n");
    ajStrTrace(instr);

    ajUser ("Processing '%S'", instr);
    handle = ajStrTokenInit (instr, " ");
    ajStrToken (&token, &handle, " ");
    ajUser ("first token '%S'", token);
    ajStrTokenRest (&token, &handle);
    ajUser ("second token '%S'", token);
    ajStrTokenClear (&handle);

    /*
     *  nustr will be created using the string constructor functions.  We
     *  can create it using ajStrNew() but this just makes a clone of a null
     *  string and is pointless if we want to put anything in the string.
     *  More useful is to make a string of the right size, for example 32
     *  bytes
     */

    nustr = ajStrNewL(32);
    ajDebug ("\nNewly contructed nustr:\n");
    ajStrTrace(nustr);

    /* now we can put something into nustr. */

    ajStrAssC (&nustr, "Hello world");
    ajDebug ("\nAssigned nustr:\n");
    ajStrTrace(nustr);

    ajUser ("Processing '%S'", nustr);
    handle = ajStrTokenInit (nustr, " ");
    ajStrToken (&token, &handle, " ");
    ajUser ("first token '%S'", token);
    ajStrTokenRest (&token, &handle);
    ajUser ("second token '%S'", token);
    ajStrTokenClear (&handle);

    today = ajTimeTodayF("yyyy-mm-dd");
    ajTimeTrace (today);
    ajUser("yyyy-mm-dd Today is '%D'", today);
    today = ajTimeTodayF("dd Mon yyyy");
    ajTimeTrace (today);
    ajUser("dd Mon yyyy Today is '%D'", today);
    today = ajTimeTodayF("fred");
    ajTimeTrace (today);
    ajUser("fred Today is '%D'", today);
    today = ajTimeTodayF("gff");
    ajTimeTrace (today);
    ajUser("gff Today is '%D'", today);

    ajStrAssC(&maskstr, "abcdefghijklmnopqrstuvwxyz");
    ajStrAssS(&masktest, maskstr);
    maskbegin=0;
    maskend=0;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssC(&maskstr, "abcdefghijklmnopqrstuvwxyz");
    ajStrAssS(&masktest, maskstr);
    maskbegin=1;
    maskend=1;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=25;
    maskend=25;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=24;
    maskend=24;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=0;
    maskend=25;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=-1;
    maskend=-1;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=-2;
    maskend=-2;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=26;
    maskend=26;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=30;
    maskend=30;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=-30;
    maskend=30;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=-30;
    maskend=-30;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajStrAssS(&masktest, maskstr);
    maskbegin=-25;
    maskend=-2;
    ajStrMask(&masktest, maskbegin, maskend, '.');
    ajUser("Mask %d..%d '%S'", maskbegin, maskend, masktest);

    ajExit();
    return 0;
}
