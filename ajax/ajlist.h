#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajlist_h
#define ajlist_h

#include "ajdefine.h"

enum AjEListType {ajEListAny, ajEListStr};

/* @data AjPListNode **********************************************************
**
** Substructure of AjPList
**
** @attr Next [struct AjSListNode*] next item
** @attr Prev [struct AjSListNode*] previous item
** @attr Item [void*] data value
** @@
******************************************************************************/

typedef struct AjSListNode {
	struct AjSListNode* Next;
	struct AjSListNode* Prev;
	void *Item;
} AjOListNode;

#define AjPListNode AjOListNode*

/* @data AjPList **************************************************************
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
** @modify ajListPush Add a new node at the start of a list.
** @modify ajListstrPush Add a new node at the start of an AjPStr list.
** @modify ajListPushApp Add a new node at the end of a list.
** @modify ajListstrPushApp Add a new node at the end of an AjPStr list.
** @modify ajListAppend Add a set of nodes at the end of the list
** @modify ajListReverse Reverse the order of the nodes in a list
** @modify ajListstrReverse Reverse the order of the nodes in an AjPStr list
** @modify ajListMap Call a function for each node in a list.
** @modify ajListstrMap Call a function for each node in a list.
** @cast   ajListToArray Create an array of the pointers to the data.
** @cast   ajListstrToArray Create an array of the pointers to the data.
** @modify ajListPushList Merges two lists.
** @modify ajListstrPushList Merges two AjPStr lists.
** @modify ajListSort Sorts a list.
** @cast ajListFirst Set pointer to first node's data. Doesn't remove the node.
** @cast ajListLast Set pointer to last node's data. Doesn't remove the node.
** @cast ajListLength get the number of nodes in a linked list.
** @cast ajListstrLength get the number of nodes in an AjPStr linked list.
** @cast ajListPeek Returns the first node but keeps it on the list
** @cast ajListstrPeek Returns the first node but keeps it on the list
** @modify ajListPop Removes and returns the first node.
** @modify ajListstrPop Removes and returns the first AjPStr node.
** @use ajListFind For each node in the list call a function
**                 and return ajTrue when found.
** @use ajListstrFind For each node in the list call a function
**                 and return ajTrue when found.
** @output ajListTrace Traces through a list and validates it
** @output ajListstrTrace Traces through an AjPStr list and validates it
**
** @attr First [AjPListNode] first node
** @attr Last [AjPListNode] dummy last node
** @attr Count [ajint] Number of nodes
** @attr Type [AjEnum] List type (any, string, etc.)
** @@
******************************************************************************/

typedef struct AjSList {
  AjPListNode First;
  AjPListNode Last;
  ajint Count;
  AjEnum Type;
} AjOList;

#define AjPList AjOList*

/* @data AjIList **************************************************************
**
** AJAX list iterator data structure
**
**
** @attr Head [AjPList] Head of list
** @attr Here [AjPListNode] Current list node
** @attr Orig [AjPListNode] First list node
** @attr Dir [AjBool] Direction of last iterative move
** @attr Modify [AjBool] Allows iterator to modify the sequence
**
** @new ajListIter Default constructor
** @new ajListIterRead Default constructor for a read-only list iterator
** @cast ajListIterDone Tests whether an iterator is finished.
** @delete ajListIterFree Deletes a list iterator.
** @cast ajListIterMore Tests whether iterator can return another item.
** @modify ajListIterNext Returns next item using iterator, or steps off the end.
** @modify ajListRemove Removes an item at the current iterator.
** @modify ajListstrRemove Removes an AjPStr item at the current iterator.
** @modify ajListInsert Inserts an item at the current iterator.
** @modify ajListstrInsert Inserts an AjPStr item at the current iterator.
** @output ajListIterTrace Traces a list iterator.
** @output ajListstrIterTrace Traces an AjPStr list iterator.
** @@
******************************************************************************/

typedef struct AjSIList {
  AjPList Head ;
  AjPListNode Here;
  AjPListNode Orig;
  AjBool Dir;
  AjBool Modify;
} AjOIList;

#define AjIList AjOIList*


void        ajListAppend (AjPList list, AjPListNode* tail);
AjPList     ajListCopy   (const AjPList list);      /* new list returned */
void        ajListDel   (AjPList *list);
void        ajListExit (void);
AjBool      ajListFirst (const AjPList thys, void** x);
void        ajListFree   (AjPList *list);
void        ajListGarbageCollect (AjPList list,
				  void (*destruct)(const void **),
				  AjBool (*compar)(const void *));
void        ajListInsert (AjIList iter, void* x);

AjIList     ajListIter (AjPList list);
AjIList     ajListIterBack (AjPList list);
AjIList     ajListIterBackRead (const AjPList list);
AjIList     ajListIterRead (const AjPList list);
AjBool      ajListIterBackDone (const AjIList iter);
AjBool      ajListIterBackMore (const AjIList iter);
void*       ajListIterBackNext (AjIList iter);
void        ajListIterFree (AjIList* iter);
void*       ajListIterNext (AjIList iter);
AjBool      ajListIterMore (const AjIList iter);
AjBool      ajListIterDone (const AjIList iter);
void        ajListIterTrace (const AjIList iter);

AjBool      ajListLast (const AjPList thys, void** x);
ajint       ajListLength (const AjPList list);
void        ajListMap    (AjPList list,
			  void apply(void **x, void *cl), void *cl);
void        ajListMapRead(const AjPList list,
			  void apply(void *x, void *cl), void *cl);
AjPList     ajListNew (void);          /* return header */
AjPList     ajListNewArgs   (void* x, ...);  /* new header returned */
AjPListNode ajListNodesNew (void *x, ...);  /* same as NewArgs but no header */
AjBool      ajListNth (const AjPList thys, ajint n, void** x);
AjBool      ajListPeek    (const AjPList list, void** x);
AjBool      ajListPop    (AjPList list, void** x);
AjBool      ajListPopEnd (AjPList thys, void** x);
void        ajListPush   (AjPList list, void* x);      /* " " */
void        ajListPushApp(AjPList list, void* x);
void        ajListReverse(AjPList list);

AjPList     ajListstrCopy   (const AjPList list);
void        ajListstrDel   (AjPList *list);
void        ajListstrFree   (AjPList *list);
void        ajListstrIterTrace (const AjIList iter);
ajint       ajListstrLength (const AjPList list);
void        ajListstrMap (AjPList thys,
			  void apply(AjPStr* x, void* cl), void* cl);
void        ajListstrMapRead (const AjPList thys,
			      void apply(AjPStr x, void* cl), void* cl);
AjPList     ajListstrNew (void);          /* return header */
AjPList     ajListstrNewArgs  (AjPStr x, ...);  /* new header returned */
AjBool      ajListstrPeek    (const AjPList list, AjPStr* x);
AjBool      ajListstrPop    (AjPList list, AjPStr* x);
AjBool      ajListstrPopEnd (AjPList thys, AjPStr *x);
void        ajListstrPush (AjPList list, AjPStr x);
void        ajListstrPushApp (AjPList list, AjPStr x);
void        ajListstrReverse (AjPList list);
ajint       ajListstrToArray (const AjPList list, AjPStr** array);
ajint       ajListstrToArrayApp (const AjPList list, AjPStr** array);
void        ajListstrTrace   (const AjPList list);

ajint       ajListToArray (const AjPList list, void*** array);
void        ajListTrace   (const AjPList list);

AjBool      ajListFind (const AjPList listhead,
			AjBool apply(void **x, void *cl), void *cl);
AjBool      ajListstrFind (const AjPList listhead,
			   AjBool apply(AjPStr* x, void *cl), void *cl);

void        ajListPushList (AjPList list, AjPList* pmore);
void        ajListstrPushList (AjPList list, AjPList* pmore);
void        ajListRemove (AjIList iter);
ajint       ajListstrClone (const AjPList thys, AjPList newlist);
void        ajListstrInsert (AjIList iter, AjPStr x);
void        ajListstrRemove (AjIList iter);
void        ajListSort (AjPList thys,
			int (*compar) (const void*, const void*));
void        ajListSort2 (AjPList thys,
			 int (*sort1) (const void*, const void*),
			 int (*sort2) (const void*, const void*));
void        ajListSort3 (AjPList thys,
			 int (*sort1) (const void*, const void*),
			 int (*sort2) (const void*, const void*),
			 int (*sort3) (const void*, const void*));

void        ajListUnique (AjPList thys,
			  int (*compar) (const void* x, const void* cl),
			  void nodedelete (void** x, void* cl));
#endif

#ifdef __cplusplus
}
#endif
