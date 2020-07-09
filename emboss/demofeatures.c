#include "emboss.h"

int main(int argc, char **argv)
{
  AjPFeatLexicon dict=NULL;
  AjPFeatTable feattable;
  AjPStr name=NULL,score=NULL,desc=NULL,source=NULL,type=NULL;
  AjEFeatStrand strand=AjStrandWatson;
  AjEFeatFrame frame=AjFrameUnknown;
  AjBool sortbytype,dictionary,sortbystart,tracedict;
  AjPFile file;
  AjPFeature feature;
  AjPFeatTabOut output = NULL;
  ajint i;

  embInit ("demofeatures", argc, argv);

  /*  file =        ajAcdGetOutfile("outfile");*/
  output     =  ajAcdGetFeatout("featout");
  dictionary =  ajAcdGetBool("dictionary");
  sortbytype =  ajAcdGetBool("typesort");
  sortbystart = ajAcdGetBool("startsort");
  tracedict =   ajAcdGetBool("tracedict");

  /* first read the dictionary if one is to be used */

  if(dictionary) 
    dict = ajFeatGffDictionaryCreate(); 

  ajStrAssC(&name,"seq1");

  feattable = ajFeatTabNew(name,dict);
  if(!dictionary)
    dict = feattable->Dictionary;

  ajStrAssC(&source,"demofeature");
  ajStrAssC(&score,"1.0");


  
  for(i=1;i<11;i++){
    if(i & 1)
      ajStrAssC(&type,"CDS");
    else
      ajStrAssC(&type,"misc_feature");

    feature = ajFeatureNew(feattable, source, type,
			  i, i+10, score, strand, frame,
			  desc , 0, 0) ;    

  }
  
  
  if(sortbytype)
    ajFeatSortByType(feattable);
  if(sortbystart)
    ajFeatSortByStart(feattable);


  if(tracedict) /* -debug need to be on aswell for this to be visible!! */
    ajFeatDickTracy(dict);

  ajFeaturesWrite (output, feattable);
  
  ajStrDel(&name);
  ajStrDel(&score);
  ajStrDel(&type);

  return 0;
}
