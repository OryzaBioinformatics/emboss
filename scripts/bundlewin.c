/* @source bundlewin application
**
** Microsoft Windows Developer bundling
**
** @author Alan Bleasby
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
**
**
** This is a UNIX or Borland (bcc32) application, only rigorously tested under
** Linux (purely as it's more convenient for the core developers).
** Its purpose
** is to take a CVS checkout of EMBOSS from the developers' site
** and construct a directory structure suitable for compiling under
** Microsoft Visual C++ Express.
**
** Compile using: cc -o bundlewin -O2 bundlewin.c   (UNIX) or
**                bcc32 bundlewin.c (WINDOWS)
**
** Usage: If you have checked-out EMBOSS from CVS in /fu/bar then:
**
**        ./bundlewin /fu/bar/emboss      or
**        bundlewin C:\fu\bar\emboss
**
** Under UNIX this will create a file called   '/fu/bar/emboss/emboss.zip'
** The zip file can then be transferred to Windows. When extracted
** it will create a folder called 'win32'.
**
** Under windows it will create a Windows folder c:\fu\bar\emboss\win32
******************************************************************************/

#include <stdio.h>
#if !defined(WIN32) || defined(__BORLANDC__)
#include <dirent.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#if defined(__BORLANDC__)
#include <dir.h>
#else
#include "win32.h"
#include "dirent_w32.h"
#include <direct.h>
#define chmod _chmod
#define mkdir _mkdir
#endif
#endif

#ifndef MAXNAMLEN
#define MAXNAMLEN 512
#endif

#include <string.h>
#include <stdlib.h>

#define COREDEF "ajaxdll.def"
#define PCREDEF "epcredll.def"
#define EXPATDEF "eexpatdll.def"
#define ZLIBDEF "ezlibdll.def"
#define GRAPHICSDEF "ajaxgdll.def"
#define ENSEMBLDEF "ensembldll.def"
#define AJAXDBDEF "ajaxdbdll.def"
#define ACDDEF "acddll.def"
#define NUCLEUSDEF "nucleusdll.def"

#ifndef WIN32
#define TMPFILE  "/tmp/hfuncts_1"
#define TMPFILE2 "/tmp/hfuncts_2"
#define CP "cp -f"
#define CPR "cp -fR"
#define CPDFPR "cp -dfpR"
#define CPFP "cp -fp"
#else
#define TMPFILE  "hfuncts_1.txt"
#define TMPFILE2 "hfuncts_2.txt"
#define CP "xcopy /Y /Q"
#define CPR "xcopy /Y /Q /E"
#define CPDFPR "xcopy /Y /Q /E"
#define CPFP "xcopy /Y /Q"
#endif

#define BUILDDIR "winbuild"

#define PROJECT "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\")"




typedef struct node
{
    char prog[MAXNAMLEN];
    struct node *next;
} listnode;


/*
** Exclude specific programs
*/

static char *exclude_names[]=
{
    "dbxreport.c",
    "dbxstat.c",
    NULL
};

    

/*
** Specify any programs with special memory reqirements.
*/

typedef struct SStackHeap
{
    char *progname;
    char *stacksize;
    char *heapsize;
} OStackHeap;

static OStackHeap memory[] = 
{
    {"cirdna", "5000000", "5000000"},
    {"lindna", "7000000", "7000000"},
    {NULL, NULL, NULL}
};


    
    
static int dir_exists(char *path);
static int file_exists(char *path);

static void copy_ajax(char *path);
static void copy_nucleus(char *path);
static void copy_plplot(char *basedir);
static void copy_DLLs(char *basedir);
static int  copy_apps(char *basedir, listnode *head);
static void copy_data(char *basedir);
static void copy_doc(char *basedir);
static void copy_test(char *basedir);
static void copy_jemboss(char *basedir);
static void copy_mysql(char *basedir);
static void copy_redist(char *basedir);
    
static void create_directories(char *basedir);
static void make_ajax_header_exports(char *basedir,char *subdir);
static int header_exports(char *dir, FILE *fp);
static void extract_funcnames(char *filename, FILE *fout);
static void write_coreexports(char *fn, char *basedir);
static void write_genajaxexport(char *fn,char *basedir,char *exportdef,
                                char *subdir,char *dllname);

static void write_nucleusexports(char *fn,char *basedir);
static void read_make_check(char *basedir, listnode **head);
static void add_node(listnode **head, char *progname);
static void make_uids(char ***uids, int napps);
static void read_prognames(char *basedir, int napps, char ***names);
static void write_solution(char *basedir, char **prognames, char **uids,
			   int napps);
static void write_projects(char *basedir, char **prognames, char **uids,
			   int napps);
static void sub_text(char *line, char *tline, char *given, char *rep);
static void zip_up(char *basedir);

static void extract_expat_funcnames(char *filename, FILE *fout);
static void make_expat_header_exports(char *basedir);
static void make_zlib_header_exports(char *basedir);
static void extract_zlib_funcnames(char *filename, FILE *fout);
static void fix_dir(char *str);




int main(int argc, char **argv)
{
    char basedir[MAXNAMLEN];
    char nucleusdir[MAXNAMLEN];
    listnode *head = NULL;
    listnode *ptr;
    listnode *tptr;
    char **uids = NULL;
    char **prognames = NULL;

    FILE *fp;
    
    int len;
    int napps;
    
    if(argc>1)
	strcpy(basedir,argv[1]);
    else
    {
	fprintf(stdout,"Path to emboss CVS top level: ");
	fgets(basedir,MAXNAMLEN,stdin);
	len = strlen(basedir);
	if(basedir[len-1] == '\n')
	    basedir[len-1] = '\0';
    }




    /* Create Windows build directories as a 'win32' subdir of basedir */
    create_directories(basedir);

    /* Copy project files for ajax, nucleus and DLLs */    

    copy_ajax(basedir);
    copy_nucleus(basedir);
    copy_plplot(basedir);
    copy_DLLs(basedir);

    /* Copy data files */

    copy_data(basedir);
    copy_doc(basedir);
    copy_test(basedir);
    copy_jemboss(basedir);
    copy_mysql(basedir);
    copy_redist(basedir);



    
    /* Construct exports file (ajaxdll.def) */


    
    make_ajax_header_exports(basedir,"core");
    write_coreexports(TMPFILE,basedir);

    make_expat_header_exports(basedir);
    write_genajaxexport(TMPFILE,basedir,EXPATDEF,"expat","eexpat");

    make_zlib_header_exports(basedir);
    write_genajaxexport(TMPFILE,basedir,ZLIBDEF,"zlib","ezlib");


    make_ajax_header_exports(basedir,"pcre");    
    write_genajaxexport(TMPFILE,basedir,PCREDEF,"pcre","epcre");


    make_ajax_header_exports(basedir,"graphics");
    write_genajaxexport(TMPFILE,basedir,GRAPHICSDEF,"graphics","ajaxg");


    make_ajax_header_exports(basedir,"ensembl");
    write_genajaxexport(TMPFILE,basedir,ENSEMBLDEF,"ensembl","ensembl");


    make_ajax_header_exports(basedir,"ajaxdb");    
    write_genajaxexport(TMPFILE,basedir,AJAXDBDEF,"ajaxdb","ajaxdb");


    make_ajax_header_exports(basedir,"acd");        
    write_genajaxexport(TMPFILE,basedir,ACDDEF,"acd","acd");
    

    /* Construct exports file (nucleusdll.def) */

    sprintf(nucleusdir,"%s/emboss/nucleus",basedir);
    
    fp=fopen(TMPFILE,"w");

    if(!fp)
    {
	fprintf(stderr,"Cannot open temporary file %s\n",TMPFILE);
	exit(-1);
    }
    

    if(header_exports(nucleusdir,fp) < 0)
    {
	fprintf(stderr,"Cannot open directory %s/nucleus",basedir);
	exit(-1);
    }
    fclose(fp);
    
    write_nucleusexports(TMPFILE,basedir);
    

    read_make_check(basedir,&head);
    napps = copy_apps(basedir,head);

    ptr = head;
    while(ptr->next)
    {
	tptr = ptr;
	ptr = ptr->next;
	free(tptr);
    }
    

    make_uids(&uids,napps);
    read_prognames(basedir,napps,&prognames);

    write_solution(basedir,prognames,uids,napps);
    write_projects(basedir,prognames,uids,napps);

    /* Create the zip archive */
    zip_up(basedir);

    return 0;
}




static void copy_nucleus(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest [MAXNAMLEN];

    sprintf(src,"%s/emboss/nucleus/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/nucleus",basedir);
    fix_dir(dest);
    
    sprintf(command,"%s %s %s",CP,src,dest);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(src,"%s/emboss/nucleus/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_ajax(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];
    
    /* First copy ajax .h & .c files */

    sprintf(src,"%s/emboss/ajax/pcre/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/pcre",basedir);
    fix_dir(dest);
    

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/ajax/pcre/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }



    
    sprintf(src,"%s/emboss/ajax/expat/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/expat",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/ajax/expat/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }




    sprintf(src,"%s/emboss/ajax/zlib/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/zlib",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/ajax/zlib/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }



    
    sprintf(src,"%s/emboss/ajax/core/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/core",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/ajax/core/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }



    
    sprintf(src,"%s/emboss/ajax/graphics/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/graphics",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/ajax/graphics/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }



    
    sprintf(src,"%s/emboss/ajax/ensembl/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/ensembl",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/ajax/ensembl/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }



    
    sprintf(src,"%s/emboss/ajax/ajaxdb/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/ajaxdb",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/ajax/ajaxdb/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }



    
    sprintf(src,"%s/emboss/ajax/acd/*.h",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/acd",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/ajax/acd/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }




    /* Copy extra files required by win32 */

    sprintf(src,"%s/emboss/win32/ajax/*",basedir);
    fix_dir(src);

    sprintf(dest,"%s/win32/ajax/core",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CPR,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    return;
}




static void copy_plplot(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];
    

    sprintf(src,"%s/emboss/plplot/*.h",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/plplot",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/plplot/*.c",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    
    sprintf(src,"%s/emboss/plplotwin/*.h",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/plplotwin/*.cpp",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(src,"%s/emboss/plplotwin/gd/include/*.h",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(src,"%s/emboss/plplotwin/haru/include/*.h",basedir);
    fix_dir(src);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(src,"%s/emboss/win32/DLLs/plplot/eplplot*",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/plplot",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(src,"%s/emboss/plplotwin/gd/lib/bgd.lib",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/plplot/lib/Debug",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(dest,"%s/win32/plplot/lib/Release",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(src,"%s/emboss/plplotwin/gd/lib/bgd.dll",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/Debug",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(dest,"%s/win32/DLLs/Release",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    
    sprintf(src,"%s/emboss/plplotwin/haru/lib/libharu.lib",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/plplot/lib/Debug",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(dest,"%s/win32/plplot/lib/Release",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }



    sprintf(src,"%s/emboss/plplotwin/haru/lib/libharu.dll",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/Debug",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(dest,"%s/win32/DLLs/Release",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }
    
    return;
}




static void copy_DLLs(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];
    
    /* First copy top level files */

    sprintf(src,"%s/emboss/win32/DLLs/DLLs.*",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    /* Copy ajax project files */

    sprintf(src,"%s/emboss/win32/DLLs/ajax/acd.vcxproj",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/ajax/acd",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/DLLs/ajax/ajaxdb.vcxproj",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/ajax/ajaxdb",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/DLLs/ajax/ajaxg.vcxproj",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/ajax/graphics",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/DLLs/ajax/core.vcxproj",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/ajax/core",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/DLLs/ajax/ensembl.vcxproj",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/ajax/ensembl",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/DLLs/ajax/expat.vcxproj",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/ajax/expat",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/DLLs/ajax/zlib.vcxproj",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/ajax/zlib",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/DLLs/ajax/pcre.vcxproj",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/ajax/pcre",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    /* Copy nucleus project files */

    sprintf(src,"%s/emboss/win32/DLLs/nucleus/*",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/nucleus",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CPR,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_data(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];

    
    sprintf(src,"%s/emboss/emboss/data/*",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/data",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CPR,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/plplot/lib/pl*",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/data",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CPFP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/misc/*.txt",basedir);
    fix_dir(src);
    
    sprintf(command,"%s %s %s",CPFP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/misc/emboss.default",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/apps/release",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CPFP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}



static void copy_doc(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];

    
    sprintf(src,"%s/emboss/doc/*",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/doc",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CPDFPR,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_jemboss(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];

    
    sprintf(src,"%s/emboss/jemboss/*",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/jemboss",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CPDFPR,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_test(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];

    
    sprintf(src,"%s/emboss/test/*",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/test",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CPDFPR,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_mysql(char *basedir)
{
    char command[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];

    
    sprintf(src,"%s/emboss/win32/mysql/Debug/*.lib",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/mysql/Debug",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/mysql/Debug/*.pdb",basedir);
    fix_dir(src);
    
    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    
    sprintf(src,"%s/emboss/win32/mysql/Release/*.lib",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/mysql/Release",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(src,"%s/emboss/win32/mysql/Release/*.pdb",basedir);
    fix_dir(src);
    
    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(src,"%s/emboss/win32/mysql/include/*.h",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/mysql/include",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/mysql/include/*.def",basedir);
    fix_dir(src);
    
    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/mysql/include/mysql/*.h",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/mysql/include/mysql",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    /* Copy dlls to top Release/Debug dirs for in-situ testing */
    
    sprintf(src,"%s/emboss/win32/mysql/Release/libmysql.dll",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/Release",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/mysql/Debug/libmysql.dll",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/DLLs/Debug",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_redist(char *basedir)
{
    char command[MAXNAMLEN];
    char prompt[MAXNAMLEN];
#ifndef WIN32    
    static char *def="/root/vc90";
    static char *def2="/root/vc80";
    static char *def3="/root/vc100";
#else
    static char *def="C:\\vc90";
    static char *def2="C:\\vc80";
    static char *def3="C:\\vc100";
#endif
    
    int len;
    
    char vc80[MAXNAMLEN];
    char vc90[MAXNAMLEN];
    char vc100[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];
    
    char *ev;

    ev = getenv("vc100files");

    if(ev)
        strcpy(vc100,ev);
    else
        strcpy(vc100,def3);

    fix_dir(vc100);

    fprintf(stdout,"Redist 100 file directory [%s]: ",vc100);
    fgets(prompt,MAXNAMLEN,stdin);
    if(*prompt == '\n')
        sprintf(prompt,"%s",vc100);

    len = strlen(prompt);
    if(prompt[len-1] == '\n')
        prompt[len-1] = '\0';

    
    sprintf(src,"%s/*",prompt);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/redist",basedir);
    fix_dir(dest);

    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    ev = getenv("vc90files");
    
    if(ev)
        strcpy(vc90,ev);
    else
        strcpy(vc90,def);
    
    fix_dir(vc90);
    
    fprintf(stdout,"Redist 90 file directory [%s]: ",vc90);
    fgets(prompt,MAXNAMLEN,stdin);
    if(*prompt == '\n')
        sprintf(prompt,"%s",vc90);

    len = strlen(prompt);
    if(prompt[len-1] == '\n')
        prompt[len-1] = '\0';

    sprintf(src,"%s/*",prompt);
    fix_dir(src);
    
    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    ev = getenv("vc80files");    

    if(ev)
        strcpy(vc80,ev);
    else
        strcpy(vc80,def2);

    fix_dir(vc80);
    
    fprintf(stdout,"Redist 80 file directory [%s]: ",vc80);
    fgets(prompt,MAXNAMLEN,stdin);
    if(*prompt == '\n')
        sprintf(prompt,"%s",vc80);

    len = strlen(prompt);
    if(prompt[len-1] == '\n')
        prompt[len-1] = '\0';

    sprintf(src,"%s/*",prompt);
    fix_dir(src);
    
    sprintf(command,"%s %s %s",CP,src,dest);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void zip_up(char *basedir)
{
#ifndef WIN32
    char command[MAXNAMLEN];

    fprintf(stdout,"Creating %s/emboss.zip\n",basedir);

    sprintf(command,"find %s/win32 -name CVS -exec rm -rf {} \\; "
	    ">/dev/null 2>&1",basedir,basedir);
    system(command);

    sprintf(command,"cd %s; zip -r emboss win32 >/dev/null 2>&1",basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"rm -rf %s/win32",basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }
#else
    (void) basedir;
#endif

    return;
}




static void create_directories(char *basedir)
{
    char newdir[MAXNAMLEN];
    static char *dirs[] = 
    {
	"win32",
	"win32/ajax",
        "win32/ajax/pcre",
        "win32/ajax/expat",
        "win32/ajax/zlib",
        "win32/ajax/core",
        "win32/ajax/graphics",
        "win32/ajax/ensembl",
        "win32/ajax/ajaxdb",
        "win32/ajax/acd",
	"win32/nucleus",
	"win32/emboss",
	"win32/data",
	"win32/doc",
	"win32/jemboss",
	"win32/test",
	"win32/apps",
	"win32/acd",
	"win32/DLLs",
        "win32/redist",
	"win32/plplot",
	"win32/plplot/lib",
	"win32/plplot/lib/Debug",
	"win32/plplot/lib/Release",
	"win32/DLLs/ajax",
	"win32/DLLs/ajax/pcre",
	"win32/DLLs/ajax/expat",
	"win32/DLLs/ajax/zlib",
	"win32/DLLs/ajax/core",
	"win32/DLLs/ajax/graphics",
	"win32/DLLs/ajax/ensembl",
	"win32/DLLs/ajax/ajaxdb",
	"win32/DLLs/ajax/acd",
	"win32/DLLs/ajax/core/Debug",
	"win32/DLLs/ajax/core/Release",
	"win32/DLLs/ajax/pcre/Debug",
	"win32/DLLs/ajax/pcre/Release",
	"win32/DLLs/ajax/expat/Debug",
	"win32/DLLs/ajax/expat/Release",
	"win32/DLLs/ajax/zlib/Debug",
	"win32/DLLs/ajax/zlib/Release",
	"win32/DLLs/ajax/graphics/Debug",
	"win32/DLLs/ajax/graphics/Release",
	"win32/DLLs/ajax/ensembl/Debug",
	"win32/DLLs/ajax/ensembl/Release",
	"win32/DLLs/ajax/ajaxdb/Debug",
	"win32/DLLs/ajax/ajaxdb/Release",
	"win32/DLLs/ajax/acd/Debug",
	"win32/DLLs/ajax/acd/Release",
	"win32/DLLs/nucleus",
	"win32/DLLs/nucleus/Debug",
	"win32/DLLs/nucleus/Release",
	"win32/DLLs/plplot",
	"win32/DLLs/plplot/Debug",
	"win32/DLLs/plplot/Release",
	"win32/DLLs/Debug",
	"win32/DLLs/Release",
	"win32/DLLs/_UpgradeReport_Files",
	"win32/apps/debug",
	"win32/apps/release",
	"win32/apps/emboss",
	"win32/apps/_UpgradeReport_Files",
	"win32/mysql",
	"win32/mysql/include",
 	"win32/mysql/include/mysql",
	"win32/mysql/Debug",
	"win32/mysql/Release",
	NULL
    };
    int i;
    int ret;

    i = 0;
    while(dirs[i])
    {
	sprintf(newdir,"%s/%s",basedir,dirs[i]);
        fix_dir(newdir);
        
	if(!dir_exists(newdir))
	{
#ifndef WIN32
	  ret = mkdir(newdir,0755);
#else
	  ret = mkdir(newdir);
#endif

	    if(ret == -1)
	    {
		fprintf(stderr,"Cannot create directory %s\n",newdir);
		exit(-1);
	    }
	}

	++i;
    }

    return;
}





static int header_exports(char *dir, FILE *fp)
{
    struct dirent *dp;
    DIR *indir;
    int len;
    char filename[MAXNAMLEN];

    indir = opendir(dir);    

    if(!indir)
	return -1;


    while((dp=readdir(indir)) != NULL)
    {
#if !defined(CYGWIN) & !defined(WIN32)
	if(!dp->d_ino || !strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#else
	if(!strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#endif
	len = strlen(dp->d_name);
	if(len > 2 && !strcmp(dp->d_name + len - 2, ".h"))
	{
	    sprintf(filename,"%s/%s",dir,dp->d_name);
            fix_dir(filename);
#ifdef DEBUG
	    fprintf(stdout,"Processing %s\n",filename);
#endif
	    extract_funcnames(filename,fp);
	}
	
    }

    closedir(indir);

    return 0;
}




static void extract_funcnames(char *filename, FILE *fout)
{
    FILE *inf;
    char line[MAXNAMLEN];
    char tline[MAXNAMLEN];
    
    int found;
    char c;
    char *p;
    char *q;
    int len;
    
    inf = fopen(filename,"r");

    if(!inf)
    {
	fprintf(stderr,"Cannot open header file %s\n",filename);
	exit(-1);
    }

    /* Look for start of prototypes */
    found = 0;
    while(fgets(line,MAXNAMLEN,inf))
	if(strstr(line,"Prototype definitions"))
	{
	    found = 1;
	    break;
	}

    if(!found)
    {
	fclose(inf);
	return;
    }


    while(fgets(line,MAXNAMLEN,inf))
    {
	if(strstr(line,"End of prototype"))
	    break;
	
	c = *line;
	if(c == ' ' || c=='\n' || c=='\t' || c=='*' || c=='#')
	    continue;
	if(!strncmp(line,"/*",2))
	    continue;

	/*
	** Expects start of a function definition here so looks for
	** an opening parenthesis. Exits if not found
        */
	p = line;
	while(*p && *p!='(')
	    ++p;
	if(!*p)
	{
	    fprintf(stderr,"Cannot find '(' in line:\n%s",line);
	    exit(-1);
	}

	/* Find last character of function name */
	--p;
	while(*p == ' ' || *p == '\t')
	    --p;
	
	q = p;

	/* Find start character of function name */
	while(*p != ' ' && *p != '\t')
	    --p;
	++p;
	while(*p=='*')
	    ++p;
	len = q - p + 1;

	strncpy(tline,p,len);
	tline[len] = '\0';

	fprintf(fout,"%s\n",tline);
    }

    fclose(inf);

    return;
}




static void write_coreexports(char *fn,char *basedir)
{
    FILE *outf;
    FILE *inf;
    
    char command[MAXNAMLEN];
    char line[MAXNAMLEN];
    char filename[MAXNAMLEN];
    
    sprintf(command,"sort %s > %s",fn, TMPFILE2);
    system(command);
    unlink(fn);

    sprintf(filename,"%s/win32/DLLs/ajax/core/%s",basedir,COREDEF);
    fix_dir(filename);
    
    outf = fopen(filename,"w");

    if(!outf)
    {
	fprintf(stderr,"Cannot open AJAX definitions file %s\n",filename);
	exit(-1);
    }

    fprintf(outf,"LIBRARY \t""ajax.dll""\n\n");
    fprintf(outf,"EXPORTS\n");

    fprintf(outf,"\tAssert_Failed\n");
    fprintf(outf,";dirent_w32\n");
    fprintf(outf,"\tclosedir\n\topendir\n\treaddir\n;others\n");

    inf = fopen(TMPFILE2,"r");

    if(!inf)
    {
	fprintf(stderr,"Cannot open sorted function file %s\n",TMPFILE2);
	exit(-1);
    }
    
    while(fgets(line,MAXNAMLEN,inf))
	fprintf(outf,"\t%s",line);
	
    fclose(inf);
    fclose(outf);

    unlink(TMPFILE2);

    return;
}




static void write_genajaxexport(char *fn,char *basedir,char *exportdef,
                                char *subdir,char *dllname)
{
    FILE *outf;
    FILE *inf;
    
    char command[MAXNAMLEN];
    char line[MAXNAMLEN];
    char filename[MAXNAMLEN];
    
    sprintf(command,"sort %s > %s",fn, TMPFILE2);
    system(command);
    unlink(fn);

    sprintf(filename,"%s/win32/DLLs/ajax/%s/%s",basedir,subdir,exportdef);
    fix_dir(filename);
    
    outf = fopen(filename,"w");

    if(!outf)
    {
	fprintf(stderr,"Cannot open %s definitions file %s\n",subdir,filename);
	exit(-1);
    }

    fprintf(outf,"LIBRARY \t""%s.dll""\n\n",dllname);
    fprintf(outf,"EXPORTS\n");

    inf=fopen(TMPFILE2,"r");

    if(!inf)
    {
	fprintf(stderr,"Cannot open sorted function file %s\n",TMPFILE2);
	exit(-1);
    }
    
    while(fgets(line,MAXNAMLEN,inf))
	fprintf(outf,"\t%s",line);
	
    fclose(inf);
    fclose(outf);

    unlink(TMPFILE2);

    return;
}




static void write_nucleusexports(char *fn,char *basedir)
{
    FILE *outf;
    FILE *inf;
    
    char command[MAXNAMLEN];
    char line[MAXNAMLEN];
    char filename[MAXNAMLEN];
    
    sprintf(command,"sort %s > %s",fn, TMPFILE2);
    system(command);
    unlink(fn);

    sprintf(filename,"%s/win32/DLLs/nucleus/%s",basedir,NUCLEUSDEF);
    fix_dir(filename);
    
    outf = fopen(filename,"w");

    if(!outf)
    {
	fprintf(stderr,"Cannot open NUCLEUS definitions file %s\n",filename);
	exit(-1);
    }

    fprintf(outf,"LIBRARY \t""nucleus.dll""\n\n");
    fprintf(outf,"EXPORTS\n");

    /*
    ** EmbPropTable no longer exists but leave this as an example
    ** of how to add rogue data definitions
    **    fprintf(outf,"\tEmbPropTable DATA\n;others\n");
    */

    inf = fopen(TMPFILE2,"r");

    if(!inf)
    {
	fprintf(stderr,"Cannot open sorted function file %s\n",TMPFILE2);
	exit(-1);
    }
    
    while(fgets(line,MAXNAMLEN,inf))
	fprintf(outf,"\t%s",line);
	
    fclose(inf);
    fclose(outf);

    unlink(TMPFILE2);

    return;
}




static int dir_exists(char *path)
{
    struct stat buf;

    if(stat(path,&buf) == -1)
	return 0;

    if((int)buf.st_mode & S_IFDIR)
	return 1;
    else
    {
	fprintf(stderr,"%s exists but isn't a directory\n",path);
	exit(-1);
    }
    
    return 0;
}




static int file_exists(char *path)
{
    struct stat buf;

    if(stat(path,&buf) == -1)
	return 0;

    if((int)buf.st_mode & S_IFREG)
	return 1;
    else
    {
	fprintf(stderr,"%s exists but isn't a regular file\n",path);
	exit(-1);
    }

    return 0;
}




static int copy_apps(char *basedir, listnode *head)
{
    struct dirent *dp;
    DIR *indir;
    int len;
    char dir[MAXNAMLEN];
    int ignore;
    listnode *listptr;
    char command[MAXNAMLEN];
    char acdname[MAXNAMLEN];
    char filename[MAXNAMLEN];
    char src[MAXNAMLEN];
    char dest[MAXNAMLEN];
    
    struct stat buf;
    int count;
    int i;
    int excluded;
    
    fprintf(stdout,"Copying applications\n");
    
    sprintf(dir,"%s/emboss/emboss",basedir);
    fix_dir(dir);
    
    indir = opendir(dir);

    if(!indir)
    {
	fprintf(stderr,"Can't locate directory %s\n",dir);
	exit(-1);
    }
    
    count = 0;

    while((dp=readdir(indir)) != NULL)
    {
#if !defined(CYGWIN) & !defined(WIN32)
	if(!dp->d_ino || !strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#else
	if(!strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#endif
	len = strlen(dp->d_name);
	if(len > 2)
	{
	    sprintf(filename,"%s/%s",dir,dp->d_name);
            fix_dir(filename);
        
	    stat(filename,&buf);
	    if((int)buf.st_mode & S_IFDIR)
		continue;

	    len = strlen(dp->d_name);
	    if(strcmp(dp->d_name + len -2,".c"))
		continue;

            i = 0;
            excluded = 0;
            while(exclude_names[i])
            {
                if(!strcmp(dp->d_name,exclude_names[i]))
                {
                    excluded = 1;
                    break;
                }

                ++i;
            }

            if(excluded)
                continue;

	    ignore = 0;
	    listptr = head;
	    while(listptr->next)
	    {
		if(!strcmp(listptr->prog,dp->d_name))
		{
		    ignore = 1;
		    break;
		}
		listptr = listptr->next;
	    }
	    

	    if(ignore)
		continue;

	    ++count;	    
	    
            sprintf(src,"%s/%s",dir,dp->d_name);
            fix_dir(src);
            
            sprintf(dest,"%s/win32/emboss",basedir);
            fix_dir(dest);

            sprintf(command,"%s %s %s",CP,src,dest);

	    if(system(command))
	    {
		fprintf(stderr,"Can't execute %s\n",command);
		exit(-1);
	    }
	    
	    strcpy(acdname,dp->d_name);
	    len = strlen(acdname);
	    acdname[len-2] = '\0';
	    strcat(acdname,".acd");

	    sprintf(filename,"%s/acd/%s",dir,acdname);
            fix_dir(filename);
            
	    if(stat(filename,&buf) == -1)
		continue;

            sprintf(src,"%s/acd/%s",dir,acdname);
            fix_dir(src);
            
            sprintf(dest,"%s/win32/acd",basedir);
            fix_dir(dest);

            sprintf(command,"%s %s %s",CP,src,dest);

	    if(system(command))
	    {
		fprintf(stderr,"Can't execute %s\n",command);
		exit(-1);
	    }
	}
    }
    

    closedir(indir);


    sprintf(src,"%s/acd/codes.english",dir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/acd",basedir);
    fix_dir(dest);
    
    sprintf(command,"%s %s %s",CP,src,dest);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/acd/*.standard",dir);
    fix_dir(src);
    
    sprintf(command,"%s %s %s",CP,src,dest);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(src,"%s/emboss/win32/exes/*.exe",basedir);
    fix_dir(src);
    
    sprintf(dest,"%s/win32/apps/release",basedir);
    fix_dir(dest);
    
    sprintf(command,"%s %s %s",CP,src,dest);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }
    


    return count;
}




static void read_make_check(char *basedir, listnode **head)
{
    char filename[MAXNAMLEN];
    FILE *fp;
    char line[MAXNAMLEN];
    char tmpstr[MAXNAMLEN];
    char progname[MAXNAMLEN];
    
    int found;
    int len;
    char *p;
    
    sprintf(filename,"%s/emboss/emboss/Makefile.am",basedir);
    fix_dir(filename);
    
    fp=fopen(filename,"r");
    
    if(!fp)
    {
	fprintf(stderr,"Cannot open %s\n",filename);
	exit(-1);
    }

    found = 0;
    
    while(fgets(line,MAXNAMLEN,fp))
	if(strstr(line,"check_PROGRAMS ="))
	{
	    found = 1;
	    break;
	}

    if(!found)
    {
	fprintf(stderr,"Cannot find ""check_PROGRAMS ="" line\n");
	exit(-1);
    }
    
    len = strlen(line);
    if(line[len-1] == '\n')
	line[len-1] = '\0';

    while(*line)
    {
	p = line;
	if(strstr(line,"check_PROGRAMS"))
	{
	    while(*p != '=')
		++p;
	    ++p;
	    while(*p == ' ' || *p == '\t')
		++p;
	}
	else
	{
	    while(*p && (*p == ' ' || *p == '\t'))
		++p;
	}
	strcpy(tmpstr,p);	

	p = strtok(tmpstr," \\\t\n");
	if(p)
	{
	    sprintf(progname,"%s.c",p);
	    add_node(head,progname);
	}
	
	while((p = strtok(NULL," \\\t\n")) != NULL)
	{
	    sprintf(progname,"%s.c",p);
	    add_node(head,progname);
	}

	if(!fgets(line,MAXNAMLEN,fp))
	{
	    fprintf(stderr,"Unexpected EOF in file %s\n",filename);
	    exit(-1);
	}

	p = line;
	len = strlen(line);
	if(line[len-1]=='\n')
	    line[len-1] = '\0';
	while(*p && (*p == ' ' || *p == '\t'))
	    ++p;
	if(!*p)
	    *line = '\0';
    }

    fclose(fp);

    return;
}




static void add_node(listnode **head, char *progname)
{
    listnode *tmp;

    tmp = malloc(sizeof(*tmp));

    if(!tmp)
    {
	fprintf(stderr,"Malloc failure creating list node\n");
	exit(-1);
    }

    strcpy(tmp->prog,progname);
    tmp->next = *head;
    *head = tmp;

    return;
}




static void make_uids(char ***uids, int napps)
{
    static char *uidext="-fc97-11d7-88ec-005056c00008";
    unsigned int uid;
    int i;
    char **uida;
    
    uid = 0xf34f5640;

    *uids = (char **) malloc(sizeof(char **) * napps);
    if(!*uids)
    {
	fprintf(stderr,"Error mallocing uid array\n");
	exit(-1);
    }

    uida = *uids;
    
    for(i=0;i<napps;++i)
    {
	uida[i] = (char *) malloc(MAXNAMLEN);
	if(!uida[i])
	{
	    fprintf(stderr,"Error mallocing uid array element\n");
	    exit(-1);
	}

	sprintf(uida[i],"%x%s",uid,uidext);
	++uid;
    }

    return;
}




static void read_prognames(char *basedir, int napps, char ***names)
{
    struct dirent *dp;
    DIR *indir;
    int len;
    char filename[MAXNAMLEN];
    char dir[MAXNAMLEN];
    char **pnames;
    int i;
    int j;
    int excluded;
    
    fprintf(stdout,"Reading program names\n");

    *names = (char **) malloc(sizeof(char **) * napps);
    if(!*names)
    {
	fprintf(stderr,"Error mallocing uid array\n");
	exit(-1);
    }

    pnames = *names;
    
    for(i=0;i<napps;++i)
    {
	pnames[i] = (char *) malloc(MAXNAMLEN);
	if(!pnames[i])
	{
	    fprintf(stderr,"Error mallocing progname array element\n");
	    exit(-1);
	}
    }


    sprintf(dir,"%s/win32/emboss",basedir);
    fix_dir(dir);
    
    indir = opendir(dir);

    if(!indir)
    {
	fprintf(stderr,"Error opening directory %s\n",dir);
	exit(-1);
    }

    i = 0;
    
    while((dp = readdir(indir)) != NULL)
    {
#if !defined(CYGWIN) & !defined(WIN32)
	if(!dp->d_ino || !strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#else
	if(!strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#endif

        j = 0;
        excluded = 0;
        while(exclude_names[j])
        {
            if(!strcmp(dp->d_name,exclude_names[j]))
            {
                printf("Found %s\n",dp->d_name);
                
                excluded = 1;
                break;
            }

            ++j;
        }
        

        len = strlen(dp->d_name);
	if(len > 2 && !strcmp(dp->d_name + len - 2, ".c") && !excluded)
	{
	    sprintf(filename,"%s",dp->d_name);
	    filename[len-2] = '\0';
            fix_dir(filename);
	    strcpy(pnames[i],filename);
	    ++i;
	}
	
    }

    closedir(indir);


    return;
}




static void write_solution(char *basedir, char **prognames, char **uids,
			   int napps)
{
    char filename[MAXNAMLEN];
    FILE *fp;
    int i;
    char *p;
    

    for(i=0;i<napps;++i)
    {
	p = uids[i];
	while(*p)
	{
	    *p = (char) toupper((int)*p);
	    ++p;
	}
    }
    

    sprintf(filename,"%s/win32/apps/apps.sln",basedir);
    fix_dir(filename);
    
    fp=fopen(filename,"w");    
    if(!fp)
    {
	fprintf(stderr,"Cannot open %s\n",filename);
	exit(-1);
    }


    fprintf(fp,"Microsoft Visual Studio Solution File, Format Version 11.00\n");
    fprintf(fp,"# Visual C++ Express 2010\n");


    for(i=0;i<napps;++i)
    {
	fprintf(fp,"%s = \"%s\", \"emboss\\%s\\%s.vcxproj\", \"{%s}\"\n",
		PROJECT,prognames[i],prognames[i],prognames[i],uids[i]);
	fprintf(fp,"EndProject\n");
    }

    fprintf(fp,"Global\n");
    fprintf(fp,"\tGlobalSection(SolutionConfigurationPlatforms"
	    " = preSolution\n");
    fprintf(fp,"\t\tDebug|Win32 = Debug|Win32\n");
    fprintf(fp,"\t\tRelease|Win32 = Release|Win32\n");
    fprintf(fp,"\tEndGlobalSection\n");
    fprintf(fp,"\tGlobalSection(ProjectConfigurationPlatforms)"
	    " = postSolution\n");

    for(i=0;i<napps;++i)
    {
	fprintf(fp,"\t\t{%s}.Debug|Win32.ActiveCfg = Debug|Win32\n",
		uids[i]);
	fprintf(fp,"\t\t{%s}.Debug|Win32.Build.0 = Debug|Win32\n",
		uids[i]);
	fprintf(fp,"\t\t{%s}.Release|Win32.ActiveCfg = Release|Win32\n",
		uids[i]);	
	fprintf(fp,"\t\t{%s}.Release|Win32.Build.0 = Release|Win32\n",
		uids[i]);
    }

    fprintf(fp,"\tEndGlobalSection\n");
    fprintf(fp,"\tGlobalSection(SolutionProperties) = preSolution\n");
    fprintf(fp,"\t\tHideSolutionNode = FALSE\n");
    fprintf(fp,"\tEndGlobalSection\n");


    fprintf(fp,"\tGlobalSection(SolutionConfigurationPlatforms ) "
            "= preSolution\n");
    fprintf(fp,"\t\tDebug|Win32 = Debug|Win32\n");
    fprintf(fp,"\t\tRelease|Win32 = Release|Win32\n");
    fprintf(fp,"\tEndGlobalSection\n");
    

    fprintf(fp,"EndGlobal\n");
    
    fclose(fp);

    chmod(filename,0755);

    return;
}




static void write_projects(char *basedir, char **prognames, char **uids,
			   int napps)
{
    char filename[MAXNAMLEN];
    int i;
    int j;
    FILE *inf;
    FILE *outf;
    char line[MAXNAMLEN];
    char tline[MAXNAMLEN];
    int found;
    int ret;


    for(i=0;i<napps;++i)
    {
	sprintf(filename,"%s/win32/apps/emboss/%s",basedir,prognames[i]);
        fix_dir(filename);
        
#ifndef WIN32
	ret = mkdir(filename,0755);
#else
	ret = mkdir(filename);
#endif

	if(ret == -1)
	{
	    fprintf(stderr,"Cannot create project directory %s\n",filename);
	    exit(-1);
	}
    }

    for(i=0;i<napps;++i)
    {
	sprintf(filename,"%s/win32/apps/emboss/%s/%s.vcxproj",basedir,
		prognames[i],prognames[i]);
        fix_dir(filename);
        
	outf = fopen(filename,"w");
	if(!outf)
	{
	    fprintf(stderr,"Cannot open %s\n",filename);
	    exit(-1);
	}

	sprintf(filename,"%s/emboss/win32/apps/template.vcxproj",basedir);
        fix_dir(filename);
        
	inf = fopen(filename,"r");

	if(!inf)
	{
	    fprintf(stderr,"Cannot open %s\n",filename);
	    exit(-1);
	}

	while(fgets(line,MAXNAMLEN,inf))
	{
	    if(strstr(line,"<PROGNAME>"))
	    {
		sub_text(line,tline,"<PROGNAME>",prognames[i]);
		fprintf(outf,"%s",tline);
		continue;
	    }

	    if(strstr(line,"<UID>"))
	    {
		sub_text(line,tline,"<UID>",uids[i]);
		fprintf(outf,"%s",tline);
		continue;
	    }


	    if(strstr(line,"<STACKSIZE>"))
	    {
		j = 0;
		found = 0;
		while(memory[j].progname)
		{
		    if(!strcmp(memory[j].progname,prognames[i]))
		    {
			found = 1;
			break;
		    }

		    ++j;
		}
		

		if(found)
		    fprintf(outf,"      <StackReserveSize>%s"
                            "</StackReserveSize>\n",
			    memory[j].stacksize);

		continue;
	    }

	    if(strstr(line,"<HEAPSIZE>"))
	    {
		j = 0;
		found = 0;
		while(memory[j].progname)
		{
		    if(!strcmp(memory[j].progname,prognames[i]))
		    {
			found = 1;
			break;
		    }

		    ++j;
		}
		

		if(found)
		    fprintf(outf,"      <HeapReserveSize>%s"
                            "</HeapReserveSize>\n",
			    memory[j].heapsize);

		continue;
	    }

	    fprintf(outf,"%s",line);
	}
	
	fclose(inf);
	fclose(outf);

	sprintf(filename,"%s/win32/apps/emboss/%s/%s.vcxproj",basedir,
		prognames[i],prognames[i]);
        fix_dir(filename);
        
	chmod(filename,0755);
    }
    
    return;
}




static void sub_text(char *line, char *tline, char *given, char *rep)
{
    char *p;

    p = strstr(line,given);
    strncpy(tline,line,p-line);
    tline[p-line] = '\0';
    strcat(tline,rep);

    p += strlen(given);
    strcat(tline,p);

    return;
}



static void make_ajax_header_exports(char *basedir,char *subdir)
{
    char defsdir[MAXNAMLEN];
    FILE *fp;
    

    sprintf(defsdir,"%s/emboss/ajax/%s",basedir,subdir);
    fix_dir(defsdir);
    
    fp = fopen(TMPFILE,"w");
    
    if(!fp)
    {
	fprintf(stderr,"Cannot open temporary file %s\n",TMPFILE);
	exit(-1);
    }
    

    if(header_exports(defsdir,fp) < 0)
    {
	fprintf(stderr,"Cannot open directory %s/emboss/ajax/%s",basedir,
                subdir);
	exit(-1);
    }

    fclose(fp);

    return;
}




static void make_expat_header_exports(char *basedir)
{
    char hfname[MAXNAMLEN];
    FILE *fp;
    

    sprintf(hfname,"%s/emboss/ajax/expat/expat.h",basedir);
    fix_dir(hfname);
    
    fp = fopen(TMPFILE,"w");

    if(!fp)
    {
	fprintf(stderr,"Cannot open temporary file %s\n",TMPFILE);
	exit(-1);
    }

    extract_expat_funcnames(hfname,fp);

    fclose(fp);
    
    return;
}




static void extract_expat_funcnames(char *filename, FILE *fout)
{
    FILE *inf;
    char line[MAXNAMLEN];
    char tline[MAXNAMLEN];
    
    char *p;
    char *q;
    int len;
    
    inf = fopen(filename,"r");

    if(!inf)
    {
	fprintf(stderr,"Cannot open header file %s\n",filename);
	exit(-1);
    }


    while(fgets(line,MAXNAMLEN,inf))
    {
	if(!strstr(line,"XMLPARSEAPI"))
	    continue;

        if(!fgets(line,MAXNAMLEN,inf))
        {
            fprintf(stderr,"Error in expat.h: expected line after "
                    "XMLPARSEAPI\n");
            exit(1);
        }

	/*
	** Expects start of a function definition here so looks for
	** an opening parenthesis. Exits if not found
        */
	p = line;
	while(*p && *p!='(')
	    ++p;
	if(!*p)
	{
	    fprintf(stderr,"Cannot find '(' in line:\n%s",line);
	    exit(-1);
	}

	/* Find last character of function name */
	--p;
	while(*p == ' ' || *p == '\t')
	    --p;
	
	q = p;

	/* Find start character of function name */
	while(*p != ' ' && *p != '\t' && p != line)
	    --p;

        if(p != line)
            ++p;

	while(*p=='*')
	    ++p;

	len = q - p + 1;

	strncpy(tline,p,len);
	tline[len] = '\0';

	fprintf(fout,"e%s\n",tline);
    }

    fclose(inf);

    return;
}




static void make_zlib_header_exports(char *basedir)
{
    char hfname[MAXNAMLEN];
    FILE *fp;
    

    sprintf(hfname,"%s/emboss/ajax/zlib/zlib.h",basedir);
    fix_dir(hfname);
    
    fp = fopen(TMPFILE,"w");

    if(!fp)
    {
	fprintf(stderr,"Cannot open temporary file %s\n",TMPFILE);
	exit(-1);
    }

    extract_zlib_funcnames(hfname,fp);

    fclose(fp);
    
    return;
}




static void extract_zlib_funcnames(char *filename, FILE *fout)
{
    FILE *inf;
    char line[MAXNAMLEN];
    char tline[MAXNAMLEN];
    
    char *p;
    char *q;
    int len;

    inf = fopen(filename,"r");    

    if(!inf)
    {
	fprintf(stderr,"Cannot open header file %s\n",filename);
	exit(-1);
    }


    while(fgets(line,MAXNAMLEN,inf))
    {
        if(!strncmp(line,"/*",2))
        {
            while(strncmp(line,"*/",2))
                if(!fgets(line,MAXNAMLEN,inf))
                {
                    fprintf(stderr,"extract_zlib_funcnames: Can't find /*\n");
                    exit(1);
                }
            
            continue;
        }

        if(!strstr(line,"ZEXTERN"))
	    continue;

	/*
	** Expects start of a function definition here so looks for
	** "OF(". Exits if not found
        */
        p = strstr(line,"OF(");
	if(!p)
	{
	    fprintf(stderr,"Cannot find 'OF(' in line:\n%s",line);
	    exit(-1);
	}

	/* Find last character of function name */
	--p;
	while(*p == ' ' || *p == '\t')
	    --p;
	
	q = p;

	/* Find start character of function name */
	while(*p != ' ' && *p != '\t' && p != line)
	    --p;

        ++p;

	while(*p=='*')
	    ++p;

	len = q - p + 1;

	strncpy(tline,p,len);
	tline[len] = '\0';

	fprintf(fout,"e%s\n",tline);
    }

    fclose(inf);

    return;
}




static void fix_dir(char *str)
{
#ifdef WIN32
    char *p;

    p = str;

    while(*p)
    {
        if(*p == '/')
            *p = '\\';

        ++p;
    }
#else
    (void) str;
#endif
    
    return;
}
