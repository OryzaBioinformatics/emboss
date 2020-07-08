/*  Last edited: Feb 24 14:08 2000 (pmr) */
#include "ajax.h"
#include "emboss.h"
#include "ajmess.h"

int main(int argc, char * argv[]){
  AjPStr str1;

  ajMessErrorInit("Mess Test");

  ajMessOut("Hello");

  ajMessOut("");

  ajMessBeep();
  
  ajMessDump("dumping.....");

  ajMessError("Unknown error??");

  if(ajMessErrorSetFile("/nfs/adnah/il/JUNK"))
    ajMessOut("ajMessErrorSetFile OKAY");
  else
    ajMessOut("ajMessErrorSetFile messed up");

  ajMessOutCode("BUSERR");

  ajMessOutCode("MIST");
  
  ajMessOutCode("ERROR Bus Error");

  if(ajMessErrorSetFile("/nfs/adnah/il/messages/messages.english"))
    ajMessOut("ajMessErrorSetFile OKAY");
  else
    ajMessOut("ajMessErrorSetFile messed up");

  ajMessOutCode("BUSERR");

  ajMessOutCode("MIST");
  
  ajMessOutCode("ERROR Bus Error");


  str1 = ajStrNewC("STR output...");

  ajFmtPrint("AGAIN..%S\n",str1);

  ajMessOut("*** %S +++",str1);



  /*  ajMessCrash("Memory Error.");*/


  ajMessErrorCode("BUSERR");

  ajMessCodesDelete();


  ajMessCrashCode("BUSERR");

  ajFatal("Should not get here");
  ajExit();
  return 0;
}

