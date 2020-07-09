#include "emboss.h"


AjPList buildListFromString(AjPStr string, AjPSeq seq, AjBool feat){
  AjPRegexp commaseparated = NULL;
  AjPList newlist = NULL;
  AjPStr test=NULL;
  AjPFeatVocFeat item=NULL;
  AjPFeatVocTag item2=NULL;


  if(ajStrLen(string) <= 1)
    return NULL;

  commaseparated = ajRegCompC("([^, ]+)") ;

  while(ajRegExec(commaseparated,string)){
    ajRegSubI(commaseparated, 1, &test) ;
    /*    ajUser("test = *%S*\n",test);*/
    (void) ajRegPost(commaseparated,&string);    
    /*    ajUser("remainder = *%S*\n",string);*/

    if(feat){
      item = CheckDictForFeature(seq->Fttable,test);

      if(item){
	if(!newlist)
	  newlist = ajListNew();
	ajListPush(newlist,item);
      }
      else{
	ajWarn("%S not a valid Feature hence not used\n",test);
      }
      ajStrDel(&test);
    }
    else{
      item2 = CheckDictForTag(seq->Fttable,test);
      
      if(item2){
	if(!newlist)
	  newlist = ajListNew();
	ajListPush(newlist,item2);
      }
      else{
	ajWarn("%S not a valid Tag hence not used\n",test);
      }
      ajStrDel(&test);
   }
  }

  ajRegFree(&commaseparated);
  
  return newlist;
}

int main (int argc, char * argv[]) {

  AjPSeq seq;
  AjPSeqout seqout;
  AjPStr ignore=NULL,onlyallow=NULL;
  AjPStr ignore2=NULL,onlyallow2=NULL;
  AjPList newlist=NULL;
  AjBool sortbytype,sortbystart;

  embInit ("seqretfeat", argc, argv);

  seq        = ajAcdGetSeq ("sequence");
  seqout     = ajAcdGetSeqout ("outseq");

  ignore     = ajAcdGetString("featignore");
  onlyallow  = ajAcdGetString("featonlyallow");

  ignore2    = ajAcdGetString("tagignore");
  onlyallow2 = ajAcdGetString("tagonlyallow");

  sortbytype = ajAcdGetBool("sortbytype");
  sortbystart = ajAcdGetBool("sortbystart");


  /* Process Features */
  newlist =  buildListFromString(ignore, seq, ajTrue);
  if(newlist){
    ajDebug("Process featignore\n");
    ajFeatIgnoreFeat(seq->Fttable,newlist);
    if(ajStrLen(onlyallow)> 1)
      ajWarn("-featonlyallow option ignored as -featignore specified aswell");
  }
  else {
    newlist =  buildListFromString(onlyallow, seq, ajTrue);
    if(newlist)
      ajFeatOnlyAllowFeat(seq->Fttable,newlist);
  }

  /* Process Tags */
  newlist =  buildListFromString(ignore2, seq, ajFalse);
  if(newlist){
    ajFeatIgnoreTag(seq->Fttable,newlist);
    if(ajStrLen(onlyallow2)> 1)
      ajWarn("-tagonlyallow option ignored as -tagignore specified aswell");
  }
  else {
    newlist =  buildListFromString(onlyallow2, seq, ajFalse);
    if(newlist)
      ajFeatOnlyAllowTag(seq->Fttable,newlist);
  }

  if(sortbytype)
    ajFeatSortByType(seq->Fttable);
  else if(sortbystart)
    ajFeatSortByStart(seq->Fttable);

  ajSeqWrite (seqout, seq);
  ajSeqTrace (seq);
  ajSeqWriteClose (seqout);
  ajSeqDel (&seq);

  ajExit ();
  return 0;
}
