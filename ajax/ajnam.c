/******************************************************************************
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
******************************************************************************/

#include "ajax.h"

#include <dirent.h>
#include <unistd.h>

enum NamEType
{
    TYPE_UNKNOWN,			/* no type set */
    TYPE_ENV,				/* env or set variable */
    TYPE_DB,				/* database definition */
    TYPE_RESOURCE,			/* resource definition */
    TYPE_IFILE				/* include filename */
};

#define NAM_INCLUDE_ESTIMATE 5	/* estimate of maximum number of include */
                                /* statements in emboss.default          */

/* Scope values for entry methods returned from nameMethod2Scope */
#define METHOD_ENTRY 1
#define METHOD_QUERY 2
#define METHOD_ALL   4
#define SLOW_ENTRY 8
#define SLOW_QUERY 16

static AjBool namDoDebug  = AJFALSE;
static AjBool namDoValid  = AJFALSE;
static AjBool namDoHomeRc = AJTRUE;
static AjPStr namRootStr  = NULL;

static AjBool namListParseOK = AJFALSE;

char* namTypes[] = { "unknown", "SET", "DBNAME", "RESOURCE", "INCLUDE" };

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

#ifdef VERSION
static char namVersion[] = VERSION;
#else
static char namVersion[] = "1.x";
#endif

static AjPStr namFixedRootBaseStr = NULL;
static AjPStr namPrefixStr        = NULL;
static AjPStr namFileOrig         = NULL;

static AjPTable namMasterTable = NULL;
static ajint namParseType      = 0;
static AjPFile namFile         = NULL;
static ajint namLine           = 0;
static ajint namErrorCount     = 0;

static AjPRegexp namNameExp = 0;
static AjPRegexp namVarExp  = NULL;




/* @datastatic NamPAttr *******************************************************
**
** Resource attribute definition structure
**
** @alias NamSAttr
** @alias NamOAttr
**
** @attr Name [char*] Attribute name
** @attr Defval [char*] Default value, usually an empty string
** @attr Comment [char*] Comment for documentation purposes
** @@
******************************************************************************/

typedef struct NamSAttr
{
    char* Name;
    char* Defval;
    char* Comment;
} NamOAttr;

#define NamPAttr NamOAttr*




/* @datastatic NamPValid ******************************************************
**
** Resource attribute validation structure
**
** @alias NamSValid
** @alias NamOValid
**
** @attr Name [char*] Attribute name
** @attr Comment [char*] Comment for documentation purposes
** @@
******************************************************************************/

typedef struct NamSValid
{
    char* Name;
    char* Comment;
} NamOValid;

#define NamPValid NamOValid*




NamOAttr namDbAttrs[] =
{
    {"name", "", "database name (required)"},
    {"format", "", "database entry format (required, at some level)"},
    {"method", "", "access method (required, at some level)"},
    {"type", "", "database type 'Nucleotide', 'Protein', etc (required)"},

    {"app", "", "external application commandline (APP, EXTERNAL)"},
    {"appall", "", "external commandline for 'methodall' (APP, EXTERNAL)"},
    {"appentry", "", "external commandline for 'methodentry' (APP, EXTERNAL)"},
    {"appquery", "", "external commandline for 'methodquery' (APP, EXTERNAL)"},

    {"command", "", "command line to return entry/ies"},
    {"comment", "", "text comment for the DB definition"},
    {"dbalias", "", "database name to be used by access method if different"},
    {"description", "", "short database description"},
    {"directory", "", "data directory"},
    {"exclude", "", "wildcard filenames to exclude from 'filename'"},
    {"fields", "", "extra database fields available, ID and ACC are standard"},
    {"filename", "", "(wildcard) database filename"},

    {"formatall", "", "database entry format for 'methodall' access"},
    {"formatentry", "", "database entry format for 'methodentry' access"},
    {"formatquery", "", "database entry format for 'methodall' access"},

    {"httpversion", "", "HTTP version for GET requests (URL, SRSWWW)"},
    {"identifier", "", "standard identifier (defaults to name)"},
    {"indexdirectory", "", "Index directory, defaults to data 'directory'"},

    {"methodall", "", "access method for all entries"},
    {"methodentry", "","access method for single entry"},
    {"methodquery", "", "access method for query (several entries)"},

    {"proxy", "", "http proxy server, or ':' to cancel a global proxy "
	          "(URL, SRSWWW)"},
    {"release", "", "release of the database, comment only"},
    {"url", "", "URL skeleton for entry level access (URL, SRSWWW)"},
    {NULL, NULL, NULL}
};

NamOAttr namRsAttrs[] =
{
    {"name", "", "resource name (required)"},
    {"type", "", "resource type (required)"},

    {"identifier", "", "standard identifier (defaults to name)"},
    {"release", "", "release of the resource"},
    {NULL, NULL, NULL}
};

NamOValid namDbTypes[] =
{
    {"N", "Nucleotide (obsolete short name)"},
    {"P", "Protein (obsolete short name)"},
    {"Nucleotide", "Nucleotide sequence data"},
    {"Protein", "Protein sequence data"},
    {"Pattern", "Pattern data"},
    {NULL, NULL}
};

NamOValid namRsTypes[] =
{
    {"Blast", "Blast database file"},
    {NULL, NULL}
};




/* @datastatic NamPEntry ******************************************************
**
** Internal database standard structure
**
** @alias NamSEntry
** @alias NamOEntry
**
** @attr name [AjPStr] token name
** @attr value [AjPStr] token value
** @attr type [ajint] token type enumerated TYPE_ENV
**                    TYPE_DB TYPE_RESOURCE TYPE_IFILE
** @attr scope [ajint] token scope always zero
** @attr data [void*] Attribute names and values for databases
** @@
******************************************************************************/

typedef struct NamSEntry
{
    AjPStr name;
    AjPStr value;
    ajint type;
    ajint scope;
    void* data;		  /* Attribute values for databases */
} NamOEntry;

#define NamPEntry NamOEntry*




static ajint  namDbAttr(const AjPStr thys);
static ajint  namDbAttrC(const char* str);
static AjBool namDbSetAttr(const AjPStr* dbattr, const char* attrib,
			   AjPStr* qrystr);
static void   namDebugDatabase(const AjPStr* dbattr);
static void   namDebugResource(const AjPStr* dbattr);
static void   namDebugVariables(void);
static void   namDebugMaster(ajint which);
static void   namEntryDelete(NamPEntry* pentry);
static void   namError(const char* fmt, ...);
static void   namListParse(AjPList listwords, AjPList listcount,
			   AjPFile file);
static void   namListMaster(ajint which);
static void   namListMasterDelete(void);
static ajint  namMethod2Scope(const AjPStr method);
static void   namNoColon(AjPStr *thys);
static void   namPrintDatabase(const AjPStr* dbattr);
static void   namPrintResource(const AjPStr* rsattr);
static AjBool namProcessFile(AjPFile file);
static ajint  namRsAttr(const AjPStr thys);
static ajint  namRsAttrC(const char* str);
static void   namUser(const char *fmt, ...);
static AjBool namValid(const NamPEntry entry);
static AjBool namValidDatabase(const NamPEntry entry);
static AjBool namValidResource(const NamPEntry entry);
static AjBool namValidVariable(const NamPEntry entry);
static AjBool namVarResolve(AjPStr* var);




/* @funcstatic namEntryDelete *************************************************
**
** Deletes a variable, database, or resource entry from the internal table.
**
** @param [r] pentry [NamPEntry*] The entry to be deleted.
** @return [void]
** @@
******************************************************************************/

static void namEntryDelete(NamPEntry* pentry)	
{

    ajint j;
    AjPStr* attrs;
    NamPEntry entry;

    entry = *pentry;

    ajStrDel(&entry->name);
    ajStrDel(&entry->value);

    if(entry->type == TYPE_DB)
    {
	attrs = entry->data;
	for(j=0; namDbAttrs[j].Name; j++)
	    ajStrDel(&attrs[j]);
	AJFREE(entry->data);
    }

    if(entry->type == TYPE_RESOURCE)
    {
	attrs = entry->data;
	for(j=0; namRsAttrs[j].Name; j++)
	    ajStrDel(&attrs[j]);
	AJFREE(entry->data);
    }
    else if(entry->type == TYPE_ENV)
    {
    }

    AJFREE(entry);

    return;
}




/* @funcstatic namListMasterDelete *****************************************
**
** Deletes all databases in the internal table. The table is converted to
** an array, and each entry in turn is passed to namEntryDelete.
**
** @return [void]
** @@
******************************************************************************/

static void namListMasterDelete(void)
{
    ajint i;
    NamPEntry fnew = 0;
    void **array;


    array = ajTableToarray(namMasterTable, NULL);

    for(i = 0; array[i]; i += 2)
    {
	AJFREE(array[i]);		/* the key */
	fnew = (NamPEntry) array[i+1];
	namEntryDelete(&fnew);
    }
    AJFREE(array);

    return;
}




/* @funcstatic namPrintDatabase ***********************************************
**
** Prints a report of defined attributes for a database definition.
**
** @param [r] dbattr [const AjPStr*] Attribute list from a database entry.
** @return [void]
** @@
******************************************************************************/

static void namPrintDatabase(const AjPStr* dbattr)
{
    ajint i;

    for(i=0; namDbAttrs[i].Name; i++)
	if(ajStrLen(dbattr[i]))
	    ajUser("\t%s: %S", namDbAttrs[i].Name, dbattr[i]);

    return;
}




/* @funcstatic namPrintResource ***********************************************
**
** Prints a report of defined attributes for a resource definition.
**
** @param [r] rsattr [const AjPStr*] Attribute list from a resource entry.
** @return [void]
** @@
******************************************************************************/

static void namPrintResource(const AjPStr* rsattr)
{
    ajint i;

    for(i=0; namRsAttrs[i].Name; i++) 
	if(ajStrLen(rsattr[i]))
	    ajUser("\t%s: %S", namRsAttrs[i].Name, rsattr[i]);

    return;
}




/* @func ajNamPrintDbAttr *****************************************************
**
** Prints a report of the database attributes available (for entrails)
**
** @param [r] outf [const AjPFile] Output file
** @param [r] full [AjBool] Full output if AjTrue
** @return [void]
******************************************************************************/

void ajNamPrintDbAttr(const AjPFile outf, AjBool full)
{
    ajint i;

    ajFmtPrintF(outf, "# Database attributes\n");
    ajFmtPrintF(outf, "%12s %10s %s\n", "Attribute", "default", "Comment");
    ajFmtPrintF(outf, "namDbAttrs {\n");
    for(i=0; namDbAttrs[i].Name; i++)
	ajFmtPrintF(outf, "%12s %10s %s\n",
		     namDbAttrs[i].Name, namDbAttrs[i].Defval,
		     namDbAttrs[i].Comment);

    ajFmtPrintF(outf, "}\n");

    return;
}




/* @func ajNamPrintRsAttr *****************************************************
**
** Prints a report of the resource attributes available (for entrails)
**
** @param [r] outf [const AjPFile] Output file
** @param [r] full [AjBool] Full output if AjTrue
** @return [void]
******************************************************************************/

void ajNamPrintRsAttr(const AjPFile outf, AjBool full)
{
    ajint i;

    ajFmtPrintF(outf, "# Resource attributes\n");
    ajFmtPrintF(outf, "%12s %10s %s\n", "Attribute", "default", "Comment");
    ajFmtPrintF(outf, "namRsAttrs {\n");
    for(i=0; namRsAttrs[i].Name; i++)
	ajFmtPrintF(outf, "%12s %10s %s\n",
		     namRsAttrs[i].Name, namRsAttrs[i].Defval,
		     namRsAttrs[i].Comment);

    ajFmtPrintF(outf, "}\n");

    return;
}




/* @funcstatic namDebugDatabase ***********************************************
**
** Prints a report of defined attributes for a database definition.
**
** @param [r] dbattr [const AjPStr*] Attribute list from a database entry.
** @return [void]
** @@
******************************************************************************/

static void namDebugDatabase(const AjPStr* dbattr)
{
    ajint i;

    for(i=0; namDbAttrs[i].Name; i++)
	if(ajStrLen(dbattr[i]))
	    ajDebug("\t%s: %S\n", namDbAttrs[i].Name, dbattr[i]);

    return;
}




/* @funcstatic namDebugResource ***********************************************
**
** Prints a report of defined attributes for a resource definition.
**
** @param [r] rsattr [const AjPStr*] Attribute list from a database entry.
** @return [void]
** @@
******************************************************************************/

static void namDebugResource(const AjPStr* rsattr)
{
    ajint i;

    for(i=0; namRsAttrs[i].Name; i++)
	if(ajStrLen(rsattr[i]))
	    ajDebug("\t%s: %S\n", namRsAttrs[i].Name, rsattr[i]);

    return;
}




/* @funcstatic namListMaster **************************************************
**
** Lists databases or variables defined in the internal table.
**
** @param [r] which [ajint] Variable type, either TYPE_ENV for environment
**                        variables or TYPE_DB for databases or
**                        TYPE_RESOURCE for resources.
** @return [void]
** @@
******************************************************************************/

static void namListMaster(ajint which)
{
    ajint i;
    NamPEntry fnew;
    void **array;
    char *key;

    array = ajTableToarray(namMasterTable, NULL);

    for(i = 0; array[i]; i += 2)
    {
	fnew =(NamPEntry) array[i+1];
	key = (char*) array[i];
	if(fnew->type == which)
	{
	    if(TYPE_DB == which)
	    {
		ajUser("DB %S\t *%s*", fnew->name, key);
		namPrintDatabase(fnew->data);
		ajUser("");
	    }
	    else if(TYPE_RESOURCE == which) 
	    {
		ajUser("RES %S\t *%s*", fnew->name, key);
		namPrintResource(fnew->data);
		ajUser("");
	    }
	    else if(TYPE_ENV == which)
	    {
		ajUser("ENV %S\t%S\t *%s*",fnew->name,fnew->value,key);
	    }
	}
    }
    AJFREE(array);

    return;
}




/* @funcstatic namDebugMaster **********************************************
**
** Lists databases or variables defined in the internal table.
**
** @param [r] which [ajint] Variable type, either TYPE_ENV for environment
**                        variables or TYPE_DB for databases or
**                        TYPE_RESOURCE for resources.
** @return [void]
** @@
******************************************************************************/

static void namDebugMaster(ajint which)
{
    ajint i;
    NamPEntry fnew;
    void **array;
    char *key;

    array = ajTableToarray(namMasterTable, NULL);

    for(i = 0; array[i]; i += 2)
    {
	fnew = (NamPEntry) array[i+1];
	key = (char*) array[i];
	if(fnew->type == which)
	{
	    if(TYPE_DB == which)
	    {
		ajDebug("DB %S\t *%s*\n", fnew->name, key);
		namDebugDatabase(fnew->data);
		ajDebug("\n");
	    }

	    if(TYPE_RESOURCE == which)
	    {
		ajDebug("RES %S\t *%s*\n", fnew->name, key);
		namDebugResource(fnew->data);
		ajDebug("\n");
	    }
	    else if(TYPE_ENV == which)
	    {
		ajDebug("ENV %S\t%S\t *%s*\n",fnew->name,fnew->value,key);
	    }
	}
    }
    AJFREE(array);

    return;
}




/* @func ajNamDbDetails *******************************************************
**
** Returns database access method information
**
** @param [r] name [const AjPStr] Database name
** @param [w] type [AjPStr*] sequence type - 'P' or 'N'
** @param [w] id [AjBool*] ajTrue = can access single entries
** @param [w] qry [AjBool*] ajTrue = can access wild/query entries
** @param [w] all [AjBool*] ajTrue = can access all entries
** @param [w] comment [AjPStr*] comment about database
** @param [w] release [AjPStr*] database release date
** @return [AjBool] ajTrue if database details were found
** @@
******************************************************************************/

AjBool ajNamDbDetails(const AjPStr name, AjPStr* type, AjBool* id,
		      AjBool* qry, AjBool* all,
		      AjPStr* comment, AjPStr* release)
{
    NamPEntry fnew = 0;
    AjPStr* dbattr = NULL;
    ajint i;
    ajint scope;
    
    *id = *qry = *all = ajFalse;
    
    ajStrDelReuse(type);
    ajStrDelReuse(comment);
    ajStrDelReuse(release);
    
    fnew = ajTableGet(namMasterTable, ajStrStr(name));
    if(fnew)
    {
	/* ajDebug("  '%S' found\n", name); */
	
	dbattr = fnew->data;
	for(i=0; namDbAttrs[i].Name; i++)
	{
	    /* ajDebug("Attribute name = %s, value = %S\n",
	       namDbAttrs[i].Name, dbattr[i]); */
	    if(ajStrLen(dbattr[i]))
	    {
		if(!strcmp("type", namDbAttrs[i].Name))
		    ajStrAss(type, dbattr[i]);

		if(!strcmp("method", namDbAttrs[i].Name))
		{
		    scope = namMethod2Scope(dbattr[i]);
		    if(scope & METHOD_ENTRY) *id = ajTrue;
		    if(scope & METHOD_QUERY) *qry = ajTrue;
		    if(scope & METHOD_ALL) *all = ajTrue;
		}

		if(!strcmp("methodentry", namDbAttrs[i].Name))
		{
		    scope = namMethod2Scope(dbattr[i]);
		    if(scope & METHOD_ENTRY) *id = ajTrue;
		}

		if(!strcmp("methodquery", namDbAttrs[i].Name))
		{
		    scope = namMethod2Scope(dbattr[i]);
		    if(scope & METHOD_ENTRY) *id = ajTrue;
		    if(scope & METHOD_QUERY) *qry = ajTrue;
		}

		if(!strcmp("methodall", namDbAttrs[i].Name))
		{
		    scope = namMethod2Scope(dbattr[i]);
		    if(scope & METHOD_ALL) *all = ajTrue;
		}

		if(!strcmp("comment", namDbAttrs[i].Name))
		    ajStrAss(comment, dbattr[i]);

		if(!strcmp("release", namDbAttrs[i].Name))
		    ajStrAss(release, dbattr[i]);
	    }
	}
	
	if(!ajStrLen(*type))
	{
	    ajWarn("Bad database definition for %S: No type. 'P' assumed",
		   name);
	    ajStrAssC(type, "P");
	}

	if(!*id && !*qry && !*all)
	    ajWarn("Bad database definition for %S: No method(s) for access",
		   name);
	
	return ajTrue;
    }
    
    ajDebug("  '%S' not found\n", name);

    return ajFalse;
}




/* @funcstatic namMethod2Scope ************************************************
**
** Returns OR'ed values of METHOD_ENTRY, METHOD_QUERY and METHOD_ALL
** for the various types of access method for databases.
**
** @param [r] method [const AjPStr] Access method string
** @return [ajint] OR'ed values for the valid scope of the access method given
** @@
******************************************************************************/
static ajint namMethod2Scope(const AjPStr method)
{

    ajint result = 0;

    if(!ajStrCmpC(method, "emblcd"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "srs"))
	result = (METHOD_ENTRY | METHOD_QUERY);
    else if(!ajStrCmpC(method, "srsfasta"))
	result = (METHOD_ENTRY | METHOD_QUERY);
    else if(!ajStrCmpC(method, "srswww"))
	result = METHOD_ENTRY;
    else if(!ajStrCmpC(method, "url"))
	result = METHOD_ENTRY;
    else if(!ajStrCmpC(method, "app"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "external"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "direct"))
	result = (METHOD_ALL | SLOW_QUERY | SLOW_ENTRY );
    else if(!ajStrCmpC(method, "blast"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "gcg"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    /* not in ajseqdb seqAccess list */
    /*
       else if(!ajStrCmpC(method, "corba"))
       result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
       */

    return result;
}




/* @func ajNamListOrigin ******************************************************
**
** Writes a simple list of where the internal tables came from..
**
** @return [void]
** @@
******************************************************************************/

void ajNamListOrigin(void)
{
    ajUser("SOURCE---------->");
    ajUser("%S", namFileOrig);
    ajUser("SOURCE---------->");
    ajUser("");

    return;
}




/* @func ajNamDebugOrigin *****************************************************
**
** Writes a simple list of where the internal tables came from..
**
** @return [void]
** @@
******************************************************************************/

void ajNamDebugOrigin(void)
{
    ajDebug("Defaults and .rc files: %S\n", namFileOrig);

    return;
}




/* @func ajNamListDatabases ***************************************************
**
** Writes a simple list of all databases in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamListDatabases(void)
{
    ajUser("DB---------->");
    namListMaster(TYPE_DB);
    ajUser("DB---------->");
    ajUser("");

    return;
}




/* @func ajNamDebugDatabases **************************************************
**
** Writes a simple debug report of all databases in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamDebugDatabases(void)
{
    ajDebug("DB databases\n");
    ajDebug("============\n");
    namDebugMaster(TYPE_DB);
    ajDebug("[DB done]\n\n");

    return;
}




/* @func ajNamDebugResources **************************************************
**
** Writes a simple debug report of all databases in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamDebugResources(void)
{
    ajDebug("RES resources\n");
    ajDebug("=============\n");
    namDebugMaster(TYPE_RESOURCE);
    ajDebug("[RES done]\n\n");

    return;
}




/* @func ajNamDebugVariables *********************************************
**
** Writes a simple debug report of all envornment variables
** in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamDebugVariables(void)
{
    ajDebug("ENV variables\n");
    ajDebug("=============\n");
    namDebugMaster(TYPE_ENV);
    ajDebug("[ENV done]\n\n");

    return;
}




/* @func ajNamListListDatabases ***********************************************
**
** Creates a AjPList list of all databases in the internal table.
**
** @param [w] dbnames [AjPList] Str List of names to be populated
** @return [void]
** @@
******************************************************************************/

void ajNamListListDatabases(AjPList dbnames)
{
    ajint i;
    NamPEntry fnew;
    void **array;

    array = ajTableToarray(namMasterTable, NULL);

    for(i = 0; array[i]; i += 2)
    {
	fnew = (NamPEntry) array[i+1];
	if(fnew->type == TYPE_DB)
	{
	    ajDebug("DB: %S\n", fnew->name);
	    ajListstrPushApp(dbnames, fnew->name);
	}
    }
    AJFREE(array);

    return;
}




/* @func ajNamListListResources ***********************************************
**
** Creates a AjPList list of all databases in the internal table.
**
** @param [w] rsnames [AjPList] Str List of names to be populated
** @return [void]
** @@
******************************************************************************/

void ajNamListListResources(AjPList rsnames)
{
    ajint i;
    NamPEntry fnew;
    void **array;

    array = ajTableToarray(namMasterTable, NULL);

    for(i = 0; array[i]; i += 2)
    {
	fnew = (NamPEntry) array[i+1];
	if(fnew->type == TYPE_RESOURCE)
	{
	    ajDebug("RES: %S\n", fnew->name);
	    ajListstrPushApp(rsnames, fnew->name);
	}
    }
    AJFREE(array);

    return;
}




/* @func ajNamVariables **************************************************
**
** Writes a simple list of all variables in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamVariables(void)
{
    ajUser("ENV---------->");
    namListMaster(TYPE_ENV);
    ajUser("ENV---------->");
    ajUser("");

    return;
}




/* @funcstatic namDebugVariables *****************************************
**
** Writes a simple list of all variables in the internal table.
**
** @return [void]
** @@
******************************************************************************/

static void namDebugVariables(void)
{
    namUser("ENV---------->\n");
    namDebugMaster(TYPE_ENV);
    namUser("ENV---------->\n");
    namUser("\n");

    return;
}




/* @funcstatic namListParse ***************************************************
**
** Parse the text in a list of tokens read from the input file.
** Derive environment variable and database definitions. Store
** all these in the internal tables.
**
** @param [r] listwords [AjPList] String list of word tokens to parse
** @param [r] listcount [AjPList] List of word counts per line for
**                                generating error messages
** @param [r] file [AjPFile] Input file for messages
** @return [void]
** @@
******************************************************************************/

static void namListParse(AjPList listwords, AjPList listcount,
			  AjPFile file)
{
    static char* tabname   = 0;
    static AjPStr name     = 0;
    static AjPStr value    = 0;
    static char quoteopen  = 0;
    static char quoteclose = 0;
    static AjPStr* dbattr  = 0;
    static AjPStr* rsattr  = 0;
    static ajint  db_input = -1;
    static ajint  rs_input = -1;

    NamPEntry fnew  = NULL;
    NamPEntry entry = NULL;

    AjBool dbsave = ajFalse;
    AjBool rssave = ajFalse;
    AjBool saveit = ajFalse;
    ajint ndbattr = 0;
    ajint nrsattr = 0;

    AjPStr includefn = NULL;
    AjPFile iinf     = NULL;
    AjPStr key       = NULL;
    AjPStr val       = NULL;

    static AjPTable Ifiles = NULL;
    AjPStr curword;

    ajint wordcount = 0;
    ajint linecount = 0;
    ajint lineword  = 0;
    ajint *iword    = NULL;
    AjBool namstatus;
    
    /* ndbattr = count database attributes */
    if(!ndbattr)
	for(ndbattr=0; namDbAttrs[ndbattr].Name; ndbattr++);

    /* nrsattr = count resource attributes */
    if(!nrsattr)
	for(nrsattr=0; namRsAttrs[nrsattr].Name; nrsattr++);
    
    ajStrDel(&name);
    ajStrDel(&value);
    quoteopen  = 0;
    quoteclose = 0;
    
    namLine = 1;
    namUser("namListParse of %F words: %d lines: %d\n", 
	    file, ajListLength(listwords), ajListLength(listcount));
    
    while(ajListstrPop(listwords, &curword))
    {
	while(ajListLength(listcount) && (lineword < wordcount))
	{
	    namUser("ajListPop %d < %d list %d\n",
		    lineword, wordcount, ajListLength(listcount));
	    ajListPop(listcount, (void**) &iword);
	    lineword = *iword;
	    linecount++;
	    namLine = linecount-1;
	    AJFREE(iword);
	}
	wordcount++;
	namUser("namListParse word: %d line: %d (%d) <%S>\n",
		wordcount, namLine, lineword, curword);
	
	if(!namParseType)
	{
	    namNoColon(&curword);
	    ajStrToLower(&curword);
	    if(ajStrPrefixCO("env", curword))
		namParseType = TYPE_ENV;
	    if(ajStrPrefixCO("setenv", curword))
		namParseType = TYPE_ENV;
	    else if(ajStrPrefixCO("dbname",curword))
		namParseType = TYPE_DB;
	    else if(ajStrPrefixCO("resource",curword))
		namParseType = TYPE_RESOURCE;
	    else if(ajStrPrefixCO("include",curword))
		namParseType = TYPE_IFILE;

	    if(!namParseType)		/* test: badtype.rc */
		namError("Invalid definition type '%S'", curword);
	    namUser("type set to %s curword '%S'\n",
		    namTypes[namParseType], curword);
	}
	else if(quoteopen && ajStrMatchC(curword, "]"))
	{				/* test; dbnoquote.rc */
	    namError("']' found, unclosed quotes in '%S'\n", value);
	    quoteopen    = 0;
	    namParseType = 0;
	}
	else if(quoteopen)
	{
	    /*
	    ** quote is open, so append word until close quote is found,
	    ** and set the appropriate save flag
	    */
	    namUser("<%c>..<%c> quote processing\n", quoteopen, quoteclose);
	    ajStrAppC(&value," ");
	    ajStrApp(&value,curword);
	    /* close quote here ?? */
	    if(ajStrChar(curword,-1) == quoteclose)
	    {
		namUser("close quotes\n");
		ajStrTrim(&value, -1);
		quoteopen = quoteclose = 0;
		if(namParseType == TYPE_ENV) /* set save flag, value found */
		    saveit = ajTrue;
		else if(namParseType == TYPE_DB)
		    dbsave = ajTrue;
		else if(namParseType == TYPE_RESOURCE)
		    rssave = ajTrue;
	    }
	}
	else if(namParseType == TYPE_ENV)
	{
	    if(name && value)
		saveit= ajTrue;
	    else if(name)
	    {
		/* if name already got then it must be the value */
		if(ajStrChar(curword,0) == '\'')
		    quoteopen = quoteclose = '\'';
		else if(ajStrChar(curword,0) == '\"')
		    quoteopen = quoteclose = '\"';

		ajStrAssS(&value, curword);
		if(quoteopen)
		{
		    /* trim the opening quote */
		    ajStrTrim(&value, 1);
		    if(!ajStrLen(value))
			ajErr("Bare quote %c found in namListParse",
			       quoteopen);
		}
		else
		    saveit = ajTrue;

		if(ajStrChar(curword, -1) == quoteclose)
		{
		    /* end of quote on same word */
		    quoteopen = quoteclose = 0;
		    saveit= ajTrue;
		    /* remove quote at the end */
		    ajStrTrim(&value, -1);
		}
		namUser("save value '%S'\n", value);
	    }
	    else
	    {
		ajStrAssS(&name, curword);
		namUser("save name '%S'\n", name);
	    }
	}
	
	
	else if(namParseType == TYPE_DB)
	{
	    if(ajStrMatchC(curword, "[")) /* [ therefore new database */
		dbattr = AJCALLOC0(ndbattr, sizeof(AjPStr)); /* new db obj */
	    else if(ajStrMatchC(curword, "]"))	/* ] therefore end of db */
		saveit = ajTrue;
	    else if(name)
	    {
		if(ajStrChar(curword, -1) == ':')
		{
		    /* if last character is : then its a keyword */
		    ajStrToLower(&curword); /* make it lower case */
		    namNoColon(&curword);
		    db_input = namDbAttr(curword);
		    if(db_input < 0)
			ajWarn("%S: bad attribute '%S' for database '%S'\n",
				namRootStr, curword, name);
		}
		else if(db_input >= 0)
		{
		    /* So if keyword type has been set */
		    if(ajStrChar(curword, 0) == '\'')
		    {
			/* is there a quote? If so expect the */
			/* same at the end. No ()[] etc here*/
			quoteopen = quoteclose = '\'';
		    }
		    else if(ajStrChar(curword, 0) == '\"')
			quoteopen = quoteclose = '\"';

		    ajStrAssS(&value, curword);
		    if(quoteopen)
		    {
			ajStrTrim(&value, 1); /* trim opening quote */
			if(!ajStrLen(value))
			    ajErr("Bare quote %c found in namListParse",
				   quoteopen);
		    }
		    else
			dbsave = ajTrue; /* we are done - simple word */

		    if(ajStrChar(curword, -1) == quoteclose)
		    {
			quoteopen = quoteclose = 0;
			ajStrTrim(&value,-1); /* trim closing quote */
			dbsave = ajTrue;
		    }
		    if(!quoteopen)     /* if we just reset it above */
			dbsave = ajTrue;
		}
	    }
	    else
	    {
		ajStrAssS(&name, curword);
		namUser("saving db name '%S'\n", name);
	    }
	}
	
	
	else if(namParseType == TYPE_RESOURCE)
	{
	    if(ajStrMatchC(curword, "[")) /* [ therefore new resource */
		rsattr = AJCALLOC0(nrsattr, sizeof(AjPStr)); /* new resource*/
	    else if(ajStrMatchC(curword, "]"))	/* ] end of resource */
		saveit = ajTrue;
	    else if(name)
	    {
		if(ajStrChar(curword, -1) == ':') /* if last character is : */
		{		     	          /* then it is a keyword */
		    ajStrToLower(&curword); /* make it lower case */
		    namNoColon(&curword);
		    rs_input = namRsAttr(curword);
		    if(rs_input < 0)	/* test: badresattr.rc */
			namError("Bad attribute '%S' for resource '%S'",
				  curword, name);
		}
		else if(rs_input >= 0)
		{		 /* So if keyword type has been set */
		    if(ajStrChar(curword, 0) == '\'')
		    {	      /* is there a quote? If so expect the */
			/* same at the end. No ()[] etc here */
			quoteopen = quoteclose = '\'';
		    }
		    else if(ajStrChar(curword, 0) == '\"')
			quoteopen = quoteclose = '\"';

		    ajStrAssS(&value, curword);
		    if(quoteopen)
		    {
			ajStrTrim(&value, 1); /* trim opening quote */
			if(!ajStrLen(value))
			    ajErr("Bare quote %c found in namListParse",
				   quoteopen);
		    }
		    else
			rssave = ajTrue;

		    if(ajStrChar(curword, -1) == quoteclose)
		    {
			quoteopen = quoteclose = 0;
			ajStrTrim(&value,-1); /* ignore quote if */
					             /* one at end */
			rssave = ajTrue;
		    }
		    if(!quoteopen)
			rssave = ajTrue;
		}
	    }
	    else
	    {
		ajStrAssS(&name, curword);
		namUser("saving resource name '%S'\n", name);
	    }
	}
	
	
	else if(namParseType == TYPE_IFILE)
	{
	    if(!Ifiles)
		Ifiles = ajStrTableNew(NAM_INCLUDE_ESTIMATE);
	    namParseType = 0;
	    if(ajTableGet(Ifiles,curword)) /* test: includeagain.rc */
		namError("%S already read .. skipping\n", curword);
	    else
	    {
		includefn = ajStrNew();
		ajStrAss(&includefn,curword);

		if(namFileOrig)
		    ajStrAppC(&namFileOrig,", ");
		ajStrApp(&namFileOrig,includefn);

		key = ajStrNewC(ajStrStr(includefn));
		val = ajStrNewC(ajStrStr(includefn));
		ajTablePut(Ifiles,key,val);

		if(!(iinf = ajFileNewIn(includefn))) /* test: badinclude.rc */
		{
		    namError("Failed to open include file '%S'\n", includefn);
		    ajStrAppC(&namFileOrig,"(Failed)");
		}
		else
		{
		    ajStrAppC(&namFileOrig,"(OK)");
		    namstatus = namProcessFile(iinf); /* replaces namFile */
		    namFile = file;	/* reset namFile */
		    namLine = linecount-1;
		    if(!namstatus)	/* test: badsummary.rc */
			namError("Error(s) found in included file %F", iinf);
		    ajFileClose(&iinf);
		}

		ajStrDel(&includefn);
	    }

	    namListParseOK = ajTrue;
	}
	
	
	if(dbsave)
	{
	    /* Save the keyword value */
	    ajStrAss(&dbattr[db_input], value);
	    db_input =-1;
	    ajStrDel(&value);
	    dbsave = ajFalse;
	}
	
	if(rssave)
	{
	    /* Save the keyword value */
	    ajStrAss(&rsattr[rs_input], value);
	    rs_input =-1;
	    ajStrDel(&value);
	    rssave = ajFalse;
	}
	
	namListParseOK = saveit;
	
	if(saveit)
	{
	    namUser("saving type %d name '%S' value '%S' line:%d\n",
		    namParseType, name, value, namLine);
	    AJNEW0(fnew);
	    tabname = ajCharNew(name);
	    fnew->name = name;
	    name = 0;
	    fnew->value = value;
	    value = 0;
	    fnew->type = namParseType;

	    if(namParseType == TYPE_DB)
		fnew->data = dbattr;
	    else if(namParseType == TYPE_RESOURCE)
		fnew->data = rsattr;
	    else
		fnew->data = 0;

	    /* Validate the master table entry */

	    if(namDoValid)
		namValid(fnew);

	    /*
	    ** Add new one to table 
	    ** be very careful that everything in the table 
	    ** is not about to be deallocated - so do not use "name" here
	    */

	    entry = ajTablePut(namMasterTable, tabname, fnew);
	    if(entry)
	    {
		/* it existed so over wrote previous table entry */
		namUser("%S: replaced previous definition of '%S'\n",
			 namRootStr,
			 entry->name);
		namEntryDelete(&entry); /* we can delete the previous entry */
		AJFREE(tabname);        /* ajTablePut reused the old key */
	    }
	    
	    saveit = ajFalse;
	    namParseType = 0;
	    db_input = -1;
	    dbattr = 0;
	}
	ajStrDel(&curword);
    }
    
    if(namParseType)
    {
	/* test: badset.rc baddb.rc  */
	namError("Unexpected end of file in %s definition",
		 namTypes[namParseType]);
	namParseType = 0;
    }
    
    if(ajListLength(listcount))	/* cleanup the wordcount list */
    {
	namUser("** remaining wordcount items: %d\n", ajListLength(listcount));
	while(ajListLength(listcount))
	{
	    ajListPop(listcount, (void**) &iword);
	    AJFREE(iword);
	}
    }

    if(value)
    {
	ajUser("++ namListParse value %x '%S'", value, value);
    }
    
    return;
}




/* @func ajNamGetenv **********************************************************
**
** Looks for name as an environment variable.
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [const AjPStr] character string find in hash table.
** @param [w] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetenv(const AjPStr name,
		    AjPStr* value)
{
    char *envval;

    envval = getenv(ajStrStr(name));
    if(envval)
    {
	ajStrAssC(value, envval);
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
** @param [r] name [const AjPStr] character string find in hash table.
** @param [w] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetValue(const AjPStr name, AjPStr* value)
{
    return ajNamGetValueC(ajStrStr(name), value);
}




/* @func ajNamGetValueC *******************************************************
**
** Looks for name as an (upper case) environment variable,
** and then as-is in the hash table and if found returns
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [const char*] character string find in hash table.
** @param [w] value [AjPStr*] Str for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetValueC(const char* name, AjPStr* value)
{
    NamPEntry fnew       = 0;
    static AjPStr namstr = NULL;
    AjBool hadPrefix     = ajFalse;
    AjBool ret           = ajFalse;
    
    if(ajStrPrefixCO(name, namPrefixStr)) /* may already have the prefix */
    {
	ajStrAssC(&namstr, name);
	hadPrefix = ajTrue;
    }
    else
    {
	ajStrAssS(&namstr, namPrefixStr);
	ajStrAppC(&namstr, name);
    }

    /* upper case for ENV, otherwise don't care */
    ajStrToUpper(&namstr);
    
    /* first test for an ENV variable */
    
    ret = ajNamGetenv(namstr, value);
    if(ret)
	return ajTrue;

    /* then test the table definitions - with the prefix */
    
    fnew = ajTableGet(namMasterTable, ajStrStr(namstr));
    if(fnew)
    {
	ajStrAssS(value, fnew->value);
	return ajTrue;
    }
    
    if(!hadPrefix)
    {

	/* then test the table definitions - as originally specified */

	fnew = ajTableGet(namMasterTable, name);
	if(fnew)
	{
	    ajStrAssS(value, fnew->value);
	    return ajTrue;
	}
    }
    
    return ajFalse;
}




/* @func ajNamDatabase ********************************************************
**
** Looks for name in the hash table and if found returns
** the attribute for this. If not found returns  NULL;
**
** @param [r] name [const AjPStr] character string find in hash table.
** @return [AjBool] true if database name is valid.
** @error  NULL if name not found in the table
** @@
**
******************************************************************************/

AjBool ajNamDatabase(const AjPStr name)
{
    NamPEntry fnew = 0;

    /* ajDebug("ajNamDatabase '%S'\n", name); */

    fnew = ajTableGet(namMasterTable, ajStrStr(name));
    if(fnew)
    {
	/* ajDebug("  '%S' found\n", name); */
	return ajTrue;
    }

    /* ajDebug("  '%S' not found\n", name); */
    return ajFalse;
}




/* @funcstatic namProcessFile *************************************************
**
** Read the definitions file and append each token to the list.
**
** @param [r] file [AjPFile] Input file object
** @return [AjBool] ajTrue if no error were found
** @@
******************************************************************************/

static AjBool namProcessFile(AjPFile file)
{
    AjPStr rdline = NULL;
    AjPStr word   = NULL;
    char *ptr;
    ajint i = 0;
    ajint len;
    char quote = '\0';
    AjPList listwords;
    AjPList listcount;
    AjPStr wordptr;
    ajint iline = 0;
    ajint *k    = NULL;
    
    ajint preverrorcount = namErrorCount;
    
    listwords = ajListstrNew();
    listcount = ajListNew();
    word      = ajStrNewL(128);
    
    namFile = file;
    namUser("namProcessFile '%F'\n", file);
    
    /* Read in the settings. */
    while(ajFileReadLine(file, &rdline))
    {
	
	AJNEW0(k);
	*k = ajListLength(listwords);
	ajListPushApp(listcount, k);
	
	/* namUser("%S\n",rdline); */
	len = ajStrLen(rdline);
	
	if(!ajStrUncommentStart(&rdline)) /* Ignore if the line is a comment */
	    continue;
	
	/* now create a linked list of the "words" */
	if(len)
	{
	    ptr = ajStrStr(rdline);
	    i = 0;
	    while(*ptr && i < len)
	    {
		if(*ptr == ' ' || *ptr == '\t')
		{
		    if(ajStrLen(word))
		    {
			wordptr = ajStrNewS(word);
			ajListstrPushApp(listwords, wordptr);
			ajStrAssC(&word, "");
		    }
		    i++;
		    ptr++;
		    continue;
		}
		else if(*ptr == '\'' || *ptr == '\"')
		{
		    ajStrAppK(&word,*ptr);
		    if(quote)
		    {
			if(quote == *ptr)
			    quote = '\0';
		    }
		    else
			quote = *ptr;
		}
		else if(!quote && ajStrLen(word) && *ptr == ']')
		{
		    wordptr = ajStrNewS(word);
		    ajListstrPushApp(listwords, wordptr);
		    ajStrAssC(&word, "");
		    wordptr = ajStrNewC("]");
		    ajListstrPushApp(listwords, wordptr);
		    ajStrAssC(&word, "");
		}
		else
		    ajStrAppK(&word,*ptr);
		i++;ptr++;
	    }

	    if(ajStrLen(word))
	    {
		wordptr = ajStrNewS(word);
		ajListstrPushApp(listwords, wordptr);
		ajStrAssC(&word, "");
	    }
	    
	}
    }
    ajStrDel(&rdline);
    
    AJNEW0(k);
    *k = ajListLength(listwords);
    ajListPushApp(listcount, k);
    
    namListParseOK = ajTrue;
    
    namUser("ready to parse\n");
    namListParse(listwords, listcount, file);
    
    if(!namListParseOK)
	namUser("Unexpected end of file in %S at line %d\n",
		namRootStr, iline);
    
    namUser("file read\n");
    ajListstrFree(&listwords);		/* Delete the linked list structure */
    namUser("wordlist free\n");
    ajListFree(&listcount);		/* Delete the linked list structure */
    namUser("countlist free\n");
    
    namDebugVariables();
    ajStrDel(&word);
    
    namUser("namProcessFile done '%F'\n", file);
    if(namErrorCount > preverrorcount)
	return ajFalse;

    return ajTrue;
}




/* @func ajNamInit ************************************************************
**
** Initialise the variable and database definitions. Find the definition
** files and read them.
**
** @param [r] prefix [const char*] Default prefix for all file
**                                 and variable names.
** @return [void]
** @@
******************************************************************************/

void ajNamInit(const char* prefix)
{
    char *prefixRoot;
    AjPFile prefixRootFile;
    AjPStr prefixRootStr = NULL;
    AjPStr prefixStr     = NULL;
    AjPStr prefixCap     = NULL;
    AjPStr debugStr      = NULL;
    AjPStr debugVal      = NULL;
    
    /* create new table to hold the values */
    
    namMasterTable = ajStrTableNewCaseC(0);
    
    /*
    ** for each type of file read it and save the values 
    ** Start at system level then go to user 
    ** static namPrefixStr is the prefix for all variable names
    */
    
    ajStrAssC(&namPrefixStr, prefix);
    
    ajStrAppC(&namPrefixStr, "_");
    
    /*
    ** local prefixRoot is the root directory 
    ** it is the value of (PREFIX)_ROOT (if set) or namFixedRoot
    */
    
    ajStrAssC(&debugStr, prefix);
    
    ajStrAppC(&debugStr, "_namdebug");
    ajStrToUpper(&debugStr);
    
    if(ajNamGetenv(debugStr, &debugVal))
	ajStrToBool(debugVal, &namDoDebug);
    
    ajStrAssC(&debugStr, prefix);
    
    ajStrAppC(&debugStr, "_namvalid");
    ajStrToUpper(&debugStr);
    
    if(ajNamGetenv(debugStr, &debugVal))
	ajStrToBool(debugVal, &namDoValid);
    
    ajStrDel(&debugStr);
    ajStrDel(&debugVal);
    
    ajStrAssC(&prefixStr, prefix);
    
    ajStrAppC(&prefixStr, "_ROOT");
    ajStrToUpper(&prefixStr);
    
    ajStrAppC(&prefixCap, prefix);
    ajStrToUpper(&prefixCap);
    
    if(ajNamGetenv(prefixStr, &prefixRootStr))
	prefixRoot = ajStrStr(prefixRootStr);
    else
	prefixRoot = namFixedRoot;
    
    /* namFixedRootBaseStr is the directory above the source root */
    
    ajStrAssC(&namFixedRootBaseStr, prefixRoot);
    ajFileDirUp(&namFixedRootBaseStr);
    
    /* look for default file in the install directory as
       <install-prefix>/share/PREFIX/emboss.default */
    
    ajFmtPrintS(&namRootStr, "%s/share/%S/%s.default",
		 namInstallRoot, prefixCap, prefix);
    prefixRootFile = ajFileNewIn(namRootStr);
    
    /* look for $(PREFIX)_ROOT/../emboss.default */
    
    if(!prefixRootFile)
    {
	/* try original directory */
	ajFmtPrintS(&namRootStr, "%s/%s.default", prefixRoot, prefix);
	prefixRootFile = ajFileNewIn(namRootStr);
    }
    
    if(namFileOrig)
	ajStrAppC(&namFileOrig, ", ");
    ajStrApp(&namFileOrig, namRootStr);
    
    if(prefixRootFile)
    {
	ajStrAppC(&namFileOrig, "(OK)");
	namProcessFile(prefixRootFile);
	ajFileClose(&prefixRootFile);
    }
    else
	ajStrAppC(&namFileOrig, "(failed)");
    
    
    
    /* look for .embossrc in an arbitrary directory */
    
    ajStrAssC(&debugStr, prefix);
    
    prefixRoot= getenv("EMBOSSRC");
    
    if(prefixRoot)
    {
	ajStrAssC(&namRootStr, prefixRoot);
	ajStrAppC(&namRootStr, "/.");
	ajStrAppC(&namRootStr, prefix);
	ajStrAppC(&namRootStr, "rc");
	if(namFileOrig)
	    ajStrAppC(&namFileOrig, ", ");
	ajStrApp(&namFileOrig, namRootStr);

	prefixRootFile = ajFileNewIn(namRootStr);
	if(prefixRootFile)
	{
	    ajStrAppC(&namFileOrig, "(OK)");
	    namProcessFile(prefixRootFile);
	    ajFileClose(&prefixRootFile);
	}
	else
	    ajStrAppC(&namFileOrig, "(failed)");
    }
    
    /* look for $HOME/.embossrc */
    
    prefixRoot= getenv("HOME");
    
    ajStrAppC(&debugStr, "_RCHOME");
    ajStrToUpper(&debugStr);
    
    if(ajNamGetenv(debugStr, &debugVal))
	ajStrToBool(debugVal, &namDoHomeRc);

    ajStrDel(&debugStr);
    ajStrDel(&debugVal);
    
    if(namDoHomeRc &&prefixRoot)
    {
	ajStrAssC(&namRootStr, prefixRoot);
	ajStrAppC(&namRootStr, "/.");
	ajStrAppC(&namRootStr, prefix);
	ajStrAppC(&namRootStr, "rc");
	if(namFileOrig)
	    ajStrAppC(&namFileOrig, ", ");
	ajStrApp(&namFileOrig, namRootStr);

	prefixRootFile = ajFileNewIn(namRootStr);
	if(prefixRootFile)
	{
	    ajStrAppC(&namFileOrig, "(OK)");
	    namProcessFile(prefixRootFile);
	    ajFileClose(&prefixRootFile);
	}
	else
	    ajStrAppC(&namFileOrig, "(failed)");
    }
    
    ajStrDel(&prefixRootStr);
    ajStrDel(&prefixStr);
    ajStrDel(&prefixCap);
    
    if(namErrorCount)		/* test: badsummary.rc */
	ajDie("Error(s) in configuration files");

    return;
}




/* @funcstatic namNoColon *****************************************************
**
** Remove any trailing colon ':' in the input string.
**
** @param [u] thys [AjPStr*] String.
** @return [void]
** @@
******************************************************************************/

static void namNoColon(AjPStr* thys)
{
    if(ajStrChar(*thys, -1) == ':')
	ajStrTrim(thys, -1);

    return;
}




/* @funcstatic namDbAttr ******************************************************
**
** Return the index for a database attribute name.
**
** @param [r] thys [const AjPStr] Attribute name.
** @return [ajint] Index in namDbAttrs, or -1 on failure.
** @@
******************************************************************************/

static ajint namDbAttr(const AjPStr thys)
{
    return namDbAttrC(ajStrStr(thys));
}




/* @funcstatic namDbAttrC *****************************************************
**
** Return the index for a database attribute name.
**
** @param [r] str [const char*] Attribute name.
** @return [ajint] Index in namDbAttrs, or -1 on failure.
** @@
******************************************************************************/

static ajint namDbAttrC(const char* str)
{
    ajint i = 0;
    ajint j = 0;
    ajint ifound = 0;

    for(i=0; namDbAttrs[i].Name; i++)
    {
	if(!strcmp(str, namDbAttrs[i].Name))
	    return i;

	if(ajStrPrefixCC(namDbAttrs[i].Name, str))
	{
	    ifound++;
	    j = i;
	}
    }

    if(ifound == 1)
	return j;

    return -1;
}




/* @funcstatic namRsAttr ******************************************************
**
** Return the index for a resource attribute name.
**
** @param [r] thys [const AjPStr] Attribute name.
** @return [ajint] Index in namRsAttrs, or -1 on failure.
** @@
******************************************************************************/

static ajint namRsAttr(const AjPStr thys)
{
    return namRsAttrC(ajStrStr(thys));
}




/* @funcstatic namRsAttrC *****************************************************
**
** Return the index for a resource attribute name.
**
** @param [r] str [const char*] Attribute name.
** @return [ajint] Index in namRsAttrs, or -1 on failure.
** @@
******************************************************************************/

static ajint namRsAttrC(const char* str)
{
    ajint i = 0;
    ajint j = 0;
    ajint ifound = 0;

    for(i=0; namRsAttrs[i].Name; i++)
    {
	if(!strcmp(str, namRsAttrs[i].Name))
	    return i;

	if(ajStrPrefixCC(namRsAttrs[i].Name, str))
	{
	    ifound++;
	    j = i;
	}
    }

    if(ifound == 1)
	return j;

    return -1;
}




/* @func ajNamExit ************************************************************
**
** Delete the initialisation values in the table.
** @return [void]
** @@
******************************************************************************/

void ajNamExit(void)
{

    namListMasterDelete(); /* Delete elements from database structure */
    ajTableFree(&namMasterTable); /* free table and database structures */
    ajStrDel(&namFixedRootBaseStr);	/* allocated in ajNamInit */
    ajStrDel(&namPrefixStr);		/* allocated in ajNamInit */
    ajStrDel(&namFileOrig);		/* allocated in ajNamInit */
    ajStrDel(&namRootStr);		/* allocated in ajNamInit */

    ajRegFree(&namNameExp);
    ajRegFree(&namVarExp);

    ajDebug("ajNamExit done\n");

    return;
}




/* @func ajNamDbTest **********************************************************
**
** Looks for a database name in the known definitions.
**
** @param [r] dbname [const AjPStr] Database name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamDbTest(const AjPStr dbname)
{
    NamPEntry data;

    data = ajTableGet(namMasterTable, ajStrStr(dbname));

    if(!data)
	return ajFalse;

    if(data->type != TYPE_DB)
	return ajFalse;

    return ajTrue;
}




/* @func ajNamDbGetUrl ********************************************************
**
** Gets the URL definition for a database definition.
**
** @param [r] dbname [const AjPStr] Database name.
** @param [w] url [AjPStr*] URL returned.
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbGetUrl(const AjPStr dbname, AjPStr* url)
{

    NamPEntry data;
    AjPStr* dbattr;
    static ajint calls = 0;
    static ajint iurl  = 0;

    if(!calls)
    {
	iurl = namDbAttrC("url");
	calls = 1;
    }
    data = ajTableGet(namMasterTable, ajStrStr(dbname));

    if(!data)
	ajFatal("%S is not a known database\n", dbname);

    if(data->type != TYPE_DB)
	ajFatal("%S is not a database\n", dbname);

    dbattr = data->data;

    if(ajStrLen(dbattr[iurl]))
    {
	ajStrAssS(url, dbattr[iurl]);
	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamDbGetDbalias ****************************************************
**
** Gets an alias name for a database.
**
** @param [r] dbname [const AjPStr] Database name.
** @param [w] dbalias [AjPStr*] Alias returned.
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbGetDbalias(const AjPStr dbname, AjPStr* dbalias)
{

    NamPEntry data;
    AjPStr* dbattr;
    static ajint calls = 0;
    static ajint idba  = 0;

    if(!calls)
    {
	idba = namDbAttrC("dbalias");
	calls = 1;
    }
    data = ajTableGet(namMasterTable, ajStrStr(dbname));

    if(!data)
	ajFatal("%S is not a known database\n", dbname);

    if(data->type != TYPE_DB)
	ajFatal("%S is not a database\n", dbname);

    dbattr = data->data;

    if(ajStrLen(dbattr[idba]))
    {
	ajStrAssS(dbalias, dbattr[idba]);
	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamDbData **********************************************************
**
** Given a query with database name and search fields,
** fill in the common fields. The query fields are set later.
**
** This part of the database definition is required (specifically
** the "fields" definition) for setting the query details.
**
** See also ajNamDbQuery, which calls this function if the common
** query data is not yet set.
**
** @param [u] qry [AjPSeqQuery] Query structure with at least
**                                    dbname filled in
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbData(AjPSeqQuery qry)
{

    NamPEntry data;

    const AjPStr* dbattr;

    data = ajTableGet(namMasterTable, ajStrStr(qry->DbName));

    if(!data || (data->type != TYPE_DB))
	ajFatal("database %S unknown\n", qry->DbName);

    dbattr = data->data;

    /* general defaults */

    namDbSetAttr(dbattr, "type", &qry->DbType);
    namDbSetAttr(dbattr, "method", &qry->Method);
    namDbSetAttr(dbattr, "format", &qry->Formatstr);
    namDbSetAttr(dbattr, "app", &qry->Application);
    namDbSetAttr(dbattr, "directory", &qry->IndexDir);
    namDbSetAttr(dbattr, "indexdirectory", &qry->IndexDir);
    namDbSetAttr(dbattr, "indexdirectory", &qry->Directory);
    namDbSetAttr(dbattr, "directory", &qry->Directory);
    namDbSetAttr(dbattr, "exclude", &qry->Exclude);
    namDbSetAttr(dbattr, "filename", &qry->Filename);
    namDbSetAttr(dbattr, "fields", &qry->DbFields);
    namDbSetAttr(dbattr, "proxy", &qry->DbProxy);
    namDbSetAttr(dbattr, "httpversion", &qry->DbHttpVer);
    /*
       ajDebug("ajNamDbQuery DbName '%S'\n", qry->DbName);
       ajDebug("    Id '%S' Acc '%S' Des '%S'\n",
                qry->Id, qry->Acc, qry->Des);
       ajDebug("    Method      '%S'\n", qry->Method);
       ajDebug("    Formatstr   '%S'\n", qry->Formatstr);
       ajDebug("    Application '%S'\n", qry->Application);
       ajDebug("    IndexDir    '%S'\n", qry->IndexDir);
       ajDebug("    Directory   '%S'\n", qry->Directory);
       ajDebug("    Filename    '%S'\n", qry->Filename);
       */

    return ajTrue;
}




/* @func ajNamDbQuery *********************************************************
**
** Given a query with database name and search fields,
** fill in the access method and some common fields according
** to the query level.
**
** @param [u] qry [AjPSeqQuery] Query structure with at least
**                                    dbname filled in
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbQuery(AjPSeqQuery qry)
{

    NamPEntry data;

    const AjPStr* dbattr;

    data = ajTableGet(namMasterTable, ajStrStr(qry->DbName));

    if(!data || (data->type != TYPE_DB))
	ajFatal("database %S unknown\n", qry->DbName);

    dbattr = data->data;

    if(!ajStrLen(qry->DbType))
	ajNamDbData(qry);

    if(!ajSeqQueryIs(qry))   /* must have a method for all entries */
    {

	namDbSetAttr(dbattr, "methodall", &qry->Method);
	namDbSetAttr(dbattr, "formatall", &qry->Formatstr);
	namDbSetAttr(dbattr, "appall", &qry->Application);
	qry->Type = QRY_ALL;
    }
    else		      /* must be able to query the database */
    {
	namDbSetAttr(dbattr, "methodquery", &qry->Method);
	namDbSetAttr(dbattr, "formatquery", &qry->Formatstr);
	namDbSetAttr(dbattr, "appquery", &qry->Application);

	if(!ajSeqQueryWild(qry)) /* ID - single entry may be available */
	{
	    namDbSetAttr(dbattr, "methodentry", &qry->Method);
	    namDbSetAttr(dbattr, "formatentry", &qry->Formatstr);
	    namDbSetAttr(dbattr, "appentry", &qry->Application);
	    qry->Type = QRY_ENTRY;
	}
	else
	    qry->Type = QRY_QUERY;
    }


    if(!ajStrLen(qry->Formatstr))
    {
	ajErr("No format defined for database '%S'", qry->DbName);
	return ajFalse;
    }

    if(!ajStrLen(qry->Method))
    {
	ajErr("No access method for database '%S'", qry->DbName);
	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic namDbSetAttr ***************************************************
**
** Sets a named attribute value from an attribute list.
**
** @param [r] dbattr [const AjPStr*] Attribute definitions.
** @param [r] attrib [const char*] Attribute name.
** @param [w] qrystr [AjPStr*] Returned attribute value.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool namDbSetAttr(const AjPStr* dbattr, const char* attrib,
			    AjPStr* qrystr)
{
    ajint i;

    i = namDbAttrC(attrib);

    if(!i)
	ajFatal("unknown attribute '%s' requested",  attrib);

    if(!ajStrLen(dbattr[i]))
	return ajFalse;

    ajStrAssS(qrystr, dbattr[i]);
    /* ajDebug("namDbSetAttr('%S')\n", *qrystr); */

    namVarResolve(qrystr);

    return ajTrue;
}




/* @funcstatic namVarResolve **************************************************
**
** Resolves any variable or function references in a string.
** Yet to be implemented, but called in the right places.
**
** @param [u] var [AjPStr*] String value
** @return [AjBool] Always ajTrue so far
** @@
******************************************************************************/

static AjBool namVarResolve(AjPStr* var)
{

    AjPStr varname = NULL;
    AjPStr newvar  = NULL;
    AjPStr restvar = NULL;

    if(!namVarExp)
	namVarExp = ajRegCompC("^\\$([a-zA-Z0-9_.]+)");

    while(ajRegExec(namVarExp, *var))
    {
	ajRegSubI(namVarExp, 1, &varname); /* variable name */

	ajNamGetValue(varname, &newvar);

	ajDebug("namVarResolve '%S' = '%S'\n", varname, newvar);

	if(ajRegPost(namVarExp, &restvar)) /* any more? */
	    ajStrApp(&newvar, restvar);
	ajStrAssS(var, newvar);
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
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void namUser(const char* fmt, ...)
{
    va_list args;

    if(!namDoDebug)
	return;
    va_start(args, fmt);
    ajFmtVError(fmt, args);
    va_end(args);

    return;
}




/* @funcstatic namError *******************************************************
**
** Formatted write as an error message.
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void namError(const char* fmt, ...)
{
    va_list args;
    AjPStr errstr = NULL;
  
    namErrorCount++;

    va_start(args, fmt);
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args);

    ajErr("File %F line %d: %S", namFile, namLine, errstr);
    ajStrDel(&errstr);

    return;
}




/* @func ajNamRootInstall *****************************************************
**
** Returns the install directory root for all file searches
** (package level)
**
** @param [w] root [AjPStr*] Root.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRootInstall(AjPStr* root)
{
    ajStrAssC(root, namInstallRoot);

    return ajTrue;
}




/* @func ajNamRootPack ********************************************************
**
** Returns the package name for the library
**
** @param [w] pack [AjPStr*] Package name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRootPack(AjPStr* pack)
{
    ajStrAssC(pack, namPackage);

    return ajTrue;
}




/* @func ajNamRootVersion *****************************************************
**
** Returns the version number for the library
**
** @param [w] version [AjPStr*] Version number.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRootVersion(AjPStr* version)
{
    ajStrAssC(version, namVersion);

    return ajTrue;
}




/* @func ajNamRoot ************************************************************
**
** Returns the directory for all file searches
** (package level)
**
** @param [w] root [AjPStr*] Root.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRoot(AjPStr* root)
{
    ajStrAssC(root, namFixedRoot);

    return ajTrue;
}




/* @func ajNamRootBase ********************************************************
**
** Returns the base directory for all for all file searches
** (above package level).
**
** @param [w] rootbase [AjPStr*] Root.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamRootBase(AjPStr* rootbase)
{
    ajStrAssS(rootbase, namFixedRootBaseStr);

    return ajTrue;
}




/* @func ajNamResolve *********************************************************
**
** Resolves a variable name if the input string starts with a dollar sign.
**
** @param [w] name [AjPStr*] String
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamResolve(AjPStr* name)
{

    AjPStr varname  = NULL;
    AjPStr varvalue = NULL;
    AjPStr restname = NULL;
    AjBool ret;

    if(!namNameExp)
	namNameExp = ajRegCompC("^\\$([A-Za-z0-9_]+)");

    namUser("ajNamResolve of '%S'\n", *name);
    ret = ajRegExec(namNameExp, *name);
    if(ret)
    {
	ajRegSubI(namNameExp, 1, &varname);
	namUser("variable '%S' found\n", varname);
	ajRegPost(namNameExp, &restname);
	ret = ajNamGetValue(varname, &varvalue);
	if(ret)
	{
	    ajStrAssS(name, varvalue);
	    ajStrApp(name, restname);
	    namUser("converted to '%S'\n", *name);
	}
	else
	{
	    namUser("Variable unknown '$%S'\n", varname);
	    ajWarn("Variable unknown in '%S'", *name);
	}
	ajStrDel(&varname);
	ajStrDel(&varvalue);
	ajStrDel(&restname);
    }

    return ret;
}




/* @funcstatic namValid *******************************************************
**
** Validation of a master table entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool namValid(const NamPEntry entry)
{
    if(entry->type == TYPE_ENV)
	return namValidVariable(entry);
    else if(entry->type == TYPE_DB)
	return namValidDatabase(entry);
    else if(entry->type == TYPE_RESOURCE)
	return namValidResource(entry);

    /* fatal: cannot test - should not happen */
    namError("Unknown definition type number %d",
	      entry->type);

    return ajFalse;
}




/* @funcstatic namValidDatabase ***********************************************
**
** Validation of a master table database entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool namValidDatabase(const NamPEntry entry)
{
    ajint iattr = 0;
    ajint j;
    ajint k;
    AjBool ok;
    AjBool oktype;
    AjPStr* attrs;
    AjBool hasformat = ajFalse;
    AjBool hasmethod = ajFalse;
    AjBool hastype   = ajFalse;
    
    attrs = entry->data;
    if(!attrs)
    {			 /* fatal - should be set for all databases */
	namError("Database '%S' has no list of valid attributes",
		  entry->name);
	return ajFalse;
    }
    
    for(j=0; namDbAttrs[j].Name; j++)
    {
	if(attrs[j])
	{
	    iattr++;
	    if(ajStrPrefixCC(namDbAttrs[j].Name, "format"))
	    {
		hasformat=ajTrue;
		if(!ajSeqFormatTest(attrs[j]))	/* test: dbunknowns.rc */
		    namError("Database '%S' %s: '%S' unknown\n",
			     entry->name, namDbAttrs[j].Name, attrs[j]);
	    }

	    if(ajStrPrefixCC(namDbAttrs[j].Name, "method"))
	    {
		hasmethod=ajTrue;
		if(!ajSeqMethodTest(attrs[j]))	/* test: dbunknowns.rc */
		    namError("Database '%S' %s: '%S' unknown\n",
			     entry->name, namDbAttrs[j].Name, attrs[j]);
	    }

	    if(ajStrPrefixCC(namDbAttrs[j].Name, "type"))
	    {
		hastype=ajTrue;
		oktype = ajFalse;
		for(k=0; namDbTypes[k].Name; k++)
		{
		    if(ajStrMatchCaseC(attrs[j], namDbTypes[k].Name)) 
			oktype = ajTrue;
		}
		if(!oktype)		/* test: dbunknowns.rc */
		    namError("Database '%S' %s: '%S' unknown\n",
			      entry->name, namDbAttrs[j].Name, attrs[j]);
	    }
	}
    }

    ok = ajTrue;
    if(!iattr)
    {					/* test: dbempty.rc */
	namError("Database '%S' has no attributes", entry->name);
	ok = ajFalse;
    }

    if(!hasformat)		/* test: dbempty.rc */
    {
	namError("Database '%S' has no format definition", entry->name);
	ok = ajFalse;
    }

    if(!hastype)			/* test: dbempty.rc */
    {
	namError("Database '%S' has no type definition", entry->name);
	ok = ajFalse;
    }

    if(!hasmethod)		/* test: dbempty.rc */
    {
	namError("Database '%S' has no access method definition",
		  entry->name);
	ok = ajFalse;
    }

    return ok;
}




/* @funcstatic namValidResource ***********************************************
**
** Validation of a master table resource entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool namValidResource(const NamPEntry entry)
{
    ajint iattr = 0;
    ajint j;
    AjPStr* attrs;
    AjBool hastype = ajFalse;
    AjBool ok;

    attrs = entry->data;
    if(!attrs)
    {			 /* fatal - should be set for all databases */
	namError("Resource '%S' has no list of valid attributes",
		  entry->name);
	return ajFalse;
    }

    for(j=0; namRsAttrs[j].Name; j++)
	if(attrs[j])
	{
	    iattr++;
	    if(ajStrPrefixCC(namDbAttrs[j].Name, "type"))
		hastype=ajTrue;
	}

    ok = ajTrue;
    if(!iattr)
    {					/* test: dbempty.rc */
	namError("Resource '%S' has no attributes", entry->name);
	ok =  ajFalse;
    }

    return ok;
}




/* @funcstatic namValidVariable ***********************************************
**
** Validation of a master table variable entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool namValidVariable(const NamPEntry entry)
{
    return ajTrue;
}




/* @func ajNamSetControl ******************************************************
**
** Sets special internal variables to reflect their presence.
**
** Currently these are "namdebug, namvalid"
**
** @param [r] optionName [const char*] option name
** @return [AjBool] ajTrue if option was recognised
** @@
******************************************************************************/

AjBool ajNamSetControl(const char* optionName)
{

    if(!ajStrCmpCaseCC(optionName, "namdebug"))
    {
	namDoDebug = ajTrue;
	return ajTrue;
    }

    if(!ajStrCmpCaseCC(optionName, "namvalid"))
    {
	namDoValid = ajTrue;
	return ajTrue;
    }

    ajDie("Unknown ajNamSetControl control option '%s'", optionName);

    return ajFalse;
}
