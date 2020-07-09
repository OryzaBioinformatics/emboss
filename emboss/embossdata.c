/* @source embossdata application
**
** Finds or fetches the data files read in by the EMBOSS programs
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @modified Alan Bleasby (ableasby@hgmp.mrc.ac.uk) for recursion
** @modified Alan Bleasby (ableasby@hgmp.mrc.ac.uk) to remove HOME dependency
** @@
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


static void check_dir(AjPStr d, AjPFile outf);
static void check_file(AjPStr d, AjPStr file, AjPFile outf);
static AjPStr data_dir(void);



int main (int argc, char * argv[])
{
    AjPList rlist=NULL;
    AjPList flocs=NULL;
    AjPFile outf;
    AjPStr  t=NULL;
    
    AjBool  recurs=ajTrue;
    int i;
    

    AjPStr filename = NULL;
    AjBool isname;
    
    AjBool fetch;
    AjBool showall;

    AjPStr directory = NULL;
    AjPStr hdir = NULL;
    AjPStr ddir = NULL;
    AjPStr path = NULL;
    AjPStr cmd = NULL;
    AjPStr *rstrs=NULL;
    
    int result;
    char *p=NULL;
    
    (void) embInit ("embossdata", argc, argv);

    filename = ajAcdGetString ("filename");
    fetch = ajAcdGetBool ("fetch");
    showall = ajAcdGetBool ("showall");
    rstrs = ajAcdGetSelect("reject");
    outf = ajAcdGetOutfile("outf");


    if(ajStrLen(filename))
	isname=ajTrue;
    else
	isname=ajFalse;


    /* Get directory reject list */
    rlist = ajListNew();
    if(!ajStrMatchCaseC(rstrs[0],"None"))
    {
	i=0;
	while(rstrs[i])
	{
	    ajListPush(rlist,(void *)rstrs[i]);
	    ++i;
	}
    }


    /* get the full pathname of the  emboss/data installation directory */
    ddir = data_dir();

    flocs = ajListNew();

    /*
     * do we want to fetch the data from the 'official' EMBOSS
     * data directory
     */
    if (fetch)
    {
	ajStrAss(&path,ddir);
	ajFileScan(path,filename,&flocs,ajFalse,ajFalse,NULL,rlist,
		   recurs, outf);
	if(!ajListPop(flocs,(void **)&t))
	    ajDie("The file '%S' does not exist.", filename);
	/* fetch it */
	(void) ajStrAppC(&cmd, "cp ");
	(void) ajStrApp(&cmd, t);
	(void) ajStrAppC(&cmd, " ");
	(void) ajStrApp(&cmd, filename);
	result = system(ajStrStr(cmd));
	if (result)
	    ajDie("File not copied.");
	ajFmtPrintF(outf, "File '%S' has been copied successfully.\n", t);
	ajStrDel(&t);
	ajStrDel(&cmd);
    } 


    /*
     *  report whether data directories exist (no filename given)
     *  or whether a specific file is in those directories
     */
    if(!fetch && !showall)
    {
	ajFmtPrintF(outf,
	"# The following directories can contain EMBOSS data files.\n");
	ajFmtPrintF(outf,
	"# They are searched in the following order "
		    "until the file is found.\n");
	ajFmtPrintF(outf,
	"# If the directory does not exist, then this is noted below.\n");
	ajFmtPrintF(outf,
	"# '.' is the UNIX name for your current working directory.\n");
	ajFmtPrintF(outf,"\n");

	(void) ajStrAssC(&directory, ".");
	if(isname)
	    check_file(directory,filename,outf);
	else
	    check_dir(directory,outf);

	/* .embossdata */
	(void) ajStrAssC(&directory, ".embossdata");
	if(isname)
	    check_file(directory,filename,outf);
	else
	    check_dir(directory,outf);

	/* HOME */    
	if((p=getenv("HOME")))
	{
	    (void) ajStrAssC(&hdir, p);
	    (void) ajStrAss(&directory, hdir);
	    if(isname)
		check_file(directory,filename,outf);
	    else
		check_dir(directory,outf);

	    /* ~/.embossdata */
	    (void) ajStrAss(&directory, hdir);
	    (void) ajStrAppC(&directory, "/.embossdata");
	    if(isname)
		check_file(directory,filename,outf);
	    else
		check_dir(directory,outf);
	}
	

	/* DATA */
	if(isname)
	{
	    ajStrAss(&path,ddir);
	    ajFileScan(path,filename,&flocs,ajFalse,ajFalse,NULL,rlist,
		       recurs,outf);
	    if(!ajListPop(flocs,(void **)&t))
		check_file(ddir,filename,outf);
	    else
	    {
		ajFmtPrintF(outf,"File %-60.60S Exists\n",t);
		ajStrDel(&t);
	    }
	}
	else
	    check_dir(ddir,outf);
    }
    

    /*
     *  Just show all the files in the EMBOSS Installation data directory
     */
    if (showall)
    {
	ajStrAss(&path,ddir);
	ajFileScan(path,NULL,NULL,ajTrue,ajFalse,NULL,rlist,recurs,outf);
    }

    ajListDel(&flocs);
    while(ajListPop(rlist,(void **)&t))
	ajStrDel(&t);
    ajListDel(&rlist);
    ajStrDel(&path);
    ajStrDel(&ddir);
    ajStrDel(&filename);
    ajFileClose(&outf);

    ajExit ();
    return 0;
}








static void check_dir(AjPStr d, AjPFile outf)
{
    if(ajSysIsDirectory(ajStrStr(d)))
	ajFmtPrintF(outf,"%-60.60S Exists\n",d);
    else
	ajFmtPrintF(outf,"%-60.60S Does not exist\n",d);

    return;
}





static void check_file(AjPStr d, AjPStr file, AjPFile outf)
{
    AjPStr s;

    s = ajStrNew();
    ajStrAss(&s,d);
    ajStrAppC(&s,"/");
    ajStrApp(&s,file);
    if(ajSysIsRegular(ajStrStr(s)))
	ajFmtPrintF(outf,"File %-60.60S Exists\n",s);
    else
	ajFmtPrintF(outf,"File %-60.60S Does not exist\n",s);

    ajStrDel(&s);

    return;
}



static AjPStr data_dir(void)
{
    static AjPStr where=NULL;
    AjPStr tmp=NULL;
    
    where = ajStrNew();
    tmp = ajStrNew();


    if(!ajNamGetValueC("DATA",&tmp))
    {
	ajNamRootInstall(&where);
	ajFileDirFix(&where);
	ajFmtPrintS(&tmp,"%Sshare/EMBOSS/data/",where);

	if(!ajFileDir(&tmp))
	{
	    if(ajNamRoot(&tmp))
		(void) ajStrAppC(&tmp,"/data");
	    else
		ajDie("The EMBOSS 'DATA' directory isn't defined.");
	}
    }

    ajStrAssC(&where,ajStrStr(tmp));

    ajStrDel(&tmp);

    return where;
}
