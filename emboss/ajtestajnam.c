/*  Last edited: May 13 11:52 1999 (ajb) */
/*
**
** Test routine for ajnam functions from AJAX.
**
*/
#include "emboss.h"

int main()
{ 
  AjPStr value = 0;
  char *name[] = {"end","acdroot","dataroot","embldir","configdir", NULL};
  int i;

  ajNamInit("emboss");

  /* Print out the source file name(s) */
  ajNamListOrigin(); 

  /* Print out the ENV values  TEST*/
  ajNamEnvironmentals();
  
  /* Print out the database values TEST*/
  ajNamListDatabases(); 


  for (i=0; name[i]; i++) {
    if (ajNamGetValueC(name[i], &value))     
      ajFmtPrint("FOUND: %s '%S'\n",name[i],value);
    else
      ajFmtPrint(" UNable to find %s\n",name[i]);
  }

  /*  ajNamExit(); Done by ajExit now */

  ajStrDel(&value);
  ajExit();
  return 0;
}







