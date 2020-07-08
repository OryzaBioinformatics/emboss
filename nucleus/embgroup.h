/*  Last edited: Mar  1 17:30 2000 (pmr) */
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embgroup_h
#define embgroup_h



/* GROUP NODE */
/*
This serves as both a node in a list of names of groups which each hold
a list of details of programs (names and documentation) and also
it is a node in a list of the details of programs (names and documentation).

Using the same structure for both is a bit confusing, but it simplifies
some of the routines which search and output the lists of gnodes of
groups and program data. 

GROUP LIST 	nodes point to	PROGRAM LISTS
----------			-------------

group gnode 	-> program gnode - program gnode - program gnode - etc.
    |
group gnode 	-> program gnode - program gnode - program gnode - etc.
    |
   etc. 


The layout of the 'alpha' list of alphabetic listing of applications is
a bit different - instead of applications being grouped, as in 'glist'
above, the applications all come under one major group and each
application holds a list of the groups it belongs to:

ALPHA LIST
----------

group gnode -> program gnode -> group gnode - group gnode - group gnode - etc.
		   |
	       program gnode -> group gnode - group gnode - group gnode - etc.
		   |
	       program gnode -> group gnode - group gnode - group gnode - etc.
		   |
	       program gnode -> group gnode - group gnode - group gnode - etc.
		   |
	       program gnode -> group gnode - group gnode - group gnode - etc.
                   |
                  etc.
                  
                 

*/
typedef struct gnode {
  AjPStr name;	/* name of group or of program */
  AjPStr doc;	/* documentation for this program (used by list of programs) */
  AjPList progs; /* list of programs in this group (used by list of groups) */
} Gnode, *GPnode;


int    embGrpCompareTwoGnodes(const void * a, const void * b);
void   embGrpGetProgGroups (AjPList glist, AjPList alpha, char **env, 
			AjBool emboss, AjBool embassy, AjBool explode);
void   embGrpGroupsListDel (AjPList *groupslist);
void   embGrpKeySearchProgs (AjPList newlist, AjPList glist, AjPStr key);
void   embGrpKeySearchSeeAlso(AjPList newlist, AjPList *appgroups,
        AjPList alpha, AjPList glist, AjPStr key);
GPnode embGrpMakeNewGnode (AjPStr name);
GPnode embGrpMakeNewPnode (AjPStr name, AjPStr doc);
void   embGrpOutputGroupsList (AjPFile outfile, AjPList groupslist,
			       AjBool showprogs, AjBool html, AjPStr link1,
			       AjPStr link2);
void   embGrpOutputProgsList (AjPFile outfile, AjPList progslist,
			      AjBool html, AjPStr link1, AjPStr link2);
void   embGrpSortGroupsList (AjPList groupslist);
void   embGrpMakeUnique(AjPList list);

#endif

#ifdef __cplusplus
}
#endif
