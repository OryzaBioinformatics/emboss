/********************************************************************
** @source AJAX nam functions 
** Creates a hash table of initial values and allow access to this
** via the routines ajNamDatabase and ajNamGetValue.
**
** @author Copyright (C) 1998 Ian Longden
** @version 1.0
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
********************************************************************/

#include "ajax.h"

#include <dirent.h>
#include <unistd.h>

#define TYPE_START 1
#define TYPE_ENV 2
#define TYPE_DB  3
#define TYPE_OPT 4

/* Scope values for entry methods returned from nameMethod2Scope */
#define METHOD_ENTRY 1
#define METHOD_QUERY 2
#define METHOD_ALL   4
#define SLOW_ENTRY 8
#define SLOW_QUERY 16

static AjBool namdebug = AJFALSE;
static AjPStr namRootStr  = NULL;

static AjBool namListParseOK = AJFALSE;

char* namTypes[] = {"unknown", "START", "ENV", "DB", "OPT"};

/* source directory where control and data files can be found */

#ifdef AJAX_FIXED_ROOT
static char namFixedRoot[] = AJAX_FIXED_ROOT;
#else
static char namFixedRoot[] = "/nfs/WWW/data/EMBOSS";
#endif

/* install target directory where installed control and data files can
   be found  - this is the --prefix= from ./configure */

#ifdef PREFIX
static char namInstallRoot[] = PREFIX;
#else
static char namInstallRoot[] = "/usr/local";
#endif

/* package name from the makefile */

#ifdef PACKAGE
static char namPackage[] = PACKAGE;
#else
static char namPackage[] = "EMBOSS";
#endif

static AjPStr namFixedRootBaseStr = NULL;
static AjPStr namPrefixStr = NULL;
static AjPStr namFileOrig = NULL;

static AjPTable standardNames = NULL;
static ajint namParseType = 0;

typedef struct NamSAttr {
  char* Name;
  char* Defval;
} NamOAttr, *NamPAttr;

NamOAttr namAttr[] = {
  {"name", ""},
  {"directory", ""},
  {"filename", ""},
  {"indexdirectory", ""},
  {"url", ""},
  {"method", ""},
  {"methodentry", ""},
  {"methodquery", ""},
  {"methodall", ""},
  {"release", ""},
  {"type", ""},
  {"format", ""},
  {"formatentry", ""},
  {"formatquery", ""},
  {"formatall", ""},
  {"command", ""},
  {"comment", ""},
  {"description", ""},
  {"dbalias", ""},
  {"app", ""},
  {"appentry", ""},
  {"appquery", ""},
  {"appall", ""},
  {"exclude", ""},
  {NULL, NULL}
};

typedef struct AjSNamStandards {
  AjPStr name;
  AjPStr value;
  ajint type;
  ajint scope;
  void* data ;	     /* Attribute values for database*/
} AjONamStandards, *AjPNamStandards;

static void namListStandardsDelete(void);
static void namPrintDatabase(AjPStr* dbattr);
static void namListStandards(ajint which);
static ajint namMethod2Scope (AjPStr method);
static void namDebugStandards(ajint which);
static void namDebugDatabases(void);
static void namDebugEnvironmentals(void);
static void namListParse(void **x,void *cl);
static void namProcessFile(FILE *file);
static void namNoColon (AjPStr *thys);
static ajint namDbAttr (AjPStr thys);
static ajint namDbAttrC (char* str);
static void namUser (char *fmt, ...);
static void namEntryDelete (AjPNamStandards entry);
static AjBool namDbSetAttr (AjPStr* dbattr, char* attrib, AjPStr* qrystr);
static AjBool namVarResolve (AjPStr* var);

/* @funcstatic namEntryDelete *************************************************
**
** Deletes a database entry from the internal table.
**
** @param [P] entry [AjPNamStandards] The entry to be deleted.
** @return [void]
** @@
******************************************************************************/

static void namEntryDelete (AjPNamStandards entry) {

  ajint j; 
  AjPStr* dbattr;

  if(entry->type == TYPE_DB){
    if(entry->name) {
      ajStrDel(&entry->name);
    }
    if(entry->value) {
      ajStrDel(&entry->value);
    }
    dbattr = entry->data;
    for (j=0; namAttr[j].Name; j++) {
      if (dbattr[j]) {
	ajStrDel(&dbattr[j]);
      }
    }
    AJFREE(entry->data);
  }
  else if(entry->type == TYPE_ENV){
    if(entry->name) {
      ajStrDel(&entry->name);
    }
    if(entry->value) {
      ajStrDel(&entry->value);
    }
  }
  AJFREE(entry);

  return;
}

/* @funcstatic namListStandardsDelete *****************************************
**
** Deletes all databases in the internal table. The table is converted to
** an array, and each entry in turn is passed to namEntryDelete.
**
** @return [void]
** @@
******************************************************************************/

static void namListStandardsDelete (void) {

  ajint i;
  AjPNamStandards fnew = 0;
  void **array = ajTableToarray(standardNames, NULL);

  for (i = 0; array[i]; i += 2) {
    AJFREE(array[i]);		/* the key */
    fnew = (AjPNamStandards) array[i+1];
    namEntryDelete (fnew);
  }
  AJFREE(array); 

  return;
}

/* @funcstatic namPrintDatabase ***********************************************
**
** Prints a report of defined attributes for a database definition.
**
** @param [P] dbattr [AjPStr*] Attribute list from a database entry.
** @return [void]
** @@
******************************************************************************/

static void namPrintDatabase (AjPStr* dbattr){

  ajint i;

  for (i=0; namAttr[i].Name; i++) {
    if (ajStrLen(dbattr[i])) {
      ajUser ("\t%s: %S", namAttr[i].Name, dbattr[i]);
    }
  }

  return;
}

/* @funcstatic namListStandards ***********************************************
**
** Lists databases or variables defined in the internal table.
**
** @param [r] which [ajint] Variable type, either TYPE_ENV for environment
**                        variables or TYPE_DB for databases.
** @return [void]
** @@
******************************************************************************/

static void namListStandards (ajint which)
{ ajint i;
  AjPNamStandards fnew;
  void **array = ajTableToarray(standardNames, NULL);
  char *key;

  for (i = 0; array[i]; i += 2) {
    fnew = (AjPNamStandards) array[i+1];
    key = (char*) array[i];
    if(fnew->type == which){
      if(TYPE_DB == which){
	ajUser ("DB %S\t *%s*", fnew->name, key);
	namPrintDatabase(fnew->data);
	ajUser ("");
      }
      else if(TYPE_ENV == which) {
	ajUser ("ENV %S\t%S\t *%s*",fnew->name,fnew->value,key);
      }
    }
  }
  AJFREE(array); 

  return;
}

/* @func ajNamDbDetails ***********************************************
**
** Returns database access method information
**
** @param [r] name [AjPStr] Database name
** @param [w] type [AjPStr*] sequence type - 'P' or 'N'
** @param [w] id [AjBool*] ajTrue = can access single entries 
** @param [w] qry [AjBool*] ajTrue = can access wild/query entries
** @param [w] all [AjBool*] ajTrue = can access all entries
** @param [w] comment [AjPStr*] comment about database
** @return [AjBool] ajTrue if database details were found
** @@
******************************************************************************/

AjBool ajNamDbDetails (AjPStr name, AjPStr* type, AjBool* id, AjBool* qry,
		       AjBool* all, AjPStr* comment) { 
  AjPNamStandards fnew = 0;
  AjPStr* dbattr = NULL;
  ajint i;
  ajint scope;

/* assume that the database can't be accessed by any method until we find otherwise */
  *id = *qry = *all = ajFalse;
  (void) ajStrDelReuse (type);
  (void) ajStrDelReuse (comment);

  fnew = ajTableGet(standardNames, ajStrStr(name));
  if (fnew) {
    ajDebug ("  '%S' found\n", name);

    dbattr = fnew->data;
    for (i=0; namAttr[i].Name; i++) {
      ajDebug("Attribute name = %s, value = %S\n", namAttr[i].Name, dbattr[i]);
      if (ajStrLen(dbattr[i])) {
        if (!strcmp ("type", namAttr[i].Name))
	  (void) ajStrAss(type, dbattr[i]);
        if (!strcmp ("method", namAttr[i].Name)) {
          scope = namMethod2Scope(dbattr[i]);
          if (scope & METHOD_ENTRY) *id = ajTrue;
          if (scope & METHOD_QUERY) *qry = ajTrue;
          if (scope & METHOD_ALL) *all = ajTrue;
        }
        if (!strcmp ("methodentry", namAttr[i].Name)) {
          scope = namMethod2Scope(dbattr[i]);
          if (scope & METHOD_ENTRY) *id = ajTrue;
        }
        if (!strcmp ("methodquery", namAttr[i].Name)) {
          scope = namMethod2Scope(dbattr[i]);
          if (scope & METHOD_ENTRY) *id = ajTrue;
          if (scope & METHOD_QUERY) *qry = ajTrue;
        }
        if (!strcmp ("methodall", namAttr[i].Name)) {
          scope = namMethod2Scope(dbattr[i]);
          if (scope & METHOD_ALL) *all = ajTrue;
        }
        if (!strcmp ("comment", namAttr[i].Name))
	  (void) ajStrAss(comment, dbattr[i]);
      }
    }

    return ajTrue;
  }

  ajDebug ("  '%S' not found\n", name);
  return ajFalse;
}

/* @funcstatic namMethod2Scope **********************************************
**
** Returns OR'ed values of METHOD_ENTRY, METHOD_QUERY and METHOD_ALL
** for the various types of access method for databases.
**
** @param [r] method [AjPStr] Variable type, either TYPE_ENV for environment
**
** @return [ajint] OR'ed values for the valid scope of the access method given
** @@
******************************************************************************/
static ajint namMethod2Scope (AjPStr method) {

ajint result = 0;

 if (!ajStrCmpC(method, "direct"))
   result = (METHOD_ALL | SLOW_QUERY | SLOW_ENTRY );
 if (!ajStrCmpC(method, "srs"))
   result = (METHOD_ENTRY | METHOD_QUERY);
 if (!ajStrCmpC(method, "srsfasta"))
   result = (METHOD_ENTRY | METHOD_QUERY);
 if (!ajStrCmpC(method, "srswww"))
   result = METHOD_ENTRY;
 if (!ajStrCmpC(method, "url"))
   result = METHOD_ENTRY;
 if (!ajStrCmpC(method, "emblcd"))
   result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
 if (!ajStrCmpC(method, "staden"))
   result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
 if (!ajStrCmpC(method, "external"))
   result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
 if (!ajStrCmpC(method, "blast"))
   result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
 if (!ajStrCmpC(method, "app"))
   result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
 if (!ajStrCmpC(method, "corba"))
   result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
 if (!ajStrCmpC(method, "nbrf"))	  /* not sure - GWW 5 Aug 1999 */
   result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
 if (!ajStrCmpC(method, "gcg"))
   result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);

  return result;
}

/* @funcstatic namDebugStandards **********************************************
**
** Writes report to standard output of databases or variables defined
** in the internal table.
**
** @param [r] which [ajint] Variable type, either TYPE_ENV for environment
**                        variables or TYPE_DB for databases.
** @return [void]
** @@
******************************************************************************/

static void namDebugStandards (ajint which)
{ ajint i;
  AjPNamStandards fnew;
  void **array = ajTableToarray(standardNames, NULL);
  char *key;

  for (i = 0; array[i]; i += 2) {
    fnew = (AjPNamStandards) array[i+1];
    key = (char*) array[i];
    if(fnew->type == which){
      if(TYPE_DB == which){
	namUser ("DB %S\t *%s*\n", fnew->name, key);
	namPrintDatabase(fnew->data);
      }
      else if(TYPE_ENV == which) {
	namUser ("ENV %S\t%S\t *%s* ",fnew->name,fnew->value,key);
	namUser ("\n");
      }
    }
  }

  AJFREE(array); 

  return;
}

/* @func ajNamListOrigin *********************************************
**
** Writes a simple list of where the internal tables came from..
**
** @return [void]
** @@
******************************************************************************/

void ajNamListOrigin (void)
{
  ajUser("SOURCE---------->");
  ajUser("%S", namFileOrig);
  ajUser("SOURCE---------->");
  ajUser("");
}

/* @func ajNamListDatabases *********************************************
**
** Writes a simple list of all databases in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamListDatabases (void)
{
  ajUser("DB---------->");
  namListStandards(TYPE_DB);
  ajUser("DB---------->");
  ajUser("");

  return;
}

/* @func ajNamListListDatabases *********************************************
**
** Creates a AjPList list of all databases in the internal table.
**
** @param [w] dbnames [AjPList] Str List of names to be populated
** @return [void]
** @@
******************************************************************************/

void ajNamListListDatabases (AjPList dbnames) {
  ajint i;
  AjPNamStandards fnew;
  void **array = ajTableToarray(standardNames, NULL);

  for (i = 0; array[i]; i += 2) {
    fnew = (AjPNamStandards) array[i+1];
    if (fnew->type == TYPE_DB) {
      ajDebug("DB: %S\n", fnew->name);
      ajListstrPushApp(dbnames, fnew->name);
    }
  }
  AJFREE(array); 

  return;
}

/* @func ajNamEnvironmentals ********************************************
**
** Writes a simple list of all variables in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamEnvironmentals(void)
{
  ajUser("ENV---------->");
  namListStandards(TYPE_ENV);
  ajUser("ENV---------->");
  ajUser("");

  return;
}

/* @funcstatic namDebugDatabases **********************************************
**
** Writes a simple list of all databases in the internal table.
**
** @return [void]
** @@
******************************************************************************/

static void namDebugDatabases (void)
{
  namUser ("DB---------->");
  namDebugStandards(TYPE_DB);
  namUser ("DB---------->");
  namUser ("\n");

  return;
}

/* @funcstatic namDebugEnvironmentals *****************************************
**
** Writes a simple list of all variables in the internal table.
**
** @return [void]
** @@
******************************************************************************/

static void namDebugEnvironmentals (void)
{
  namUser ("ENV---------->\n");
  namDebugStandards(TYPE_ENV);
  namUser ("ENV---------->\n");
  namUser ("\n");

  return;
}

/* @funcstatic namListParse ***************************************************
**
** Parse the text in a list of tokens read from the input file.
** Derive environment variable and database definitions. Store
** all these in the internal tables.
**
** @param [P] x [void**] Standard argument for list mapping. List item.
** @param [P] cl [void*] Standard argument for list mapping. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void namListParse (void** x,void* cl) {
  static char* tabname = 0;
  static AjPStr name = 0;
  static AjPStr value = 0;
  static char quoteopen = 0, quoteclose = 0;
  static AjPStr* dbattr = 0;
  static ajint  db_input = -1;
  AjPNamStandards fnew = 0, entry=0;
  AjPStr *p = (AjPStr*)x;
  AjBool dbsave = ajFalse;
  AjBool saveit = ajFalse;
  ajint nattr = 0;

  for (nattr=0; namAttr[nattr].Name; nattr++) ;	/* nattr = count attributes */

  namUser("namListParse <%S>\n", *p);

  if (!namParseType){
    namNoColon(p);
    (void) ajStrToLower(p);
    if (ajStrPrefixCO ("env", *p)) {
      namParseType = TYPE_ENV;
    }
    if (ajStrPrefixCO ("setenv", *p)) {
      namParseType = TYPE_ENV;
    }
    if (ajStrPrefixCO ("start", *p)) {
      return;
    }
    else if(ajStrPrefixCO ("dbname",*p)){
      namParseType = TYPE_DB;
    }
    else if(ajStrPrefixCO ("option",*p)){
      namParseType = TYPE_OPT;
    }
    if (!namParseType)
      ajWarn ("%S: Invalid type '%S'\n", namRootStr, *p);
    namUser("type set to %s\n", namTypes[namParseType]);
  }
  else if (quoteopen){		/* quote is open, so append word */
				/* till close quote found */
    namUser ("<%c>..<%c> quote processing\n", quoteopen, quoteclose);
    (void) ajStrAppC(&value," ");    
    (void) ajStrApp(&value,*p);    
    if(ajStrChar(*p,-1) == quoteclose){  /* close quote here ?? */
      namUser ("close quotes\n");
      (void) ajStrTrim(&value, -1);
      quoteopen = quoteclose = 0;
      if(namParseType == TYPE_ENV)	/* set save flag as value in quotes */
				/* has been obtained */
	saveit = ajTrue;
      else if(namParseType == TYPE_DB) 
	dbsave = ajTrue;
    }
  }
  else if(namParseType == TYPE_ENV) {
    if (name && value) {
      saveit= ajTrue;
    }
    else if (name) {		/* if name already got then it must be */
				/* the value */
      if(ajStrChar(*p,0) == '\'') {
	quoteopen = quoteclose = '\'';
      }
      else if (ajStrChar(*p,0) == '\"') {
	quoteopen = quoteclose = '\"';
      }

      (void) ajStrAss (&value, *p);
      if(quoteopen) {                        /* remove open quote */
	(void) ajStrTrim(&value, 1);
      }
      else
	saveit = ajTrue;

      if(ajStrChar(*p, -1) == quoteclose){ /* end of quote on same word */
	quoteopen = quoteclose = 0;
	saveit= ajTrue;
	(void) ajStrTrim(&value, -1);       /* remove quote at the end */ 
      }
    }
    else {
      (void) ajStrAss (&name, *p);
    }
  }
  else if(namParseType == TYPE_DB) {
    if(ajStrMatchC(*p, "[")) {           /* [ therefore new database */
      dbattr = AJCALLOC0(nattr, sizeof(AjPStr));   /* new database structure */
    }
    else if (ajStrMatchC(*p, "]")) {     /* ] therefore end of database */
      saveit = ajTrue;
    }
    else if(name){
      if(ajStrChar(*p, -1) == ':'){  /* if last character is a : */
				          /* then it a keyword */
	(void) ajStrToLower(p);                  /* make it lower case */
	namNoColon(p);
	db_input = namDbAttr(*p);
	if (db_input < 0)
	  ajWarn ("%S: bad attribute '%S' for database '%S'\n",
		  namRootStr, *p, name);
      }
      else if(db_input >= 0){        /* So if keyword type has been set */
	if(ajStrChar(*p, 0) == '\'') {   /* is there a quote? If so expect the */
	    /* same at the end. No ()[] etc yet here*/
	  quoteopen = quoteclose = '\'';
	}
	else if (ajStrChar(*p, 0) == '\"') {
	  quoteopen = quoteclose = '\"';
	}

	(void) ajStrAss (&value, *p);
	if (quoteopen)
	  (void) ajStrTrim(&value, 1);
	else
	  dbsave = ajTrue;

	if(ajStrChar(*p, -1) == quoteclose){  
	  quoteopen = quoteclose = 0;
	  (void) ajStrTrim(&value,-1);   /* ignore quote if there is */
				              /* one at end*/ 
	  dbsave = ajTrue;
	}
	if(!quoteopen)
	  dbsave = ajTrue;
      }
    }
    else {
      (void) ajStrAss (&name, *p);
    }
  }
  if (dbsave) {                                /* Save the keyword value */
    (void) ajStrAss(&dbattr[db_input], value);
    db_input =-1;
    ajStrDel(&value);
    dbsave = ajFalse;
  }
    
  namListParseOK = saveit;

  if (saveit) {

    AJNEW0(fnew);
    fnew->name = 0;
    fnew->value = 0;
    tabname = ajCharNew(name);
    fnew->name = name;
    name = 0;
    fnew->value = value;
    value = 0;
    fnew->scope = 0;
    fnew->type = namParseType;
    if(namParseType == TYPE_DB) {
      fnew->data = dbattr;
    } 
    else
      fnew->data = 0;

      /* Add new one to table */
      /* be very careful that everything in the table */
      /* is not about to be deallocated - so do not use "name" here */

    entry = ajTablePut (standardNames, tabname, fnew);
    if (entry) {  /* it existed so over write previous table entry */
      ajDebug ("%S: replaced previous definition of '%S'\n",
	      namRootStr,entry->name); /* never writes - too soon to debug */
      namEntryDelete (entry);
      AJFREE (tabname);
    }
 
    namParseType = 0;
    db_input = -1;
    dbattr = 0;
  }

  return;
}

/* @func ajNamGetenv **********************************************************
**
** Looks for name as an environment variable.
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [AjPStr] character string find in hash table.
** @param [wP] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetenv (AjPStr name,
		    AjPStr* value) {
  char *envval;

  envval = getenv(ajStrStr(name));
  if (envval) {
    (void) ajStrAssC (value, envval);
    return ajTrue;
  }

  ajStrDel(value);
  return ajFalse;
}

/* @func ajNamGetValue ********************************************************
**
** Looks for name as an (upper case) environment variable,
** and then as-is in the hash table and if found returns 
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [AjPStr] character string find in hash table.
** @param [wP] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetValue (AjPStr name, AjPStr* value){
  return ajNamGetValueC (ajStrStr(name), value);
}

/* @func ajNamGetValueC *******************************************************
**
** Looks for name as an (upper case) environment variable,
** and then as-is in the hash table and if found returns 
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [char*] character string find in hash table.
** @param [wP] value [AjPStr*] Str for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetValueC (char* name, AjPStr* value){
  AjPNamStandards fnew = 0;
  static AjPStr namstr = NULL;
  AjBool ret = ajFalse;

  if (ajStrPrefixCO(name, namPrefixStr)) /* may already have the prefix */
    (void) ajStrAssC (&namstr, name);
  else {
    (void) ajStrAss (&namstr, namPrefixStr);
    (void) ajStrAppC (&namstr, name);
  }
  /* upper case for ENV, otherwise don't care */
  (void) ajStrToUpper (&namstr);

  /* first test for an ENV variable */

  ret = ajNamGetenv(namstr, value);
  if (ret) {
    return ajTrue;
  }

  /* then test the table definitions */

  fnew = ajTableGet(standardNames, ajStrStr(namstr));
  if (fnew) {
    (void) ajStrAss (value, fnew->value);
    return ajTrue;
  }

  return ajFalse;
}

/* @func ajNamDatabase **************************************************
**
** Looks for name in the hash table and if found returns 
** the attribute for this. If not found returns  NULL;
**
** @param [r] name [AjPStr] character string find in hash table.
** @return [AjBool] true if database name is valid.
** @error  NULL if name not found in the table
** @@
**
******************************************************************************/

AjBool ajNamDatabase(AjPStr name) {
  AjPNamStandards fnew = 0;

  /* ajDebug ("ajNamDatabase '%S'\n", name); */

  fnew = ajTableGet(standardNames, ajStrStr(name));
  if(fnew) {
    /* ajDebug ("  '%S' found\n", name); */
    return ajTrue;
  }

  /* ajDebug ("  '%S' not found\n", name); */
  return ajFalse;
}

/* @funcstatic namProcessFile *************************************************
**
** Read the definitions file and apend each token to the list.
**
** @param [P] file [FILE*] C file handle.
** @return [void]
** @@
******************************************************************************/

static void namProcessFile (FILE* file) {
  char line[512];
  char word[512];
  char *ptr;
  ajint i=0,j=0,len;
  char quote='\0';
  AjPList list1;
  AjPStr wordptr;

  list1 = ajListstrNew();

  /* Read in the settings. */
  while (fgets(line, 512, file)) {
    /*    (void) printf("%s",line);*/
    len = strlen(line);

    if(line[0] == '#') /* Ignore if the line is a comment */
      continue;

    /* now create a linked list of the "words" */
    if(len){
      ptr = &line[0];
      j=0;
      i=0;
      while(*ptr != '\n' && i < len){ 
	
	if(*ptr == ' ' || *ptr == '\t'){
	  if(j != 0) {
	    /*save word */
	    word[j] = '\0';
	    wordptr = ajStrNewC(word);
	    ajListstrPushApp(list1, wordptr); 
	    j = 0;
	  }
	  i++;ptr++;
	  continue;
	}
	else if(*ptr == '\'' || *ptr == '\"'){
	  word[j++] = *ptr;
	  if (quote)
	    quote = '\0';
	  else
	    quote = *ptr;
	}
	else if(!quote && j !=0 && *ptr == ']'){
	  word[j] = '\0';
	  wordptr = ajStrNewC(word);
	  ajListstrPushApp (list1, wordptr); 
	  j = 0;
	  wordptr = ajStrNewC("]");
	  ajListstrPushApp(list1, wordptr); 
	}
	else
	  word[j++] = *ptr;
	i++;ptr++;
      }
      if(j != 0) {
	/*save word */
	word[j] = '\0';
	wordptr = ajStrNewC(word);
	ajListstrPushApp(list1, wordptr); 
	j = 0;
      }
      
    }
  }

  namListParseOK = ajTrue;

  /* parse the linked list to */
  /*obtain ENV, DB and OPT values */
  (void) ajListstrMap(list1, (void (*)(AjPStr *,void *))namListParse, NULL);

  if (!namListParseOK)
    namUser("Unexpected end of file in %S\n", namRootStr);

  ajListstrFree(&list1);		/* Delete the linked list structure */

  namDebugEnvironmentals();

  return;
}

/* @func ajNamInit ************************************************************
**
** Initialise the variable and database definitions. Find the definition
** files and read them.
**
** @param [P] prefix [char*] Default prefix for all file and variable names.
** @return [void]
** @@
******************************************************************************/

void ajNamInit (char* prefix)
{
    char *prefixRoot;
    FILE *prefixRootFile;
    AjPStr prefixRootStr = NULL;
    AjPStr prefixStr = NULL;
    AjPStr prefixCap = NULL;

    /* create new table to hold the values */

    standardNames = ajStrTableNewCaseC(0);

    /* for each type of file read it and save the values */
    /* Start at system level then go to user */

    /* static namPrefixStr is the prefix for all variable names */

    namPrefixStr = ajStrNewC(prefix);
    (void) ajStrAppC(&namPrefixStr, "_");

    /* local prefixRoot is the root directory */
    /* it is the value of (PREFIX)_ROOT (if set) or namFixedRoot */

    prefixStr = ajStrNewC(prefix);
    (void) ajStrAppC(&prefixStr, "_ROOT");
    (void) ajStrToUpper (&prefixStr);

    (void) ajStrAppC(&prefixCap, prefix);
    (void) ajStrToUpper (&prefixCap);

    if (ajNamGetenv (prefixStr, &prefixRootStr))
	prefixRoot = ajStrStr(prefixRootStr);
    else
	prefixRoot = namFixedRoot;

    /* namFixedRootBase is the directory above the source root */

    ajStrAssC (&namFixedRootBaseStr, prefixRoot);
    ajFileDirUp (&namFixedRootBaseStr);

    /* look for $(PREFIX)_ROOT/../emboss.default */

    ajFmtPrintS (&namRootStr, "%s/share/%S/%s.default",
		 namInstallRoot, prefixCap, prefix);
    prefixRootFile = fopen (ajStrStr(namRootStr), "r");

    /* look for $(PREFIX)_ROOT/../emboss.default */

    if (!prefixRootFile)
    {					/* try original directory */
	ajFmtPrintS (&namRootStr, "%s/%s.default", prefixRoot, prefix);
	prefixRootFile = fopen (ajStrStr(namRootStr), "r");
    }

    if (namFileOrig)
	(void) ajStrAppC (&namFileOrig, ", ");
    (void) ajStrApp (&namFileOrig, namRootStr);

    if (prefixRootFile)
    {
	(void) ajStrAppC (&namFileOrig, "(OK)");
	namProcessFile(prefixRootFile);
	(void) fclose(prefixRootFile);
    }
    else
	(void) ajStrAppC (&namFileOrig, "(failed)");



    /* look for .embossrc in an arbitrary directory */

    prefixRoot= getenv("EMBOSSRC");

    if (prefixRoot)
    {
	(void) ajStrAssC(&namRootStr, prefixRoot);
	(void) ajStrAppC(&namRootStr, "/.");
	(void) ajStrAppC(&namRootStr, prefix);
	(void) ajStrAppC(&namRootStr, "rc");
	if (namFileOrig) (void) ajStrAppC (&namFileOrig, ", ");
	(void) ajStrApp (&namFileOrig, namRootStr);

	prefixRootFile = fopen (ajStrStr(namRootStr), "r");
	if (prefixRootFile)
	{
	    (void) ajStrAppC (&namFileOrig, "(OK)");
	    namProcessFile(prefixRootFile);
	    if(fclose(prefixRootFile))
		ajFatal("Closing prefixRootFile");
	}
	else
	    (void) ajStrAppC (&namFileOrig, "(failed)");
    }

    /* look for $HOME/.embossrc */
    
    prefixRoot= getenv("HOME");
    
    if (prefixRoot)
    {
	(void) ajStrAssC(&namRootStr, prefixRoot);
	(void) ajStrAppC(&namRootStr, "/.");
	(void) ajStrAppC(&namRootStr, prefix);
	(void) ajStrAppC(&namRootStr, "rc");
	if (namFileOrig) (void) ajStrAppC (&namFileOrig, ", ");
	(void) ajStrApp (&namFileOrig, namRootStr);
    
	prefixRootFile = fopen (ajStrStr(namRootStr), "r");
	if (prefixRootFile)
	{
	    (void) ajStrAppC (&namFileOrig, "(OK)");
	    namProcessFile(prefixRootFile);
	    if(fclose(prefixRootFile))
		ajFatal("Closing prefixRootFile");
	}
	else
	    (void) ajStrAppC (&namFileOrig, "(failed)");
    }
    

    ajStrDel(&prefixRootStr);
    ajStrDel(&prefixStr);
    ajStrDel(&prefixCap);

    return;
}

/* @funcstatic namNoColon *****************************************************
**
** Remove any trailing colon ':' in the input string.
**
** @param [P] thys [AjPStr*] String.
** @return [void]
** @@
******************************************************************************/

static void namNoColon (AjPStr* thys) {
  if (ajStrChar(*thys, -1) == ':')
    (void) ajStrTrim (thys, -1);

  return;
}

/* @funcstatic namDbAttr ******************************************************
**
** Return the index for an attribute name.
**
** @param [P] thys [AjPStr] Attribute name.
** @return [ajint] Index in namAttr, or 1 beyond the last known attribute
**               on failure.
** @@
******************************************************************************/

static ajint namDbAttr (AjPStr thys) {
  ajint i = 0;
  ajint j = 0;
  ajint ifound = 0;

  for (i=0; namAttr[i].Name; i++) {
    if (ajStrMatchC (thys, namAttr[i].Name))
	return i;
    if (ajStrPrefixCO (namAttr[i].Name, thys)) {
      ifound++;
      j = i;
    }
  }
  if (ifound == 1)
    return j;

  ajErr ("Bad database attribute '%S'\n", thys);
  return i;
}

/* @funcstatic namDbAttrC *****************************************************
**
** Return the index for an attribute name.
**
** @param [P] str [char*] Attribute name.
** @return [ajint] Index in namAttr, or 1 beyond the last known attribute
**               on failure.
** @@
******************************************************************************/

static ajint namDbAttrC (char* str) {
  ajint i = 0;
  ajint j = 0;
  ajint ifound = 0;

  for (i=0; namAttr[i].Name; i++) {
    if (!strcmp (str, namAttr[i].Name))
	return i;
    if (ajStrPrefixCC (namAttr[i].Name, str)) {
      ifound++;
      j = i;
    }
  }
  if (ifound == 1)
    return j;

  ajErr ("Bad database attribute '%s'\n", str);
  return i;
}

/* @func ajNamExit ************************************************************
**
** Delete the initialisation values in the table.
** @return [void]
** @@
******************************************************************************/

void ajNamExit (void){

  namListStandardsDelete();       /* Delete elements from database structure */
  ajTableFree(&standardNames); /* free table and database structures */

  return;
}

/* @func ajNamDbTest ****************************************************
**
** Looks for a database name in the known definitions.
**
** @param [P] dbname [AjPStr] Database name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamDbTest (AjPStr dbname) {
  AjPNamStandards data;

  data = ajTableGet(standardNames, ajStrStr(dbname));
  if (!data)
    return ajFalse;
  if (data->type != TYPE_DB)
    return ajFalse;

  return ajTrue;
}

/* @func ajNamDbGetUrl **************************************************
**
** Gets the URL definition for a database definition.
**
** @param [P] dbname [AjPStr] Database name.
** @param [P] url [AjPStr*] URL returned.
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbGetUrl (AjPStr dbname, AjPStr* url) {

  AjPNamStandards data;
  AjPStr* dbattr;
  static ajint calls = 0;
  static ajint iurl = 0;
  if (!calls) {
    iurl = namDbAttrC("url");
    calls = 1;
  }
  data = ajTableGet(standardNames, ajStrStr(dbname));
  if (!data)
    ajFatal ("%S is not a known database\n", dbname);

  if (data->type != TYPE_DB)
    ajFatal ("%S is not a database\n", dbname);

  dbattr = data->data;

  if (ajStrLen(dbattr[iurl])) {
    (void) ajStrAss (url, dbattr[iurl]);
    return ajTrue;
  }

  return ajFalse;
}
  
/* @func ajNamDbGetDbalias **********************************************
**
** Gets an alias name for a database.
**
** @param [P] dbname [AjPStr] Database name.
** @param [P] dbalias [AjPStr*] Alias returned.
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbGetDbalias (AjPStr dbname, AjPStr* dbalias) {

  AjPNamStandards data;
  AjPStr* dbattr;
  static ajint calls = 0;
  static ajint idba = 0;
  if (!calls) {
    idba = namDbAttrC("dbalias");
    calls = 1;
  }
  data = ajTableGet(standardNames, ajStrStr(dbname));
  if (!data)
    ajFatal ("%S is not a known database\n", dbname);

  if (data->type != TYPE_DB)
    ajFatal ("%S is not a database\n", dbname);

  dbattr = data->data;

  if (ajStrLen(dbattr[idba])) {
    (void) ajStrAss (dbalias, dbattr[idba]);
    return ajTrue;
  }

  return ajFalse;
}
  
/* @func ajNamDbQuery *********************************************************
**
** Given a query with database name and search fields,
** fill in the access method and some common fields according
** to the query level.
**
** @param [u] qry [AjPSeqQuery] Query structure with at least dbname filled in
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbQuery (AjPSeqQuery qry) {

  AjPNamStandards data;

  AjPStr* dbattr;

  data = ajTableGet(standardNames, ajStrStr(qry->DbName));

  if (!data || (data->type != TYPE_DB))
    ajFatal ("database %S unknown\n", qry->DbName);

  dbattr = data->data;
 
  /* general defaults */

  (void) namDbSetAttr(dbattr, "type", &qry->DbType);
  (void) namDbSetAttr(dbattr, "method", &qry->Method);
  (void) namDbSetAttr(dbattr, "format", &qry->Formatstr);
  (void) namDbSetAttr(dbattr, "app", &qry->Application);
  (void) namDbSetAttr(dbattr, "directory", &qry->IndexDir);
  (void) namDbSetAttr(dbattr, "indexdirectory", &qry->IndexDir);
  (void) namDbSetAttr(dbattr, "indexdirectory", &qry->Directory);
  (void) namDbSetAttr(dbattr, "directory", &qry->Directory);
  (void) namDbSetAttr(dbattr, "exclude", &qry->Exclude);
  (void) namDbSetAttr(dbattr, "filename", &qry->Filename);

  if (!ajSeqQueryIs(qry)) { /* must have a method for all entries */

    (void) namDbSetAttr(dbattr, "methodall", &qry->Method);
    (void) namDbSetAttr(dbattr, "formatall", &qry->Formatstr);
    (void) namDbSetAttr(dbattr, "appall", &qry->Application);
    qry->Type = QRY_ALL;

  }

  else { /* must be able to query the database */

    (void) namDbSetAttr(dbattr, "methodquery", &qry->Method);
    (void) namDbSetAttr(dbattr, "formatquery", &qry->Formatstr);
    (void) namDbSetAttr(dbattr, "appquery", &qry->Application);

    if (!ajSeqQueryWild (qry)) { /* ID - single entry may be available */

      (void) namDbSetAttr(dbattr, "methodentry", &qry->Method);
      (void) namDbSetAttr(dbattr, "formatentry", &qry->Formatstr);
      (void) namDbSetAttr(dbattr, "appentry", &qry->Application);
      qry->Type = QRY_ENTRY;
    }
    else {
      qry->Type = QRY_QUERY;
    }
  }

  /*
  ajDebug ("ajNamDbQuery DbName '%S'\n", qry->DbName);
  ajDebug ("    Id '%S' Acc '%S' Des '%S'\n", qry->Id, qry->Acc, qry->Des);
  ajDebug ("    Method      '%S'\n", qry->Method);
  ajDebug ("    Formatstr   '%S'\n", qry->Formatstr);
  ajDebug ("    Application '%S'\n", qry->Application);
  ajDebug ("    IndexDir    '%S'\n", qry->IndexDir);
  ajDebug ("    Directory   '%S'\n", qry->Directory);
  ajDebug ("    Filename    '%S'\n", qry->Filename);
  */

  if (!ajStrLen(qry->Formatstr)) {
    ajErr("No format defined for database '%S'", qry->DbName);
    return ajFalse;
  }
    
  if (!ajStrLen(qry->Method)) {
    ajErr("No access method for database '%S'", qry->DbName);
    return ajFalse;
  }

  return ajTrue;
}

/* @funcstatic namDbSetAttr ***************************************************
**
** Sets a named attribute value from an attribute list.
**
** @param [P] dbattr [AjPStr*] Attribute definitions.
** @param [P] attrib [char*] Attribute name.
** @param [P] qrystr [AjPStr*] Returned attribute value.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool namDbSetAttr (AjPStr* dbattr, char* attrib, AjPStr* qrystr) {

  ajint i = namDbAttrC(attrib);

  if (!i) {
    ajFatal("unknown attribute '%s' requested",  attrib);
  }

  if (!ajStrLen(dbattr[i])) return ajFalse;

  (void) ajStrAss (qrystr, dbattr[i]);
  /* ajDebug("namDbSetAttr('%S')\n", *qrystr); */

  (void) namVarResolve (qrystr);

  return ajTrue;
}

/* @funcstatic namVarResolve **************************************************
**
** Resolves any variable or function references in a string.
** Yet to be implemented, but called in the right places.
**
** @param [uP] var [AjPStr*] String value
** @return [AjBool] Always ajTrue so far
** @@
******************************************************************************/

static AjBool namVarResolve (AjPStr* var) {

  static AjPRegexp varexp = NULL;
  AjPStr varname = NULL;
  AjPStr newvar = NULL;
  AjPStr restvar = NULL;

  if (!varexp) varexp = ajRegCompC("^\\$([a-zA-Z0-9_.]+)");

  while (ajRegExec (varexp, *var)) {

    ajRegSubI(varexp, 1, &varname); /* variable name */

    (void) ajNamGetValue(varname, &newvar);

    ajDebug("namVarResolve '%S' = '%S'\n", varname, newvar);

    if (ajRegPost(varexp, &restvar)) /* any more? */
      (void) ajStrApp (&newvar, restvar);
    (void) ajStrAss (var, newvar);
  }

  ajStrDel(&varname);
  ajStrDel(&newvar);
  ajStrDel(&restvar);

  return ajFalse;
}

/* @funcstatic namUser ********************************************************
**
** Formatted write as an error message.
**
** @param [P] fmt [char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void namUser (char* fmt, ...) {  va_list args ;

  if (!namdebug) return;
  va_start (args, fmt) ;
  ajFmtVError(fmt, args) ;
  va_end (args) ;

  return;
}

/* @func ajNamRootInstall *****************************************************
**
** Returns the insatll directory root for all file searches
** (package level)
**
** @param [P] root [AjPStr*] Root.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRootInstall (AjPStr* root) {
  (void) ajStrAssC (root, namInstallRoot);

  return ajTrue;
}

/* @func ajNamRootPack *****************************************************
**
** Returns the package name for the library
**
** @param [P] pack [AjPStr*] Package name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRootPack (AjPStr* pack) {
  (void) ajStrAssC (pack, namPackage);

  return ajTrue;
}

/* @func ajNamRoot ******************************************************
**
** Returns the directory for all file searches
** (package level)
**
** @param [P] root [AjPStr*] Root.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRoot (AjPStr* root) {
  (void) ajStrAssC (root, namFixedRoot);

  return ajTrue;
}

/* @func ajNamRootBase ******************************************************
**
** Returns the base directory for all for all file searches
** (above package level).
**
** @param [P] rootbase [AjPStr*] Root.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRootBase (AjPStr* rootbase) {
  (void) ajStrAssS (rootbase, namFixedRootBaseStr);

  return ajTrue;
}

/* @func ajNamResolve ***************************************************
**
** Resolves a variable name if the input string starts with a dollar sign.
**
** @param [P] name [AjPStr*] String
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamResolve (AjPStr* name) {

  static AjPRegexp namexp = 0;
  AjPStr varname = NULL;
  AjPStr varvalue = NULL;
  AjPStr restname = NULL;
  AjBool ret;

  if (!namexp)
    namexp = ajRegCompC ("^\\$([A-Za-z0-9_]+)");

  ajDebug ("ajNamResolve of '%S'\n", *name);
  ret = ajRegExec (namexp, *name);
  if (ret) {
    ajRegSubI(namexp, 1, &varname);
    ajDebug ("variable '%S' found\n", varname);
    (void) ajRegPost(namexp, &restname);
    ret = ajNamGetValue (varname, &varvalue);
    if (ret) {
      (void) ajStrAss (name, varvalue);
      (void) ajStrApp (name, restname);
      ajDebug ("converted to '%S'\n", *name);
    }
    else {
      ajDebug ("Variable unknown '$%S'\n", varname);
      ajWarn ("Variable unknown in '%S'", *name);
    }
    ajStrDel(&varname);
    ajStrDel(&varvalue);
    ajStrDel(&restname);
  }

  return ret;
}

/* @func ajNamUnused *******************************************************
**
** Dummy function to prevent compiler warnings
**
** @return [void]
** @@
******************************************************************************/

void ajNamUnused(void)
{
    namDebugDatabases();
    return;
}
