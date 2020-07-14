#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embgroup_h
#define embgroup_h


/* @data GPnode ***************************************************************
** This serves as both a node in a list of names of groups which each hold
** a list of details of programs (names and documentation) and also
** it is a node in a list of the details of programs (names and documentation).
**
** Using the same structure for both is a bit confusing, but it simplifies
** some of the routines which search and output the lists of gnodes of
** groups and program data.
**
** GROUP LIST 	nodes point to	PROGRAM LISTS
** ----------			-------------
**
** group gnode 	-> program gnode - program gnode - program gnode - etc.
**     |
** group gnode 	-> program gnode - program gnode - program gnode - etc.
**     |
**    etc.
**
**
** The layout of the 'alpha' list of alphabetic listing of applications is
** a bit different - instead of applications being grouped, as in 'glist'
** above, the applications all come under one major group and each
** application holds a list of the groups it belongs to:
**
** ALPHA LIST
** ----------
**
** group gnode -> program gnode -> group gnode - group gnode - etc.
** 		   |
** 	       program gnode -> group gnode - group gnode - group gnode - etc.
** 		   |
** 	       program gnode -> group gnode - group gnode - group gnode - etc.
** 		   |
** 	       program gnode -> group gnode - group gnode - group gnode - etc.
** 		   |
** 	       program gnode -> group gnode - group gnode - group gnode - etc.
**                    |
**                   etc.
**
**
** @alias Gnode
**
** @attr name [AjPStr] name of group or of program
** @attr doc [AjPStr] documentation for this program (used by list of programs)
** @attr progs [AjPList] list of programs in this group (used by groups list)
** @@
******************************************************************************/

typedef struct gnode {
  AjPStr name;
  AjPStr doc;
  AjPList progs;
} Gnode;
#define GPnode Gnode*


ajint    embGrpCompareTwoGnodes(const void * a, const void * b);
void   embGrpGetProgGroups (AjPList glist, AjPList alpha, char * const env[],
          AjBool emboss, AjBool embassy, AjBool explode, AjBool colon,
          AjBool gui);
void   embGrpGroupsListDel (AjPList *groupslist);
void   embGrpKeySearchProgs (AjPList newlist, const AjPList glist,
			     const AjPStr key);
void   embGrpKeySearchSeeAlso(AjPList newlist, AjPList *appgroups,
			      const AjPList alpha, const AjPList glist,
			      const AjPStr key);
GPnode embGrpMakeNewGnode (const AjPStr name);
GPnode embGrpMakeNewPnode (const AjPStr name, const AjPStr doc);
void   embGrpOutputGroupsList (AjPFile outfile, const AjPList groupslist,
			       AjBool showprogs, AjBool html,
			       const AjPStr link1,
			       const AjPStr link2);
void   embGrpOutputProgsList (AjPFile outfile,  const AjPList progslist,
			      AjBool html,
			      const AjPStr link1, const AjPStr link2);
void   embGrpSortGroupsList (AjPList groupslist);
void   embGrpMakeUnique(AjPList list);

#endif

#ifdef __cplusplus
}
#endif
