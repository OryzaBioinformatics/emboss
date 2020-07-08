/*  Last edited: Jun 21 14:11 1999 (pmr) */
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

int main (int argc, char **argv) {

  AjPStr instr;
  AjPStr nustr;
  AjPStrTok handle = NULL;
  AjPStr token = NULL;

  AjPTime today;

  embInit ("demostring", argc, argv);

  /* the ajStrTrace function writes the string contents out to the
     debug file demostring.dbg when you put -debug on the command line */

  /* instr is defined in the ACD processing so we just need to pick it
     up and should *not* be calling a constructor for instr. Think of
     ajAcdGetString as a constructor. Whatever you provide as the value when
     prompted will be stored in the instr variable.  */

  instr = ajAcdGetString("instring");

  ajDebug ("\n\n============ demostring output ========\n\n");
  ajDebug ("\nTrace of instr:\n");
  ajStrTrace(instr);

  /* nustr will be created using the string constructor functions.  We
     can create is using ajStrNew() but this just makes a clone of a null
     string and is pointless if we want to put anything in the string.
     More useful is to make a string of the right size, for example 32
     bytes */

  nustr = ajStrNewL(32);
  ajDebug ("\nNewly contructed nustr:\n");
  ajStrTrace(nustr);

  /* now we can put something into nustr. */

  ajStrAssC (&nustr, "Hello world");
  ajDebug ("\nAssigned nustr:\n");
  ajStrTrace(nustr);

  handle = ajStrTokenInit (nustr, " ");
  ajStrToken (&token, &handle, " ");
  ajUser ("first token '%S'\n", token);
  ajStrTokenRest (&token, &handle);
  ajUser ("second token '%S'\n", token);
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

  ajExit();
  return 0;
}
