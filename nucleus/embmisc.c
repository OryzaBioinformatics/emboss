/* @source embmisc.c
**
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




/* @func embMiscMatchPattern **************************************************
**
** Does a simple OR'd test of matches to (possibly wildcarded) words.
** The words are tested one at a time until a match is found.
** Whitespace and , ; | characters can separate the words in the pattern.
**
** @param [r] str [AjPStr] string to test
** @param [r] pattern [AjPStr] pattern to match with
**
** @return [AjBool] ajTrue = found a match
** @@
******************************************************************************/

AjBool embMiscMatchPattern (AjPStr str, AjPStr pattern)
{

    char whiteSpace[] = " \t\n\r,;|";  /* skip whitespace and , ; | */
    AjPStrTok tokens;
    AjPStr key = NULL;
    AjBool val = ajFalse;

    tokens = ajStrTokenInit(pattern, whiteSpace);
    while (ajStrToken( &key, &tokens, NULL))
	if (ajStrMatchWild(str, key))
	{
	    val = ajTrue;
	    break;
	}

    ajStrTokenClear( &tokens);
    ajStrDel(&key);

    return val;

}
