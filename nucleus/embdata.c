/* @source embdata.c
**
** General routines for alignment.
** Copyright (c) 1999 Mark Faller
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




/*
** Routines for getting the data into the data structure. The data structure
** consists of a list of tables. This means the routine can read any amount
** of data from a file. It is up to the developer to know the order of the
** tables in the list and what each refers to
*/




static AjBool dataListNextLine(AjPFile pfile, char *commentLine,
				 AjPStr * line);
static void dataListRead(AjPList data, AjPFile pfile);




/* @func embDataListDel *******************************************************
**
** Deletes the tables of data list. Calls ajTableFree for each table in the
** list, and then calls ajListFree to free the actual list.
**
** @param [w] data [AjPList] is the list of data tables to delete
** @return [void]
**
** @@
******************************************************************************/

void embDataListDel(AjPList data)
{
   AjIList iter;
   AjPTable table;

   iter = ajListIter(data);
   while(ajListIterMore(iter))
   {
      table = ajListIterNext(iter);
      ajTableFree(&table);
   }
   ajListIterFree(iter);
   ajListFree(&data);

   return;
}




/* @funcstatic dataListNextLine ***********************************************
**
** private function to read in the next line of data from the file. It is
** called from dataListRead.
**
** @param [r] pfile [AjPFile] file poiter to the data file
** @param [r] commentLine [char *] the character used as the to describe the
**        start of a comment line in the data file
** @param [w] line [AjPStr *] Buffer to hold the current line
** @return [AjBool] returns AjTrue if found another line of input otherwise
**         returns AjFalse
**
** @@
******************************************************************************/

static AjBool dataListNextLine(AjPFile pfile, char *commentLine,
			       AjPStr * line)
{
   ajint i;
   AjBool test;

   test = ajFileReadLine(pfile, line);
   while(test)
   {
      i = ajStrFindC(*line, commentLine);
      if(i!=0)
	  break;
      test = ajFileReadLine(pfile, line);
   }

   if(test)
       return ajTrue;

   return ajFalse;
}




/* @funcstatic dataListRead ***************************************************
**
** General routine for reading in data from a file. The keys and values of
** each table are stored as AjPStr. This is a private routine, It is called
** from embDataListInit.
**
** @param [w] data [AjPList] is the list of data tables.
** @param [r] pfile [AjPFile] pointer to the data file
** @return [void]
**
** @@
******************************************************************************/

static void dataListRead(AjPList data, AjPFile pfile)
{
   AjPStr line = NULL;
   AjPStrTok tokens;
   char whiteSpace[]  = " \t\n\r";
   char commentLine[] = "#";
   char endOfData[]   = "//";
   AjPStr key;
   AjPStr copyKey;
   AjPStr value;
   static AjPTable table;
   AjIList iter = NULL;
   AjPTable ptable;
   AjPStr tmp;

   tmp  = ajStrNew();
   line = ajStrNew();


   while(dataListNextLine(pfile, commentLine, &line))
   {
      tokens = ajStrTokenInit(line, whiteSpace);

      /* the first token is the key for the row */
      key = ajStrNew();
      ajStrToken(&key, &tokens, NULL);
      if(!ajStrLen(key))
      {
         ajFmtError("Error, did not pick up first key");
         ajFatal("Error, did not pick up first key");
      }

      while(1)
      {
	  /*
	  ** while there are more tokens generate new table in list and
	  ** add (key,value)
	  */
         value = NULL;
         if(ajStrToken(&value, &tokens, NULL))
         {
            table = ajStrTableNewCase(350);
            copyKey = ajStrDup(key);
            ajTablePut(table, copyKey, value);
            ajListPushApp(data, table);
         }
	 else break;
      }

      while(dataListNextLine(pfile, commentLine, &line))
      {
         /*
	 ** for rest of data iterate for each table in list adding
	 ** (key,value) to each
	 */
         tokens = ajStrTokenInit(line, whiteSpace);
         ajStrToken(&key, &tokens, NULL);
	 /* check for end of data block*/
	 if(! ajStrCmpC(key, endOfData))
	     break;
         iter = ajListIter(data);
         while(ajListIterMore(iter))
         {
            ptable = ajListIterNext(iter);
            copyKey = ajStrDup(key);
            if(!ajStrToken(&tmp, &tokens, NULL)) break;
            value = ajStrDup(tmp);
            ajTablePut(ptable, copyKey, value);
         }
      }
   }

   ajStrDel(&tmp);
   ajStrDel(&line);
   ajStrTokenClear(&tokens);
   ajListIterFree(iter);

   return;
}




/* @func embDataListInit ******************************************************
**
** Reads in the data file and puts the data into the list of tables. The
** keys and values of each table are stored as AjPStr.
**
** @param [w] data [AjPList] llist of data tables
** @param [r] file_name [AjPStr] the data filename
**
** @return [void]
** @@
******************************************************************************/

void embDataListInit(AjPList data, AjPStr file_name)
{
   AjPFile pfile = NULL;


   /* open the data table file */
   ajFileDataNew(file_name, &pfile);
   if(pfile==NULL)
       ajFatal("Unable to find the data file %S", file_name);

   dataListRead(data, pfile);
   ajFileClose(&pfile);

   return;
}




/* @func embDataListGetTables *************************************************
**
** Returns a list of data tables as requested. The data must already have been
** read in and stored as a list of tables. An unsigned integer is used to
** request tables. The first table has a value of 1, the second a value of 2,
** the third a value of 4, the fourth a value of 8 etc. For example a value
** of 10 would request the second and fourth tables from the list in that
** order. Only returns a list of pointers to the data. It does not copy the
** tables.
**
** @param [r] fullList [AjPList] The list containing all the tables of data
** @param [w] returnList [AjPList] The new list containing just the tables
**        requested
** @param [r] required [ajuint] used to request tables. A value of 1
**        requests the first table, a value of 16 requests the fifth table,
**        a value of 14 returns the second third and fourth tables in the
**        original list.
** @return [void]
**
** @@
******************************************************************************/

void embDataListGetTables(AjPList fullList, AjPList returnList,
			   ajuint required)
{
   AjIList iter;
   AjPTable table;

   iter = ajListIter(fullList);
   while(ajListIterMore(iter))
   {
      table = ajListIterNext(iter);
      if(required & 1) ajListPushApp(returnList, table);
      required >>= 1;
   }

   ajListIterFree(iter);

   return;
}




/* @func embDataListGetTable **************************************************
**
** Returns a single table of data from the list of data tables. The data must
** already have been read in and stored as a list of tables. An unsigned
** integer is used to request a table. The first table in the list has a
** value of 1, the second a value of 2, the third a value of 4, the fourth a
** value of 8 etc. For example a value of 64 would request the seventh data
** table in the list. When looking for which table to return the position of
** the lowest set bit in the value determines which table is returned i.e.
** a value of 66 would request the second table (not the seventh)
**
** @param [r] fullList [AjPList] The list containing all the tables of data
** @param [r] required [ajuint] used to request a table. A value of 1
**        requests the first table, a value of 16 requests the fifth table,
**        a value of 14 returns the second table in the original list.
** @return [AjPTable] the data table. Key and value are stored as AjPStrs
**
** @@
******************************************************************************/

AjPTable embDataListGetTable(AjPList fullList, ajuint required)
{
   AjIList iter;
   AjPTable returnTable = NULL;

   iter = ajListIter(fullList);
   while(ajListIterMore(iter))
   {
      returnTable = ajListIterNext(iter);
      if(required & 1)
	  break;
      required >>= 1;
   }


   ajListIterFree(iter);

   return returnTable;
}
