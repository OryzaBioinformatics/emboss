/********************************************************************
** @source AJAX table functions 
**
** Hash table functions.
**
** @author Copyright (C) 1998 Ian Longden
** @version 1.0
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

#include <limits.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "ajmem.h"
#include "ajassert.h"
#include "ajtable.h"
#include "ajstr.h"
#include "ajmess.h"

static ajint cmpatom(const void *x, const void *y) {
	return x != y;
}
static ajuint hashatom(const void *key, ajuint hashsize) {
	return ((unsigned long)key>>2) % hashsize;
}

static void tableStrPrint (const void* key, void** value, void* cl);
static void tableStrPrintC (const void* key, void** value, void* cl);
static void tableStrDel (const void* key, void** value, void* cl);

/* @func  ajTableNew ***************************************************
**
** creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of key-value pairs. NOTE if cmp=null or hash = null
** ajTableNew uses a function suitable for ajAtom Keys.
** 
** @param [r] hint [ajint] estimate of number of base pairs
** @param [fN] cmp  [ajint function] function for comparing
** @param [fN] hash [ajuint function] function for hashing keys
**
** @return [AjPTable] new table.
**
** @@
**
************************************************************************/

AjPTable ajTableNew(ajint hint,
	ajint cmp(const void *x, const void *y),
	ajuint hash(const void *key, ajuint hashsize)) {
	AjPTable table;
	ajint i;
	static ajint primes[] = { 509, 509, 1021, 2053, 4093,
		8191, 16381, 32771, 65521, INT_MAX };
	assert(hint >= 0);
	for (i = 1; primes[i] < hint; i++)
		;
	table = AJALLOC(sizeof (*table) +
		primes[i-1]*sizeof (table->buckets[0]));
	table->size = primes[i-1];
	table->cmp  = cmp  ?  cmp : cmpatom;
	table->hash = hash ? hash : hashatom;
	table->buckets = (struct binding **)(table + 1);
	for (i = 0; i < table->size; i++)
		table->buckets[i] = NULL;
	table->length = 0;
	table->timestamp = 0;
	return table;
}

/* @func ajTableGet ***************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [AjPTable] table to search
** @param [r] key   [const void*] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
** @@
************************************************************************/

void * ajTableGet(AjPTable table, const void *key) {

  ajint i;
  struct binding *p;

  assert(table);
  assert(key);

  i = (*table->hash)(key, table->size);
  for (p = table->buckets[i]; p; p = p->link) {
    if ((*table->cmp)(key, p->key) == 0)
      break;
  }

  return p ? p->value : NULL;
}

/* @func ajTableKey ***************************************************
**
** returns the key value associated with key in table, or null
** if table does not hold key.
**
** Intended for case-insensitive keys, to return the true key
**
** @param [r] table [AjPTable] table to search
** @param [r] key   [const void*] key to find.
** @return [void*] key value as stored in the table
** @error NULL if key not found in table.
** @@
************************************************************************/

void * ajTableKey(AjPTable table, const void *key) {

  ajint i;
  struct binding *p;

  assert(table);
  assert(key);

  i = (*table->hash)(key, table->size);
  for (p = table->buckets[i]; p; p = p->link) {
    if ((*table->cmp)(key, p->key) == 0)
      break;
  }

  return p ? (void*)p->key : NULL;
}

/* @func ajTableTrace ***************************************************
**
** Writes debug messages to trace the contents of a table.
**
** @param [u] table [AjPTable] Table
** @return [void]
** @@
*************************************************************************/

void ajTableTrace (AjPTable table) {

  ajint i;
  ajint j;
  ajint k=0;
  struct binding *p;

  assert(table);

  ajDebug ("table trace: ");
  ajDebug (" length: %d", table->length);
  ajDebug (" size: %d", table->size);
  ajDebug (" timestamp: %u", table->timestamp);

  for (i = 0; i < table->size; i++) {
    if (table->buckets[i]) {
      j = 0;
      for (p = table->buckets[i]; p; p = p->link) {
	j++;
      }
      k += j;
    }
  }
  ajDebug(" links: %d\n", k);

  return;
}

/* @func ajStrTableTrace ***************************************************
**
** Writes debug messages to trace the contents of a table,
** assuming all keys and values are strings.
**
** @param [u] table [AjPTable] Table
** @return [void]
** @@
*************************************************************************/

void ajStrTableTrace (AjPTable table) {

  ajint i;
  ajint j;
  ajint k=0;
  struct binding *p;

  assert(table);

  ajDebug ("(string) table trace: ");
  ajDebug (" length: %d", table->length);
  ajDebug (" size: %d", table->size);
  ajDebug (" timestamp: %u", table->timestamp);

  for (i = 0; i < table->size; i++) {
    if (table->buckets[i]) {
      j = 0;
      ajDebug("buckets[%d]\n", i);
      for (p = table->buckets[i]; p; p = p->link) {
	ajDebug("   '%S' => '%S'\n", (AjPStr) p->key, (AjPStr) p->value);
	j++;
      }
      k += j;
    }
  }
  ajDebug(" links: %d\n", k);

  return;
}

/* @func ajTablePut ***************************************************
**
** change the value associated with key in table to value and returns
** the previous value, or adds key and value if table does not hold key,
** and returns null.
**
** @param [u] table [AjPTable] Table to add to
** @param [r] key [const void*] key
** @param [r] value [void*] value of key
** @return [void*] previous value if key exists, NULL if not.
** @@
*************************************************************************/

void * ajTablePut(AjPTable table, const void *key, void *value) {

  ajint i;
  struct binding *p;
  void *prev;

  assert(table);
  assert(key);

  i = (*table->hash)(key, table->size);
  for (p = table->buckets[i]; p; p = p->link) {
    if ((*table->cmp)(key, p->key) == 0)
      break;
  }
  if (p == NULL) {
    AJNEW0(p);
    p->key = key;
    p->link = table->buckets[i];
    table->buckets[i] = p;
    table->length++;
    prev = NULL;
  }
  else {
    prev = p->value;
  }

  p->value = value;
  table->timestamp++;

  return prev;
}

/* @func  ajTableLength ***************************************************
**
** returns the number of key-value pairs in table.
**
** @param [r] table [AjPTable] Table to be applied.
** @return [ajint] number of key-value pairs.
** @@
***************************************************************************/

ajint ajTableLength(AjPTable table) {

  assert(table);

  return table->length;
}

/* @func  ajTableMap *****************************************************
**
** calls function 'apply' for each key-value in table
** in an unspecified order.
**
** @param [u] table [AjPTable] Table.
** @param [f] apply [void function] function to be applied
** @param [P] cl [void*] Standard. Usually NULL. To be passed to apply
** @return [void]
** @@
***************************************************************************/

void ajTableMap(AjPTable table,
	void apply(const void *key, void **value, void *cl),
	void *cl) {

  ajint i;
  ajuint stamp;
  struct binding *p;

  assert(table);
  assert(apply);

  stamp = table->timestamp;
  for (i = 0; i < table->size; i++) {
    for (p = table->buckets[i]; p; p = p->link) {
      apply(p->key, &p->value, cl);
      assert(table->timestamp == stamp);
    }
  }

  return;
}

/* @func ajTableRemove **************************************************
**
** removes the key-value pair from table and returns the removed
** value. If table does not hold key, ajTableRemove has no effect
** and returns null.
**
** @param [u] table [AjPTable] Table
** @param [r] key [const void*] key to be removed
** @return [void*] removed value.
** @error NULL if key not found.
** @@
***********************************************************************/

void * ajTableRemove(AjPTable table, const void *key) {

  ajint i;
  struct binding **pp;

  assert(table);
  assert(key);

  table->timestamp++;
  i = (*table->hash)(key, table->size);
  for (pp = &table->buckets[i]; *pp; pp = &(*pp)->link) {
    if ((*table->cmp)(key, (*pp)->key) == 0) {
      struct binding *p = *pp;
      void *value = p->value;
      *pp = p->link;
      AJFREE(p);
      table->length--;
      return value;
    }
  }

  return NULL;
}

/* @func ajTableToarray ****************************************************
**
** creates a 2N+1 element array that holds the N key-value pairs
** in table in an unspecified order and returns a pointer to the
** first element. The keys appear in the even-numbered array
** elements and the corresponding values appear in the following
** odd-numbered elements; element 2N is end.
**
** @param [r] table [AjPTable] Table
** @param [r] end  [void*] end terminator
** @return [void**] pointer to first element in array.
** @@
***************************************************************************/

void ** ajTableToarray(AjPTable table, void *end) {
  ajint i, j = 0;
  void **array;
  struct binding *p;

  assert(table);

  array = AJALLOC((2*table->length + 1)*sizeof (*array));

  for (i = 0; i < table->size; i++)
    for (p = table->buckets[i]; p; p = p->link) {
      array[j++] = (void *)p->key;
      array[j++] = p->value;
    }
  array[j] = end;

  return array;
}

/* @func ajTableFree ******************************************************
**
** deallocates and clears a Table.
**
** @param [u] table [AjPTable*] Table (by reference)
** @return [void]
** @@
******************************************************************************/

void ajTableFree(AjPTable* table) {

  assert(table && *table);

  if ((*table)->length > 0) {
    ajint i;
    struct binding *p, *q;
    for (i = 0; i < (*table)->size; i++) {
      for (p = (*table)->buckets[i]; p; p = q) {
	q = p->link;
	AJFREE(p);
      }
    }
  }

  AJFREE(*table);

  return;
}

/* @section String Table Functions ********************************************
**
** These functions are the equivalent of the ajTable functions for string
** and character string keys
**
******************************************************************************/

/* @func ajStrTableNewCaseC **************************************************
**
** Creates a table with a character string key and case insensitive searching.
**
** @param [r] hint [ajint] Hash size estimate.
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajStrTableNewCaseC (ajint hint) {
  return ajTableNew (hint, ajStrTableCmpCaseC, ajStrTableHashCaseC);
}

/* @func ajStrTableNewCase **************************************************
**
** Creates a table with a string key and case insensitive searching.
**
** @param [r] hint [ajint] Hash size estimate.
** @return [AjPTable] New table object with a string key.
** @@
******************************************************************************/

AjPTable ajStrTableNewCase (ajint hint) {
  return ajTableNew (hint, ajStrTableCmpCase, ajStrTableHashCase);
}

/* @func ajStrTableNewC ******************************************************
**
** Creates a table with a character string key.
**
** @param [r] hint [ajint] Hash size estimate.
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajStrTableNewC (ajint hint) {
  return ajTableNew (hint, ajStrTableCmpC, ajStrTableHashC);
}

/* @func ajStrTableNew ******************************************************
**
** Creates a table with a string key
**
** @param [r] hint [ajint] Hash size estimate.
** @return [AjPTable] New table object with a string key.
** @@
******************************************************************************/

AjPTable ajStrTableNew (ajint hint) {
  return ajTableNew (hint, ajStrTableCmp, ajStrTableHash);
}

/* @func ajStrTableHashCaseC **************************************************
**
** Hash function for a table with a character string key and
** case insensitivity.
**
** @param [P] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value.
** @@
******************************************************************************/

ajuint ajStrTableHashCaseC (const void* key, ajuint hashsize) {
  ajuint hash;
  char* s = (char*) key;

  for (hash = 0; *s; s++) {
    hash = (hash * 127 + toupper((ajint)*s)) % hashsize;
  }

  return hash;
}

/* @func ajStrTableHashCase **************************************************
**
** Hash function for a table with a string key and
** case insensitivity.
**
** @param [P] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value.
** @@
******************************************************************************/

ajuint ajStrTableHashCase (const void* key, ajuint hashsize) {
  AjPStr str = (AjPStr) key;
  char* s = ajStrStr(str);

  ajuint hash;

  for (hash = 0; *s; s++) {
    hash = (hash * 127 + toupper((ajint)*s)) % hashsize;
  }

  return hash;
}

/* @func ajStrTableHashC ******************************************************
**
** Hash function for a table with a character string key
**
** @param [P] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value.
** @@
******************************************************************************/

ajuint ajStrTableHashC (const void* key, ajuint hashsize) {
  ajuint hash;
  char* s = (char*) key;

  for (hash = 0; *s; s++) {
    hash = (hash * 127 + *s) % hashsize;
  }

  return hash;
}

/* @func ajStrTableHash ******************************************************
**
** Hash function for a table with a string key
**
** @param [P] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value.
** @@
******************************************************************************/

ajuint ajStrTableHash (const void* key, ajuint hashsize) {
  AjPStr str = (AjPStr) key;
  char* s = ajStrStr(str);

  ajuint hash;

  for (hash = 0; *s; s++) {
    hash = (hash * 127 + *s) % hashsize;
  }

  return hash;
}

/* @func ajStrTableCmpCaseC **************************************************
**
** Comparison function for a table with a character string key
** and case insensitivity.
**
** @param [P] x [const void*] Standard argument. Item value.
** @param [P] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. 0 if equal, +1 if x is higher.
**               -1 if x is lower.
** @@
******************************************************************************/

ajint ajStrTableCmpCaseC (const void* x, const void* y) {
  char* sx = (char*) x;
  char* sy = (char*) y;

  return (ajint)ajStrCmpCaseCC (sx, sy);
}

/* @func ajStrTableCmpCase **************************************************
**
** Comparison function for a table with a string key
** and case insensitivity.
**
** @param [P] x [const void*] Standard argument. Item value.
** @param [P] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. 0 if equal, +1 if x is higher.
**               -1 if x is lower.
** @@
******************************************************************************/

ajint ajStrTableCmpCase (const void* x, const void* y) {
  AjPStr sx = (AjPStr) x;
  AjPStr sy = (AjPStr) y;

  return (ajint)ajStrCmpCase (sx, sy);
}

/* @func ajStrTableCmpC ******************************************************
**
** Comparison function for a table with a character string key
**
** @param [P] x [const void*] Standard argument. Item value.
** @param [P] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. 0 if equal, +1 if x is higher.
**               -1 if x is lower.
** @@
******************************************************************************/

ajint ajStrTableCmpC (const void* x, const void* y) {
  char* sx = (char*) x;
  char* sy = (char*) y;

  return (ajint)strcmp (sx, sy);
}

/* @func ajStrTableCmp ******************************************************
**
** Comparison function for a table with a string key
**
** @param [P] x [const void*] Standard argument. Item value.
** @param [P] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. 0 if equal, +1 if x is higher.
**               -1 if x is lower.
** @@
******************************************************************************/

ajint ajStrTableCmp (const void* x, const void* y) {
  AjPStr sx = (AjPStr) x;
  AjPStr sy = (AjPStr) y;

  return (ajint)ajStrCmpO (sx, sy);
}

/* @func ajStrTablePrint ******************************************************
**
** Print function for a table with a string key.
**
** @param [P] table [AjPTable] Table.
** @return [void]
** @@
******************************************************************************/

void ajStrTablePrint (AjPTable table) {
  ajTableMap (table, tableStrPrint, NULL);
}

/* @funcstatic tableStrPrint **************************************************
**
** Print function for entries in a a table with a string key.
**
** @param [P] key [const void*] Standard argument. Table key.
** @param [P] value [void**] Standard argument. Table item.
** @param [P] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void tableStrPrint (const void* key, void** value, void* cl) {
  AjPStr keystr = (AjPStr) key;
  AjPStr valstr = (AjPStr) *value;

  ajUser("key '%S' value '%S'", keystr, valstr);
}

/* @func ajStrTablePrintC *****************************************************
**
** Print function for a table with a character string key.
**
** @param [P] table [AjPTable] Table.
** @return [void]
** @@
******************************************************************************/

void ajStrTablePrintC (AjPTable table) {
  ajTableMap (table, tableStrPrintC, NULL);
}

/* @funcstatic tableStrPrintC *************************************************
**
** Print function for entries in a table with a character string key.
**
** @param [P] key [const void*] Standard argument. Table key.
** @param [P] value [void**] Standard argument. Table item.
** @param [P] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void tableStrPrintC (const void* key, void** value, void* cl) {
  char* keystr = (char*) key;
  char* valstr = (char*) *value;

  ajUser("key '%s' value '%s'", keystr, valstr);
}

/* @func ajStrTableFree *******************************************************
**
** Free strings in a table and free the table. Use only where the strings
** in the table are real, and not just copies of pointers. Otherwise
** a call to ajTableFree is enough.
**
** @param [P] ptable [AjPTable*] Table
** @return [void]
** @@
******************************************************************************/

void ajStrTableFree (AjPTable* ptable) {

  if (!*ptable) return;

  ajTableMap (*ptable, tableStrDel, NULL);

  ajTableFree (ptable);

  return;
}

/* @funcstatic tableStrDel ****************************************************
**
** Delete an entry in a string table.
**
** @param [P] key [const void*] Standard argument. Table key.
** @param [P] value [void**] Standard argument. Table item.
** @param [P] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void tableStrDel (const void* key, void** value, void* cl) {
  AjPStr* p = (AjPStr*) value;
  AjPStr  q = (AjPStr) key;

  ajStrDel(p);
  ajStrDel(&q);

  return;
}


