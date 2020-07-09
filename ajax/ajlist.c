/********************************************************************
** @source AJAX list functions
** These functions create and control linked lists.
**
** @author Copyright (C) 1998 Ian Longden
** @version 1.0
** @author Copyright (C) 2001 Alan Bleasby
** @version 2.0 Changed lists to be double-linked, completely rewrote
**              iterator handling and added back-iteration functions.
**              Operation of ajListInsert made more intuitive.
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
********************************************************************/

/*Library* List Library ********************************************
**
** All lists consist of an initial header followed by the body
** of the list. The Header has three variables:-
** 1) First - a pointer to the linked list (see body)
** 2) Last -  a pointer to the a dummy last node object with next = self
** 3) Count - which holds the number of objects in the linked list 
**           (NOT including the header)
** 4) Type - the list type
**
** The body of the linked list contains three variables:-
** 1) next - a pointer to the next linked list object or NULL
** 2) prev - a pointer to the previous linked list object or NULL
** 3) item - a void pointer to the data.
*******************************************************************/
 
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ajstr.h"
#include "ajassert.h"
#include "ajmem.h"
#include "ajlist.h"
#include "ajmess.h"

#define ajLASTFWD  0	/* For iteration shows direction of last walk */
#define ajLASTBACK 1

static AjPList listNew(AjEnum type);
static void listInsertNode (AjPListNode* pnode, void* x);
static AjPListNode listDummyNode (AjPListNode* pnode);
static void listNodesTrace (AjPListNode node);
static AjBool listNodeDel (AjPListNode* pnode);
static void* listNodeItem (AjPListNode node);
static void listArrayTrace (void** array);

/* @func ajListNew *********************************************
**
** Creates a new general list.
**
** @return [AjPList] new list;
** @@
****************************************************************/

AjPList ajListNew(void)
{
    return listNew(ajEListAny);
}

/* @func ajListstrNew *********************************************
**
** Creates a new string list.
**
** @return [AjPList] new list;
** @@
****************************************************************/

AjPList ajListstrNew(void)
{
    return listNew(ajEListStr);
}

/* @funcstatic listNew *********************************************
**
** Creates a new list.
**
** @param [r] type [AjEnum] Defined list type.
** @return [AjPList] new list;
** @@
****************************************************************/

static AjPList listNew(AjEnum type)
{
    AjPList list;

    AJNEW0 (list);
    list->Type = type;

    list->Last = listDummyNode (&list->First);

    return list;
}

/* @func ajListPush ********************************************
**
** Add a new node at the start of the list and add the
** data pointer.
**
** @param [u] thys [AjPList] list to be changed.
** @param [P] x [void*] Pointer to data.
** @return [void]
** @@
*****************************************************************/  

void ajListPush (AjPList thys, void* x)
{
    assert (thys);

    listInsertNode (&thys->First, x);

    if(!thys->Count++)
	thys->Last->Prev = thys->First;

    return;
}

/* @func ajListstrPush ********************************************
**
** Add a new node at the start of a string list.
**
** @param [u] thys [AjPList] list to be changed.
** @param [P] x [AjPStr] String data.
** @return [void]
** @@
*****************************************************************/  

void ajListstrPush (AjPList thys, AjPStr x)
{
    ajListPush (thys, (void*) x);

    return;
}

/* @func ajListTrace ********************************************
**
** Traces through a list and validates it
**
** @param [r] thys [AjPList] list to be traced.
** @return [void]
** @@
*****************************************************************/  

void ajListTrace (AjPList thys)
{
    ajint i = 0;
    AjPListNode node;

    if (!thys) return;

    ajDebug ("\nList Trace %x type %d count %d\n",
	     thys, thys->Type, thys->Count);
    ajDebug("first-> %x last-> %x\n", thys->First, thys->Last);

    for (node=thys->First; node->Next; node=node->Next) {
	i++;
	ajDebug ("Item[%d] item %x (data %x) rest -> %x prev -> %x\n",
		 i, node, node->Item, node->Next, node->Prev);
    }

    if (i != thys->Count)
    {
	ajDebug("*** list error expect %d items, found %d\n",
		thys->Count, i);
	ajErr("*** list error expect %d items, found %d",
	      thys->Count, i);
    }

    if (thys->Last != node)
    {
	ajDebug("*** list error expect end at %x, found at %x\n",
		thys->Last, node);
	ajErr("*** list error expect end at %x, found at %x",
	      thys->Last, node);
    }

    return;
}

/* @func ajListstrTrace ********************************************
**
** Traces through a string list and validates it
**
** @param [r] thys [AjPList] list to be traced.
** @return [void]
** @@
*****************************************************************/  

void ajListstrTrace (AjPList thys)
{

    ajint i = 0;
    AjPListNode node;

    if (!thys)
	return;

    ajDebug ("\nList Trace %x type %d count %d\n",
	     thys, thys->Type, thys->Count);
    ajDebug("rest-> %x last-> %x\n",
	    thys->First, thys->Last);

    for (node=thys->First; node->Next; node=node->Next)
    {
	i++;
	ajDebug ("Item[%d] item %x '%S' rest -> %x prev -> %x\n",
		 i, node, (AjPStr) node->Item, node->Next, node->Prev);
    }

    if (i != thys->Count)
    {
	ajDebug("*** list error expect %d items, found %d\n",
		thys->Count, i);
	ajErr("*** list error expect %d items, found %d",
	      thys->Count, i);
    }

    if (thys->Last != node)
    {
	ajDebug("*** list error expect end at %x, found at %x\n",
		thys->Last, node);
	ajErr("*** list error expect end at %x, found at %x",
	      thys->Last, node);
    }

    return;
}

/* @func ajListNewArgs ********************************************
**
** Create a new list, create the nodes and add the data.
**
** @param [r] x [void*] First data item
** @param [v] [...] Variable length argument list
** @return [AjPList] new linked list.
** @@
****************************************************************/

AjPList ajListNewArgs (void* x, ...)
{
    AjPList list;
    va_list ap;
    ajint i=0;
    void* y;

    list = ajListNew();

    if (!x)
	return list;

    va_start(ap, x);
    y = x;

    for ( i=0; y; y = va_arg(ap, void*),i++)
	ajListPushApp(list, y);

    va_end(ap);

    return list;
}

/* @func ajListstrNewArgs ********************************************
**
** Create a new list, create the nodes and add the data.
**
** @param [r] x [AjPStr] First string
** @param [v] [...] Variable length argument list
** @return [AjPList] new linked list.
**
** @@
****************************************************************/

AjPList ajListstrNewArgs (AjPStr x, ...)
{
    AjPList list;
    va_list ap;
    ajint i=0;
    AjPStr y;

    list = ajListstrNew();

    if (!x)
	return list;

    va_start(ap, x);
    y = x;

    for ( i=0; y; y = va_arg(ap, AjPStr),i++)
	ajListstrPushApp(list, y);

    va_end(ap);

    return list;
}

/* @func ajListNodesNew *******************************************
**
** Create new nodes (NO header) and add data.
**
** @param [r] x [void*] First data item.
** @param [v] [...] Variable length argument list
** @return [AjPListNode] new list (body).
** @@
********************************************************************/

AjPListNode ajListNodesNew (void* x, ...)
{
    va_list ap;
    AjPListNode topnode, node;
    va_start(ap, x);

    topnode = listDummyNode (&node);
    
    ajDebug("ajListNodesNew topnode: %x -> %x\n", topnode, topnode->Next);
    for ( ; x; x = va_arg(ap, void *))
    {
	node->Item = x;
	(void) listDummyNode(&node->Next);
	node->Next->Prev = node;
	ajDebug ("topnode: %x node: %x, item: %x -> %x\n",
		 topnode, node, x, node->Next);
	/* node = node->Next; ??AJB comment: shouldn't this be here?? */
    }
    va_end(ap);

    topnode->Prev = NULL;
  
    listNodesTrace(node);

    return node;
}

/* @funcstatic listNodesTrace *************************************************
**
** Writes debug messages to trace from the current list node.
**
** @param [r] node [AjPListNode] Current node.
** @return [void]
** @@
******************************************************************************/

static void listNodesTrace (AjPListNode node)
{
    AjPListNode p = node;

    ajDebug("listNodesTrace %x\n", p);

    while (p->Next)
    {
	ajDebug ("node %x item %x -> %x\n", p, p->Item, p->Next);
	p = p->Next;
    }
    return;
}

/* @func ajListAppend ************************************************
**
** Add a new node at the end of the list and add the
** data pointer.
**
** @param [P] thys [AjPList]  list to be changed.
** @param [r] morenodes [AjPListNode] link list to append.
** @return [void]
** @@
*********************************************************************/  

void ajListAppend(AjPList thys, AjPListNode morenodes)
{
    AjPListNode more = morenodes;
        
    listNodesTrace(morenodes);

    more->Next->Prev = thys->Last;
    thys->Last->Next = more->Next;
    thys->Last->Item = more->Item;

    while(more->Next)
    {				/* need to get to the end of the list */
	more = more->Next;
	thys->Count++;
    }

    thys->Last = more;		/* now we can set the end of the list */
    AJFREE (morenodes);		/* first extra node (only) was duplicated */

    return;
}

/* @func ajListPushApp ************************************************
**
** Add a new node at the end of the list and add the
** data pointer.
**
** @param [P] thys [AjPList] List to be changed.
** @param [P] x [void*] Pointer to data to append.
** @return [void]
** @@
*********************************************************************/  

void ajListPushApp(AjPList thys, void* x)
{
    /* cannot use listInsertNode because that needs a pointer to the
       penultimate node, so we use the dummy node and make a new dummy node
       instead */
    AjPListNode tmp=NULL;

    if(!thys->Count)
    {
	ajListPush(thys,x);
	return;
    }
    
    thys->Last->Item = x;

    tmp = thys->Last;
    thys->Last = listDummyNode(&thys->Last->Next);
    thys->Last->Prev = tmp;
    
    thys->Count++;

    return;
}

/* @func ajListstrPushApp ************************************************
**
** Add a new node at the end of the list and add the
** data pointer.
**
** @param [P] thys [AjPList] List to be changed.
** @param [P] x [AjPStr] String to append.
** @return [void]
** @@
*********************************************************************/  

void ajListstrPushApp(AjPList thys, AjPStr x)
{
    ajListPushApp (thys, (void*) x);
}

/* @func ajListstrCopy ************************************************
**
** Copy a string list.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] thys [AjPList] List to be copied
** @return [AjPList] New, copied, list.
** @@
*******************************************************************/

AjPList ajListstrCopy(AjPList thys)
{
    return ajListCopy (thys);
}

/* @func ajListCopy ************************************************
**
** Copy a list.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] thys [AjPList] list to be copied
** @return [AjPList] new copied list.
** @@
*******************************************************************/

AjPList ajListCopy(AjPList thys)
{
    AjPList newlist;

    AjPListNode node;

    if(!thys)
	return NULL;

    newlist = ajListNew();
    newlist->Type = thys->Type;

    for ( node=thys->First; node->Next; node=node->Next)
	ajListPushApp (newlist, node->Item);


    return newlist;
}

/* @func ajListFirst **********************************************
**
** Set pointer to first node's data. Does NOT remove the first node.
**
** @param [u] thys [AjPList] List
** @param [P] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
****************************************************************/

AjBool ajListFirst(AjPList thys, void** x)
{

    if (!thys)
	return ajFalse;

    if (x)
	*x = listNodeItem(thys->First);

    return ajTrue;
}

/* @func ajListLast **********************************************
**
** Set pointer to last node's data. Does NOT remove the last node.
**
** @param [u] thys [AjPList] List
** @param [P] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
****************************************************************/

AjBool ajListLast(AjPList thys, void** x)
{
    AjPListNode rest;

    if (!thys)
	return ajFalse;

    for (rest = thys->First; rest->Next; rest = rest->Next)
	if(!rest->Next->Next)
	    break;

    if (x)
	*x = listNodeItem(rest);

    return ajTrue;
}

/* @func ajListNth **********************************************
**
** Set pointer to last node's nth data item. 0 <= n < number of elements. 
**
** @param [r] thys [AjPList] List
** @param [r] n [AjPList] element of the list
** @param [w] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
****************************************************************/

AjBool ajListNth(AjPList thys, ajint n, void** x)
{
    AjPListNode rest;
    ajint len;
    ajint i;
  
    if (!thys || n<1)
	return ajFalse;

    len = ajListLength(thys);
    if(n>len)
	return ajFalse;
  
    for (i=0,rest = thys->First; i<n ; rest = rest->Next);

    if (x)
	*x = listNodeItem(rest);

    return ajTrue;
}

/* @func ajListPop **********************************************
**
** remove the first node but set pointer to data first.
**
** @param [u] thys [AjPList] List
** @param [P] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
****************************************************************/

AjBool ajListPop(AjPList thys, void** x)
{

    if (!thys)
	return ajFalse;

    if (x)
	*x = listNodeItem(thys->First);

    if (!listNodeDel(&thys->First))
	return ajFalse;

    thys->First->Prev = NULL;

    thys->Count--;
    return ajTrue;
}

/* @funcstatic listNodeDel **********************************************
**
** Remove a first node from the list.
**
** @param [u] pnode  [AjPListNode*] Current node.
** @return [AjBool] ajTrue on success.
** @@
****************************************************************/

static AjBool listNodeDel (AjPListNode* pnode)
{
    AjPListNode node = *pnode;
    AjPListNode tmp;
  
    if (!node || !node->Next)
	return ajFalse;

    tmp = node->Prev;
    node = node->Next;
    node->Prev = tmp;
    AJFREE (*pnode);
    *pnode = node;

    return ajTrue;
}

/* @funcstatic listNodeItem **********************************************
**
** Return the data item for a list node.
**
** @param [r] node  [AjPListNode] Current node.
** @return [void*] Data item.
** @@
****************************************************************/

static void* listNodeItem (AjPListNode node)
{

    if (!node || !node->Next)
	return NULL;

    return node->Item;
}


/* @func ajListstrPop **********************************************
**
** Remove the first node but set pointer to data first.
**
** @param [u] thys [AjPList] List
** @param [P] x [AjPStr*] String
** @return [AjBool] ajTrue on success.
** @@
****************************************************************/

AjBool ajListstrPop (AjPList thys, AjPStr* x)
{

    if (!thys)
	return ajFalse;

    if (x)
	*x = (AjPStr) listNodeItem(thys->First);

    if (!listNodeDel(&thys->First))
	return ajFalse;

    thys->First->Prev = NULL;

    thys->Count--;
    return ajTrue;
}

/* @func ajListReverse ****************************************
**
** Reverse the order of the nodes in an abstract list.
**
** @param [u] thys [AjPList] List
** @return [void]
** @@
***************************************************************/

void ajListReverse(AjPList thys)
{
    AjPListNode head = thys->Last;
    AjPListNode savenext;
    AjPListNode node;

    if (thys->Count <= 1)
	return;

    thys->Last->Prev = thys->First;

    for ( node = thys->First; node->Next; node = savenext)
    {
	savenext = node->Next;
	node->Prev = node->Next;
	node->Next = head;
	head = node;
    }
    thys->First = head;

    thys->First->Prev = NULL;
  
    return;
}

/* @func ajListstrReverse ****************************************
**
** Reverse the order of the nodes in a string list.
**
** @param [u] thys [AjPList] List
** @return [void]
** @@
***************************************************************/

void ajListstrReverse(AjPList thys)
{
    ajListReverse (thys);
}

/* @func ajListLength ****************************************
**
** get the number of nodes in the linked list.
**
** @param [r] thys [AjPList] List
** @return [ajint] Number of nodes in list.
** @@
***************************************************************/

ajint ajListLength(AjPList thys)
{
    if (!thys)
	return 0;

    return thys->Count;
}

/* @func ajListstrLength ****************************************
**
** get the number of nodes in the linked list.
**
** @param [r] thys [AjPList] List
** @return [ajint] Number of nodes in list.
** @@
***************************************************************/

ajint ajListstrLength(AjPList thys)
{
    return ajListLength(thys);
}


/* @func ajListFree *****************************************
**
** Free all nodes in the list.
** NOTE: The data is only freed with a specified list type.
**       For undefined data types we recommend you to
**       use ajListMap with a routine to free the memory.
**
** @param [u] pthis [AjPList*] List
** @return [void]
** @@
**************************************************************/

void ajListFree(AjPList* pthis)
{
    AjPListNode next;
    AjPListNode *rest;
    AjPList thys;

    if (!pthis)
	return;

    if (!*pthis)
	return;

    thys = *pthis;
    rest = &thys->First;

    if (!thys->Count)
    {
	AJFREE(thys->Last);
	AJFREE(*pthis);    
	return;
    }

    /* don't free the data in the list (we don't know how) */
    /* just free the nodes */

    for ( ; (*rest)->Next; *rest = next)
    {
	next = (*rest)->Next;
	AJFREE(*rest);
    }

    AJFREE(*rest);
    AJFREE(*pthis);

    return;
}

/* @func ajListstrFree *****************************************
**
** Free all nodes in a string list.
** Also deletes all the strings. If these are to be preserved,
** use ajListstrDel instead.
**
** @param [u] pthis [AjPList*] List
** @return [void]
** @@
**************************************************************/

void ajListstrFree(AjPList* pthis)
{
    AjPListNode next;
    AjPListNode *rest;
    AjPList thys;

    if (!pthis)
	return;

    if (!*pthis)
	return;

    thys = *pthis;
    rest = &thys->First;

    if (thys->Count)
    {
	/* free the data in the list (if we know how) */
	for ( ; (*rest)->Next; *rest = next)
	{
	    next = (*rest)->Next;
	    ajStrDel ((AjPStr*) &(*rest)->Item);
	    AJFREE(*rest);
	}
    }

    AJFREE(*rest);
    AJFREE(*pthis);

    return;
}

/* @func ajListDel *****************************************
**
** Free the list. Do not attempt to free the nodes.
** For use where the node data has been saved elsewhere, for example
** by ajListToArray or where the list is a temporary structure
** referring to permanent data.
**
** @param [u] pthis [AjPList*] List
** @return [void]
** @@
**************************************************************/

void ajListDel(AjPList* pthis)
{
    AjPList list;

    if (!pthis)
	return;
    if (!*pthis)
	return;

    list = *pthis;
    if (!list->Count)
	if (list->Last == list->First)
	    AJFREE (list->First);

    AJFREE(*pthis);

    return;
}

/* @func ajListstrDel *****************************************
**
** Free the list. Do not attempt to free the nodes.
** For use where the node data has been saved elsewhere, for example
** by ajListToArray or where the list is a temporary structure
** referring to permanent data.
**
** @param [u] pthis [AjPList*] List
** @return [void]
** @@
**************************************************************/

void ajListstrDel(AjPList* pthis)
{
    ajListDel (pthis);

    return;
}

/* @func ajListMap ************************************************************
**
** For each node in the list call function apply.
**
** @param [u] thys [AjPList] List.
** @param [f] apply [void function] Function to call for each list item.
** @param [P] cl [void*] Standard, usually NULL.
** @return [void]
** @@
******************************************************************************/

void ajListMap (AjPList thys, void apply(void** x, void* cl), void* cl)
{
    AjPListNode rest;
    assert(apply);

    for (rest = thys->First; rest->Next; rest = rest->Next)
	apply((void**) &rest->Item, cl);

    return;
}

/* @func ajListstrMap *********************************************************
**
** For each node in the list call function apply,
** with the address of the string and a client pointer.
**
** @param [u] thys [AjPList] List.
** @param [f] apply [void function] Function to call for each list item.
** @param [P] cl [void*] Standard, usually NULL.
** @return [void]
** @@
******************************************************************************/

void ajListstrMap(AjPList thys, void apply(AjPStr* x, void* cl), void* cl)
{
    AjPListNode rest;
    assert(apply);

    for (rest=thys->First; rest->Next; rest = rest->Next)
	apply((AjPStr*) &rest->Item, cl);


    return;
}

/* @func ajListToArray ********************************************************
**
** Create an array of the pointers to the data.
**
** @param [r] thys [AjPList] List
** @param [P] array [void***] Array of pointers to list items.
** @return [ajint] Size of array of pointers.
** @@
******************************************************************************/

ajint ajListToArray(AjPList thys, void*** array)
{
    ajint i;
    ajint n = thys->Count;
    AjPListNode rest = thys->First;

    if (!n)
    {
	*array = NULL;
	return 0;
    }

    *array = AJALLOC((n+1)*sizeof(array));
    for (i = 0; i < n; i++)
    {
	(*array)[i] = rest->Item;
	rest = rest->Next;
    }
    (*array)[n] = 0;

    return n;
}

/* @func ajListstrToArray *****************************************************
**
** create an array of the pointers to the data.
**
** @param [r] thys [AjPList] List
** @param [P] array [AjPStr**] Array of Stringss.
**
** @return [ajint] Size of array of pointers.
**
** @@
******************************************************************************/

ajint ajListstrToArray(AjPList thys, AjPStr** array)
{
    ajint i;
    ajint n = thys->Count;
    AjPListNode rest = thys->First;

    if (!n)
    {
	*array = NULL;
	return 0;
    }

    *array = AJALLOC((n)*sizeof (array));

    for (i = 0; i < n; i++)
    {
	(*array)[i] = (AjPStr) rest->Item;
	rest = rest->Next;
    }

    return n;
}

/* @func ajListFind *********************************************
**
** For each node in the list call function 'apply' and return
** ajTrue when any node is matched by the function.
**
** @param [r] thys [AjPList] List
** @param [f] apply [AjBool function] Function to call to test each list item.
** @param [P] cl [void*] Standard, usually NULL.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************/

AjBool ajListFind(AjPList thys, AjBool apply(void** x, void* cl), void* cl)
{
    AjPListNode list;

    assert(apply);

    for ( list = thys->First; list->Next; list = list->Next)
	if(apply(&list->Item, cl))
	    return ajTrue;
    return ajFalse;
}

/* @func ajListstrFind *********************************************
**
** For each node in the list call function apply and return
** ajTrue when any node is matched by the function.
**
** @param [r] thys [AjPList] List
** @param [f] apply [AjBool function] Function to call to test each list item.
** @param [P] cl [void*] Standard, usually NULL.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************/

AjBool ajListstrFind (AjPList thys, AjBool apply(AjPStr* x, void* cl),
		      void* cl)
{
    AjPListNode list;

    assert(apply);

    for ( list = thys->First; list->Next; list = list->Next)
	if(apply((AjPStr*) &list->Item, cl))
	    return ajTrue;
    return ajFalse;
}

/* @func ajListIter *******************************************************
**
** Creates an iterator to operate from start to end of list.
**
** @param [r] thys [AjPList] List
** @return [AjIList] New list iterator
** @@
******************************************************************************/

AjIList ajListIter (AjPList thys)
{
    AjIList iter;

    if (!thys)
	return NULL;

    AJNEW0(iter);
    iter->Head = thys;
    iter->Dir = ajLASTFWD;
    iter->Here = thys->First;
    iter->Orig = thys->First;
    
    return iter;
}

/* @func ajListIterBack ****************************************************
**
** Creates an iterator to operate from end to start of the list.
**
** @param [r] thys [AjPList] List
** @return [AjIList] New list iterator
** @@
******************************************************************************/

AjIList ajListIterBack (AjPList thys)
{
    AjIList iter;
    AjPListNode node = NULL;
    AjPListNode tmp  = NULL;
    
    if (!thys)
	return NULL;

    if(!thys->Count)
	return NULL;

    for(node=thys->First; node->Next; node = node->Next)
	tmp = node;
    thys->Last->Prev = tmp;

    AJNEW0(iter);
    iter->Head = thys;
    iter->Dir = ajLASTBACK;
    iter->Here = tmp->Next;

    return iter;
}

/* @func ajListIterDone *******************************************************
**
** Tests whether an iterator has completed yet.
**
** @param [r] iter [AjIList] List iterator.
** @return [AjBool] ajTrue if the iterator is exhausted.
** @@
******************************************************************************/

AjBool ajListIterDone (AjIList iter)
{
    if (ajListIterMore(iter))
	return ajFalse;

    return ajTrue;
}

/* @func ajListIterBackDone **************************************************
**
** Tests whether a backwards iterator has completed yet.
**
** @param [r] iter [AjIList] List iterator.
** @return [AjBool] ajTrue if the iterator is exhausted.
** @@
******************************************************************************/

AjBool ajListIterBackDone (AjIList iter)
{
    if (ajListIterBackMore(iter))
	return ajFalse;

    return ajTrue;
}

/* @func ajListIterFree *******************************************************
**
** Destructor for a list iterator.
**
** @param [w] iter [AjIList] List iterator.
** @return [void]
** @@
******************************************************************************/

void ajListIterFree (AjIList iter)
{
    AJFREE(iter);

    return;
}

/* @func ajListIterMore *******************************************************
**
** Tests whether ajListIterNext can return another item.
**
** @param [r] iter [AjIList] List iterator.
** @return [AjBool] ajTrue if the iterator can continue.
** @@
******************************************************************************/

AjBool ajListIterMore (AjIList iter)
{
    AjPListNode p;

    if (!iter)
	return ajFalse;

    p = iter->Here;

    if(iter->Dir == ajLASTFWD)
    {
	if(!p->Next)
	    return ajFalse;
    }
    else
	if(!p->Next->Next || !p->Next->Next->Next)
	    return ajFalse;
	
    return ajTrue;
}


/* @func ajListIterBackMore **************************************************
**
** Tests whether ajListIterBackNext can return another item.
**
** @param [r] iter [AjIList] List iterator.
** @return [AjBool] ajTrue if the iterator can continue.
** @@
******************************************************************************/

AjBool ajListIterBackMore (AjIList iter)
{
    AjPListNode p;

    if (!iter)
	return ajFalse;

    p = iter->Here;

    if(!p->Prev)
	return ajFalse;


    return ajTrue;
}

/* @func ajListIterNext *******************************************************
**
** Returns next item using iterator, or steps off the end.
**
** @param [r] iter [AjIList] List iterator.
** @return [void*] Data item returned.
** @@
******************************************************************************/

void* ajListIterNext (AjIList iter)
{
    AjPListNode p;
    void *ret;

    if(!ajListIterMore(iter))
	return NULL;

    p = iter->Here;

    if(iter->Dir == ajLASTFWD)
    {
	ret = p->Item;
	iter->Here = p->Next;
    }
    else
    {
	iter->Dir = ajLASTFWD;
	ret = p->Next->Item;
	iter->Here = p->Next->Next;
    }

    return ret;
}

/* @func ajListIterBackNext **************************************************
**
** Returns next item using back iterator.
**
** @param [r] iter [AjIList] List iterator.
** @return [void*] Data item returned.
** @@
******************************************************************************/

void* ajListIterBackNext (AjIList iter)
{
    AjPListNode p;
    void* ret;

    if(!ajListIterBackMore(iter))
	return NULL;

    p = iter->Here;

    if(iter->Dir == ajLASTFWD)
    {
	ret = p->Prev->Prev->Item;
	iter->Here = p->Prev->Prev;
	iter->Dir = ajLASTBACK;
    }
    else
    {
	ret = p->Prev->Item;
	iter->Here = p->Prev;
    }

    return ret;
}

/* @func ajListRemove *******************************************************
**
** Remove an item from a list, using an iterator (if not null)
** to show which item. Otherwise remove the first item.
**
** We want to remove the item just fetched by the iterator.
**
** @param [r] iter [AjIList] List iterator.
** @return [void]
** @@
******************************************************************************/

void ajListRemove (AjIList iter)
{
    AjPListNode p;
    
    /* ajDebug ("ajListRemove\n");*/

    p = iter->Here;

    if(iter->Dir == ajLASTFWD)
    {
	if(!p->Prev)
	    ajFatal("Attempt to delete from unused iterator\n");

	if(!p->Prev->Prev)
	    (void) listNodeDel(&(iter->Head->First));
	else
	    (void) listNodeDel(&p->Prev->Prev->Next);
    }
    else
	(void) listNodeDel(&p->Prev->Prev->Next);
    

    iter->Head->Count--;

    return;
}

/* @func ajListstrRemove ******************************************************
**
** Remove an item from a list, using an iterator (if not null)
** to show which item. Otherwise remove the first item.
**
** We want to remove the item just fetched by the iterator.
**
** @param [r] iter [AjIList] List iterator.
** @return [void]
** @@
******************************************************************************/

void ajListstrRemove (AjIList iter)
{

    ajListRemove (iter);

    return;
}

/* @func ajListInsert *******************************************************
**
** Insert an item in a list, using an iterator (if not null)
** to show which position to insert. Otherwise, simply push.
**
** @param [r] iter [AjIList] List iterator.
** @param [r] x [void*] Data item to insert.
** @return [void]
** @@
******************************************************************************/

void ajListInsert (AjIList iter, void* x)
{
    AjPList list = iter->Head;
    AjPListNode p;
    
    /* ajDebug ("ajListInsert\n");*/


    p = iter->Here;


    if(iter->Dir == ajLASTFWD)
    {
	if(!p->Prev)
	    listInsertNode(&list->First,x);
	else
	    listInsertNode(&p->Prev->Next,x);
	iter->Here = p->Prev;
    }
    else
    {
	if(!p->Next)
	    ajFatal("Cannot add a new node for unused back iterator\n");
	
	if(!p->Prev)
	    listInsertNode(&list->First,x);
	else
	    listInsertNode(&p->Prev->Next,x);
    }

    list->Count++;

    /*ajListTrace (list);*/
    /*ajListIterTrace (iter);*/

    return;
}


/* @func ajListstrInsert ****************************************************
**
** Insert an item in a list, using an iterator (if not null)
** to show which position to insert. Otherwise, simply push.
**
** @param [r] iter [AjIList] List iterator.
** @param [r] x [AjPStr] String to insert.
** @return [void]
** @@
******************************************************************************/

void ajListstrInsert (AjIList iter, AjPStr x)
{
    AjPList list = iter->Head;
    AjPListNode p;
    
    /*ajDebug ("ajListstrInsert\n");*/
    ajListstrTrace (list);
    ajListstrIterTrace (iter);

    p = iter->Here;


    if(iter->Dir == ajLASTFWD)
    {
	if(!p->Prev)
	    listInsertNode(&list->First,x);
	else
	    listInsertNode(&p->Prev->Next,x);
	iter->Here = p->Prev;
    }
    else
    {
	if(!p->Next)
	    ajFatal("Cannot add a new node for unused back iterator\n");
	
	if(!p->Prev)
	    listInsertNode(&list->First,x);
	else
	    listInsertNode(&p->Prev->Next,x);
    }

    list->Count++;

    ajListstrTrace (list);
    ajListstrIterTrace (iter);

    return;
}

/* @funcstatic listInsertNode ************************************************
**
** Inserts a new node in a list at the current node position.
**
** @param [u] pnode [AjPListNode*] Current node.
** @param [r] x [void*] Data item to insert.
** @return [void]
** @@
******************************************************************************/

static void listInsertNode (AjPListNode* pnode, void* x)
{
    AjPListNode p;
  
    AJNEW0(p);
    p->Item = x;
    p->Next = (*pnode);
    p->Prev = (*pnode)->Prev;
    p->Next->Prev = p;
    *pnode = p;

    return;
}

/* @funcstatic listDummyNode **********************************************
**
** Creates a new empty node.
**
** @param [u] pnode [AjPListNode*] New node.
** @return [AjPListNode] Copy of current node.
** @@
******************************************************************************/

static AjPListNode listDummyNode (AjPListNode* pnode)
{
  
    AJNEW0(*pnode);

    return *pnode;
}

/* @func ajListIterTrace ********************************************
**
** Traces a list iterator and validates it.
**
** @param [r] thys [AjIList] list iterator to be traced.
** @return [void]
** @@
*****************************************************************/  

void ajListIterTrace (AjIList thys)
{
    if (!thys)
	return;

    ajDebug("\nIterator Head %x Here %x Dir %d\n",thys->Head,
	    thys->Here,thys->Dir);

    return;
}

/* @func ajListstrIterTrace ********************************************
**
** Traces a list iterator and validates it
**
** @param [r] thys [AjIList] List iterator to be traced.
** @return [void]
** @@
*****************************************************************/  

void ajListstrIterTrace (AjIList thys)
{
    if (!thys)
	return;

    ajDebug("\nIterator Head %x Here %x Dir %d Item %S\n",thys->Head,
	    thys->Here,thys->Dir,(AjPStr)thys->Here->Item);

    return;
}

/* @func ajListPushList *******************************************************
**
** Adds a list to the start of the current list, then deletes to second list.
**
** @param [r] thys [AjPList] List.
** @param [r] pmore [AjPList*] List to be merged.
** @return [void]
** @@
******************************************************************************/

void ajListPushList (AjPList thys, AjPList* pmore)
{
    AjPList more = *pmore;

    if (more->Count)
    {					/* more list has items */

	if (thys->Count)
	{				/* master list has items */
	    more->Last->Item = thys->First->Item;
	    more->Last->Next = thys->First->Next;
	    thys->First->Next->Prev = more->Last;
	}
	else
	    thys->Last = more->Last;

	AJFREE (thys->First);
	thys->First = more->First;
	thys->Count += more->Count;
	thys->First->Prev = NULL;
    }

    ajListDel (pmore);			/* free the list but not the nodes */

    return;
}

/* @func ajListstrPushList ****************************************************
**
** Adds a list to the start of the current list, then deletes to second list.
**
** @param [r] thys [AjPList] List.
** @param [r] pmore [AjPList*] List to be merged.
** @return [void]
** @@
******************************************************************************/

void ajListstrPushList (AjPList thys, AjPList* pmore)
{
    ajListPushList (thys, pmore);

    return;
}

/* @funcstatic listArrayTrace *************************************************
**
** Writes debug messages to trace an array generated from a list.
**
** @param [r] array [void**] Array to trace
** @return [void]
** @@
******************************************************************************/

static void listArrayTrace (void** array)
{
    void** v = array;
    ajint i=0;
    while (*v)
	ajDebug ("array[%d] %x\n", i++, *v++);

    return;
}

/* @func ajListSort *******************************************************
**
** Sort the items in a list.
**
** @param [r] thys [AjPList] List.
** @param [r] compar [int* function] Function to compare two list items.
** @return [void]
** @@
******************************************************************************/

void ajListSort (AjPList thys, int (*compar) (const void*, const void*))
{
    void** array = NULL;
    ajint i = 0;
    AjPListNode node = thys->First;

    /*ajDebug ("ajListSort %d items\n", thys->Count);*/
    /*ajListTrace(thys);*/

    if (thys->Count <= 1)
	return;

    (void) ajListToArray(thys, &array);
    /* listArrayTrace(array);*/

    qsort (array, thys->Count, sizeof(void*), compar);

    while (node->Next)
    {
	node->Item = array[i++];
	node = node->Next;
    }

    AJFREE (array);

    return;
}

/* Dummy function to catch all unused functions defined above */
void ajListDummyFunction(void** array)
{
    listArrayTrace(array);
}
