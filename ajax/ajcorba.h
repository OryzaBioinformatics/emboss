#ifdef __cplusplus
extern "C"
{
#endif
#ifdef HAVE_ORB_ORBIT_H


#ifndef ajcorba_h
#define ajcorba_h

typedef struct AjSCorbatype
{
    AjPStr       Name;
    AjPStr       Source;
    AjPStr       Id;
    int          Start;
    int          End;
    short        Strand;
    int          Ntags;
    AjPStr       *Tag;
    int          *Nval;
    AjPStr       **Val;
    int          Nlocs;
    int          *LSpos;
    int	         *LSex;
    int          *LSfuzzy;
    int          *LEpos;
    int          *LEex;
    int          *LEfuzzy;
    int          *LStrand;
    AjPStr       Seq;
} AjOCorbatype,*AjPCorbatype;

typedef struct AjSCorbafeat
{
    int          Ntypes;
    AjPCorbatype *Types;
} AjOCorbafeat, *AjPCorbafeat;


void         ajCorbafeatDel(AjPCorbafeat *thys);
void         ajCorbatypeDel(AjPCorbatype *thys);
AjPCorbafeat ajCorbafeatNew(int ntypes);
AjPCorbatype ajCorbatypeNew(int ntags, int nlocs);
AjPStr       ajSeqCorbaEmbl(char *acc, char **exerr, int *exint,
			    AjPCorbafeat *feat, AjBool dofeat);


#endif
#endif

#ifdef __cplusplus
}
#endif

