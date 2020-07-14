#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtable_h
#define ajtable_h




struct binding {
	struct binding *link;
	const void *key;
        void *value;
};



/* @data AjPTable *************************************************************
**
** Hash table object. Tables are key/value pairs with a simple hash function
** to provide rapid searching for keys.
**
** Tables can hold any data type. Special functions are available for
** tables of AjPStr values, but these are in the ajstr library,
** and start with ajStrTable...
**
** In general, these functions are the same
** but with different hash and comparison functions used. Alternative
** function names are provided in all cases to save remembering which
** calls need special cases.
**
** @new ajTableNew Creates a table.
** @delete ajTableFree Deallocates and clears a table.
** @modify ajTablePut Adds or updates a value for a given key.
** @modify ajTableMap Calls a function for each key/value in a table.
** @modify ajTableRemove Removes a key/value pair from a table, and returns
**                    the value.
** @cast ajTableToarray Creates an array to hold each key value pair
**                     in pairs of array elements. The last element is null.
** @cast ajTableGet Returns the value for a given key.
** @cast ajTableLength Returns the number of keys in a table.
** @output ajTableTrace Writes debug messages to trace the contents of a table.
**
** @attr size [ajint] Size - number of hash buckets
** @attr cmp [(ajint*)] Compare function (0 for match, -1 or +1 if not matched)
** @attr hash [(unsigned*)] Hash function
** @attr length [ajint] Number of entries
** @attr timestamp [unsigned] Time stamp
** @attr buckets [struct binding**] Buckets
** @@
******************************************************************************/

typedef struct AjSTable {
  ajint size;
  ajint (*cmp)(const void *x, const void *y);
  unsigned (*hash)(const void *key, unsigned hashsize);
  ajint length;
  unsigned timestamp;
  struct binding **buckets;
} AjOTable;

#define AjPTable AjOTable*

void       ajTableExit(void);
AjPTable   ajTableNew (ajint hint,
		       ajint cmp(const void *x, const void *y),
		       unsigned hash(const void *key, unsigned hashsize));
AjPTable   ajTableNewL (ajint size,
			ajint cmp(const void *x, const void *y),
			unsigned hash(const void *key, unsigned hashsize));
void       ajTableFree (AjPTable* table);
void*      ajTableGet  (const AjPTable table, const void *key);
void*      ajTableKey  (const AjPTable table, const void *key);
ajint      ajTableLength (const AjPTable table);
void       ajTableMap    (AjPTable table,
			  void apply(const void *key, void **value, void *cl),
			  void *cl);
void       ajTableMapDel (AjPTable table,
			  void apply(const void **key, void **value, void *cl),
			  void *cl);
void*      ajTablePut   (AjPTable table, const void *key,
			 void *value);
void*      ajTableRemove (AjPTable table, const void *key);
void**     ajTableToarray (const AjPTable table, void *end);
void       ajTableTrace   (const AjPTable table);

ajint      ajStrTableCmp      (const void *x, const void *y);
ajint      ajStrTableCmpC     (const void *x, const void *y);
ajint      ajStrTableCmpCase  (const void *x, const void *y);
ajint      ajStrTableCmpCaseC (const void *x, const void *y);
void       ajStrTableFree (AjPTable *table);
unsigned   ajStrTableHash      (const void *key, unsigned hashsize);
unsigned   ajStrTableHashC     (const void *key, unsigned hashsize);
unsigned   ajStrTableHashCase  (const void *key, unsigned hashsize);
unsigned   ajStrTableHashCaseC (const void *key, unsigned hashsize);
AjPTable   ajStrTableNew  (ajint hint);
AjPTable   ajStrTableNewC (ajint hint);
AjPTable   ajStrTableNewCase  (ajint hint);
AjPTable   ajStrTableNewCaseC (ajint hint);
void       ajStrTablePrint  (const AjPTable table);
void       ajStrTablePrintC (const AjPTable table);
void       ajStrTableTrace (const AjPTable table);

#endif

#ifdef __cplusplus
}
#endif
