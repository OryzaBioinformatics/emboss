/******************************************************************************
** @source AJAX tree functions
** These functions create and control trees.
**
** @author Copyright (C) 2004 Peter Rice
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
******************************************************************************/

/*Library* Tree Library *******************************************************
**
** All trees consist of an initial header followed by the body
** of the tree. The Header has three variables:-
** 1) Nodes -  an array of top level nodes with names
** 2) Count - which holds the number of named objects in the trees
** 3) Type - the tree type
**
** The body of the tree contains three variables:-
** 1) next - a pointer to the next tree node or NULL
** 2) prev - a pointer to the higher tree node or NULL
** 3) item - a void pointer to the data.
******************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ajstr.h"
#include "ajassert.h"
#include "ajmem.h"
#include "ajtree.h"
#include "ajmess.h"

static ajint treeNewCnt     = 0;
static ajint treeDelCnt     = 0;
static ajint treeMaxNum     = 0;
static ajint treeNodeCnt    = 0;
static ajint treeIterNewCnt = 0;
static ajint treeIterDelCnt = 0;




static AjPTree treeNew(AjEnum type);
static void treeTraceNode (const AjPTree thys, ajint margin);


/* @func ajTreeNew ************************************************************
**
** Creates a new general tree.
**
** @return [AjPTree] new tree;
** @category new [AjPTree] Creates a new general tree.
** @@
******************************************************************************/

AjPTree ajTreeNew(void)
{
    return treeNew(ajETreeAny);
}




/* @func ajTreestrNew *********************************************************
**
** Creates a new string tree.
**
** @return [AjPTree] new tree;
** @category new [AjPTree] Creates a new AjPStr tree.
** @@
******************************************************************************/

AjPTree ajTreestrNew(void)
{
    return treeNew(ajETreeStr);
}




/* @funcstatic treeNew ********************************************************
**
** Creates a new tree.
**
** @param [r] type [AjEnum] Defined tree type.
** @return [AjPTree] new tree;
** @@
******************************************************************************/

static AjPTree treeNew(AjEnum type)
{
    AjPTree tree;

    AJNEW0(tree);
    tree->Type = type;

    treeNodeCnt++;			/* dummy */
    treeNewCnt++;

    return tree;
}



/* @func ajTreestrCopy ********************************************************
**
** Copy a string tree.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] thys [const AjPTree] Tree to be copied
** @return [AjPTree] New, copied, tree.
** @category new [AjPTree] Copies an AjPStr tree to a new tree.
** @@
******************************************************************************/

AjPTree ajTreestrCopy(const AjPTree thys)
{
    return ajTreeCopy(thys);
}




/* @func ajTreeCopy ***********************************************************
**
** Copy a tree.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] thys [const AjPTree] tree to be copied
** @return [AjPTree] new copied tree.
** @category new [AjPTree] Copies a tree to a new tree.
** @@
******************************************************************************/

AjPTree ajTreeCopy(const AjPTree thys)
{
    AjPTree newtree;

    if(!thys)
	return NULL;

    newtree = ajTreeNew();
    newtree->Type = thys->Type;

    return newtree;
}




/* @func ajTreeLength *********************************************************
**
** get the number of nodes in the linked tree.
**
** @param [r] thys [const AjPTree] Tree
** @return [ajint] Number of nodes in tree.
** @category cast [AjPTree] get the number of nodes in a linked tree.
** @@
******************************************************************************/

ajint ajTreeLength(const AjPTree thys)
{
    ajint icnt = 0;
    if(!thys)
	return 0;

    /* trace tree  - as in ajTreeTrace */
    /* return number of nodes */

    return icnt;
}




/* @func ajTreestrLength ******************************************************
**
** get the number of nodes in the linked tree.
**
** @param [r] thys [const AjPTree] Tree
** @return [ajint] Number of nodes in tree.
** @category cast [AjPTree] get the number of nodes in an AjPStr linked tree.
** @@
******************************************************************************/

ajint ajTreestrLength(const AjPTree thys)
{
    return ajTreeLength(thys);
}




/* @func ajTreeFree ***********************************************************
**
** Free all nodes in the tree.
** NOTE: The data is only freed with a specified tree type.
**       For undefined data types we recommend you to
**       use ajTreeMap with a routine to free the memory.
**
** @param [d] pthis [AjPTree*] Tree
** @return [void]
** @category delete [AjPTree] Free the tree, and free the items with
**                            a simple "free".
** @@
******************************************************************************/

void ajTreeFree(AjPTree* pthis)
{
    AjPTree thys;

    if(!pthis)
	return;

    if(!*pthis)
	return;

   treeDelCnt++;

    thys = *pthis;


    return;
}




/* @func ajTreestrFree ********************************************************
**
** Free all nodes in a string tree.
** Also deletes all the strings. If these are to be preserved,
** use ajTreestrDel instead.
**
** @param [d] pthis [AjPTree*] Tree
** @return [void]
** @category delete [AjPTree] Free the tree, and free the items with ajStrDel
** @@
******************************************************************************/

void ajTreestrFree(AjPTree* pthis)
{
    AjPTree thys;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    treeDelCnt++;

    thys = *pthis;
    AJFREE(*pthis);

    return;
}




/* @func ajTreeDel ************************************************************
**
** Free the tree. Do not attempt to free the nodes.
** For use where the node data has been saved elsewhere, for example
** by ajTreeToArray or where the tree is a temporary structure
** referring to permanent data.
**
** @param [d] pthis [AjPTree*] Tree
** @return [void]
** @category delete [AjPTree] Free the tree but do not try to free the nodes.
**                            Nodes should be freed first by ajTreeMap.
** @@
******************************************************************************/

void ajTreeDel(AjPTree* pthis)
{
    AjPTree tree;

    if(!pthis)
	return;
    if(!*pthis)
	return;

    treeDelCnt++;

    tree = *pthis;

    AJFREE(*pthis);

    return;
}




/* @func ajTreestrDel *********************************************************
**
** Free the tree. Do not attempt to free the nodes.
** For use where the node data has been saved elsewhere, for example
** by ajTreeToArray or where the tree is a temporary structure
** referring to permanent data.
**
** @param [d] pthis [AjPTree*] Tree
** @return [void]
** @category delete [AjPTree] Free the tree but do not try to free the nodes.
**                         use where nodes are still in use,
**                         e.g. in ajTreeToArray.
** @@
******************************************************************************/

void ajTreestrDel(AjPTree* pthis)
{
    ajTreeDel(pthis);

    return;
}




/* @func ajTreeMap ************************************************************
**
** For each node in the tree call function apply.
**
** @param [u] thys [AjPTree] Tree.
** @param [f] apply [void function] Function to call for each tree item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
** @category modify [AjPTree] Call a function for each node in a tree.
** @@
******************************************************************************/

void ajTreeMap(AjPTree thys, void apply(void** x, void* cl), void* cl)
{

    assert(apply);

    return;
}




/* @func ajTreestrMap *********************************************************
**
** For each node in the tree call function apply,
** with the address of the string and a client pointer.
**
** @param [u] thys [AjPTree] Tree.
** @param [f] apply [void function] Function to call for each tree item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
** @category modify [AjPTree] Call a function for each node in a tree.
** @@
******************************************************************************/

void ajTreestrMap(AjPTree thys, void apply(AjPStr* x, void* cl), void* cl)
{

    assert(apply);


    return;
}




/* @func ajTreeToArray ********************************************************
**
** Create an array of the pointers to the data.
**
** @param [r] thys [const AjPTree] Tree
** @param [w] array [void***] Array of pointers to tree items.
** @return [ajint] Size of array of pointers.
** @category cast [AjPTree] Create an array of the pointers to the data.
** @@
******************************************************************************/

ajint ajTreeToArray(const AjPTree thys, void*** array)
{
    ajint n = 0;
 
    return n;
}




/* @func ajTreestrToArray *****************************************************
**
** create an array of the pointers to the data.
**
** @param [r] thys [const AjPTree] Tree
** @param [w] array [AjPStr**] Array of Strings.
**
** @return [ajint] Size of array of pointers.
** @category cast [AjPTree] Create an array of the pointers to the data.
** @@
******************************************************************************/

ajint ajTreestrToArray(const AjPTree thys, AjPStr** array)
{
    ajint n = 0;

    return n;
}






/* @func ajTreeDummyFunction **************************************************
**
** Dummy function to catch all unused functions defined in ajtree
**
** @return [void]
******************************************************************************/

void ajTreeDummyFunction()
{

    return;
}





/* @func ajTreeAddData ********************************************************
**
** Sets the data value in a terminal tree node.
**
** @param [u] thys [AjPTree] Terminal tree node. Must have no descendants
** @param [r] data [void*] Data value
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajTreeAddData(AjPTree thys, void* data)
{
    if (thys->Down) {
	ajErr("tried to define data value for non-terminal tree node\n");
	return ajFalse;
    }

    thys->Data = data;
    return ajTrue;
}

/* @func ajTreeAddNode ********************************************************
**
** Creates a new child node of the parent
**
** @param [u] thys [AjPTree] Parent tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                  define data or further nodes.
******************************************************************************/

AjPTree ajTreeAddNode(AjPTree thys)
{
    AjPTree ret;

    AJNEW0(ret);
    ret->Up = thys;
    thys->Down = ret;

    return ret;
}


/* @func ajTreeAddSubNode *****************************************************
**
** Creates a new sibling node of the parent
**
** @param [u] thys [AjPTree] Left tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                  define data or further nodes.
******************************************************************************/

AjPTree ajTreeAddSubNode(AjPTree thys)
{
    AjPTree ret;

    AJNEW0(ret);
    ret->Up = thys->Up;
    ret->Left = thys;
    thys->Right = ret;

    return ret;
}


/* @func ajTreeTrace *********************************************************
**
** Prints a trace of a tree to debug output
**
** @param [r] thys [const AjPTree] Tree object
** @return [void]
** @@
******************************************************************************/

void ajTreeTrace(const AjPTree thys)
{
    treeTraceNode(thys, 0);

    return;
}

/* @funcstatic treeTraceNode  *************************************************
** **
** Write information about a tee to debug output
**
** @param [r] thys [const AjPTree] Tree node
** @param [r] margin [ajint] Left margin
** @return [void]
******************************************************************************/

static void treeTraceNode (const AjPTree thys, ajint margin)
{
    ajint i;
    ajint ileft = 0;
    ajint iright = 0;
    const AjPTree p;

    for (i=0; i < margin; i++)
	ajDebug(" ");
    ajDebug("node:%x", thys);

    for (i=0; i < margin; i++)
	ajDebug(" ");
    if (thys->Data)
    {
	if (thys->Down)
	    ajDebug(" Data and down link: ** broken node %x **\n", thys);
	else
	    ajDebug(" End node data: %x\n", thys->Data);
	return;
    }
    else if (!thys->Down)
    {
	ajDebug(" No data, no down link: ** broken node %x **\n", thys);
	return;
    }

    for (p=thys;p->Left; p=p->Left)
	ileft++;
    for (p=thys;p->Right; p=p->Right)
	ileft++;
    for (i=0; i < margin; i++)
	ajDebug(" ");
    ajDebug("Left: %d Right: %d\n", ileft, iright);

    if (thys->Down)
	treeTraceNode(thys->Down, margin+2);
    else if (thys->Up)
	treeTraceNode(thys->Right, margin);
    else if (!thys->Right)
	return;

    treeTraceNode(thys->Right, margin);
    return;
 }

/* @func ajTreeExit ***********************************************************
**
** Prints a summary of tree usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajTreeExit(void)
{
    ajDebug("Tree usage : %d opened, %d closed, %d maxsize %d nodes\n",
	     treeNewCnt, treeDelCnt, treeMaxNum, treeNodeCnt);
    ajDebug("Tree iterator usage : %d opened, %d closed, %d maxsize\n",
	     treeIterNewCnt, treeIterDelCnt);

    return;
}