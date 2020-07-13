#ifdef __cplusplus
extern "C"
{
#endif
#ifdef HAVE_ORB_ORBIT_H


#ifndef ajcorba_h
#define ajcorba_h

/* @data AjPCorbatype *********************************************************
**
** AJAX CORBA type data structure
**
******************************************************************************/

typedef struct AjSCorbatype
{
    AjPStr       Name;
    AjPStr       Source;
    AjPStr       Id;
    ajint          Start;
    ajint          End;
    short        Strand;
    ajint          Ntags;
    AjPStr       *Tag;
    ajint          *Nval;
    AjPStr       **Val;
    ajint          Nlocs;
    ajint          *LSpos;
    ajint	         *LSex;
    ajint          *LSfuzzy;
    ajint          *LEpos;
    ajint          *LEex;
    ajint          *LEfuzzy;
    ajint          *LStrand;
    AjPStr       Seq;
} AjOCorbatype,*AjPCorbatype;

/* @data AjPCorbafeat *********************************************************
**
** AJAX CORBA feature data structure
**
******************************************************************************/

typedef struct AjSCorbafeat
{
    ajint          Ntypes;
    AjPCorbatype *Types;
} AjOCorbafeat, *AjPCorbafeat;


void         ajCorbafeatDel(AjPCorbafeat *thys);
void         ajCorbatypeDel(AjPCorbatype *thys);
AjPCorbafeat ajCorbafeatNew(ajint ntypes);
AjPCorbatype ajCorbatypeNew(ajint ntags, ajint nlocs);
AjPStr       ajSeqCorbaEmbl(char *acc, char **exerr, ajint *exint,
			    AjPCorbafeat *feat, AjBool dofeat);


#endif
#endif

#ifdef __cplusplus
}
#endif

