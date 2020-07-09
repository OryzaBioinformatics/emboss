#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajlist_h
#define ajlist_h

#include "ajdefine.h"

enum AjEListType {ajEListAny, ajEListStr};

/* @data AjPListNode *******************************************************
**
** Substructure of AjPList
**
** @@
******************************************************************************/

typedef struct AjSListNode AjOListNode, *AjPListNode;
struct AjSListNode {
	AjPListNode Next;	/* next item */
	AjPListNode Prev;       /* previous item */
	void *Item;		/* data */
};

/* @data AjPList ********************************************************
**
** List data object. Lists are simple linked lists with performance optimised
** to allow rapid extension of the beginning or end of the list.
**
** Lists can hold any data type. Special functions are available for lists
** of AjPStr values. In general, these functions are the same. Many are
** provided for ease of use to save remembering which calls need special cases.
**
** @new ajListNew Creates a new general list.
** @new ajListstrNew Creates a new AjPStr list.
** @new ajListNewArgs Create a new list, create the nodes and 
**                       add the other arguments as data.
** @new ajListstrNewArgs Create a new AjPStr list, create the nodes and
**                       add the other arguments as data.
** @new ajListCopy Copies a list to a new list.
** @new ajListstrCopy Copies an AjPStr list to a new list.
** @delete ajListFree Free the list, and free the items with a simple "free".
** @delete ajListstrFree Free the list, and free the items with ajStrDel
** @delete ajListDel Free the list but do not try to free the nodes.
**                Nodes should be freed first by ajListMap.
** @delete ajListstrDel Free the list but do not try to free the nodes.
**                use where nodes are still in use, e.g. in ajListToArray.
** @set ajListPush Add a new node at the start of a list.
** @set ajListstrPush Add a new node at the start of an AjPStr list.
** @set ajListPushApp Add a new node at the end of a list.
** @set ajListstrPushApp Add a new node at the end of an AjPStr list.
** @set ajListAppend Add a set of nodes at the end of the list
** @set ajListReverse Reverse the order of the nodes in a list
** @set ajListstrReverse Reverse the order of the nodes in an AjPStr list
** @set ajListMap Call a function for each node in a list.
** @set ajListstrMap Call a function for each node in a list.
** @set ajListToArray Create an array of the pointers to the data.
** @set ajListstrToArray Create an array of the pointers to the data.
** @set ajListPushList Merges two lists.
** @set ajListstrPushList Merges two AjPStr lists.
** @set ajListSort Sorts a list.
** @cast ajListFirst Set pointer to first node's data. Doesn't remove the node.
** @cast ajListLast Set pointer to last node's data. Doesn't remove the node.
** @cast ajListLength get the number of nodes in a linked list.
** @cast ajListstrLength get the number of nodes in an AjPStr linked list.
** @cast ajListPop Removes and returns the first node.
** @cast ajListstrPop Removes and returns the first AjPStr node.
** @use ajListFind For each node in the list call a function
**                 and return ajTrue when found.
** @use ajListstrFind For each node in the list call a function
**                 and return ajTrue when found.
** @use ajListIter Creates a list iterator.
** @set ajListIterDone Tests whether an iterator is finished.
** @use ajListIterFree Deletes a list iterator.
** @set ajListIterMore Tests whether iterator can return another item.
** @set ajListIterNext Returns next item using iterator, or steps off the end.
** @set ajListRemove Removes an item at the current iterator.
** @set ajListstrRemove Removes an AjPStr item at the current iterator.
** @set ajListInsert Inserts an item at the current iterator.
** @set ajListstrInsert Inserts an AjPStr item at the current iterator.
** @use ajListTrace Traces through a list and validates it
** @use ajListstrTrace Traces through an AjPStr list and validates it
** @use ajListIterTrace Traces a list iterator.
** @use ajListstrIterTrace Traces an AjPStr list iterator.
** @@
******************************************************************************/

typedef struct AjSList {
  AjPListNode First;	/* first node */
  AjPListNode Last;	/* dummy last node */
  ajint Count;
  AjEnum Type;
} AjOList, *AjPList;

typedef struct AjSIList {
  AjPList Head ;
  AjPListNode Here;
  AjPListNode Orig;
  AjBool Dir;
} *AjIList;




AjPList ajListNew (void);          /* return header */
AjPList ajListstrNew (void);          /* return header */

void ajListFree   (AjPList *list);
void ajListstrFree   (AjPList *list);

void ajListDel   (AjPList *list);
void ajListstrDel   (AjPList *list);

void ajListPush   (AjPList list, void* x);      /* " " */
void ajListstrPush (AjPList list, AjPStr x);

void ajListPushApp(AjPList list, void* x);
void ajListstrPushApp(AjPList list, AjPStr x);

AjBool ajListFirst(AjPList thys, void** x);
AjBool ajListLast(AjPList thys, void** x);
AjBool ajListNth(AjPList thys, ajint n, void** x);

AjPList ajListCopy   (AjPList list);      /* new list returned */
AjPList ajListstrCopy   (AjPList list);

void ajListTrace   (AjPList list);
void ajListstrTrace   (AjPList list);

AjBool ajListPop    (AjPList list, void** x);
AjBool ajListstrPop    (AjPList list, AjPStr* x);
AjBool ajListPopEnd(AjPList thys, void** x);
AjBool ajListstrPopEnd(AjPList thys, AjPStr *x);

ajint ajListToArray (AjPList list, void*** array);
ajint ajListstrToArray (AjPList list, AjPStr** array);

void ajListReverse(AjPList list);
void ajListstrReverse(AjPList list);

AjPList ajListNewArgs   (void* x, ...);  /* new header returned */
AjPList ajListstrNewArgs   (AjPStr x, ...);  /* new header returned */

ajint ajListLength (AjPList list);
ajint ajListstrLength (AjPList list);

AjPListNode ajListNodesNew (void *x, ...);  /* same as NewArgs but no header */

void ajListMap    (AjPList list,
		   void apply(void **x, void *cl), void *cl);
void ajListstrMap(AjPList thys,
		  void apply(AjPStr* x, void* cl), void* cl);

AjIList ajListIter (AjPList listhead);
void ajListIterFree (AjIList iter);
void* ajListIterNext (AjIList iter);
AjBool ajListIterMore (AjIList iter);
AjBool ajListIterDone (AjIList iter);
void ajListIterTrace (AjIList iter);
void ajListstrIterTrace (AjIList iter);

AjIList ajListIterBack (AjPList thys);
AjBool  ajListIterBackDone (AjIList iter);
AjBool  ajListIterBackMore (AjIList iter);
void*   ajListIterBackNext (AjIList iter);


void ajListAppend (AjPList list, AjPListNode tail);

AjBool ajListFind(AjPList listhead,
	       AjBool apply(void **x, void *cl), void *cl);
AjBool ajListstrFind(AjPList listhead,
	       AjBool apply(AjPStr* x, void *cl), void *cl);

void ajListPushList (AjPList list, AjPList* pmore);
void ajListstrPushList (AjPList list, AjPList* pmore);
void ajListInsert (AjIList iter, void* x);
void ajListInsertOld (AjIList iter, void* x);
void ajListRemove (AjIList iter);
void ajListstrInsert (AjIList iter, AjPStr x);
void ajListstrRemove (AjIList iter);
void ajListSort (AjPList thys, int (*compar) (const void*, const void*));

#endif

#ifdef __cplusplus
}
#endif
