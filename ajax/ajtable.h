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
** @set ajTablePut Adds or updates a value for a given key.
** @set ajTableMap Calls a function for each key/value in a table.
** @set ajTableRemove Removes a key/value pair from a table, and returns
**                    the value.
** @set ajTableToArray Creates an array to hold each key value pair
**                     in pairs of array elements. The last element is null.
** @cast ajTableGet Returns the value for a given key.
** @cast ajTableLength Returns the number of keys in a table.
** @use ajTableTrace Writes debug messages to trace the contents of a table.
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
void      *ajTableGet  (const AjPTable table, const void *key);
void      *ajTableKey  (const AjPTable table, const void *key);
ajint      ajTableLength (const AjPTable table);
void       ajTableMap    (AjPTable table,
			  void apply(const void *key, void **value, void *cl),
			  void *cl);
void      *ajTablePut   (AjPTable table, const void *key,
			 void *value);
void      *ajTableRemove (AjPTable table, const void *key);
void     **ajTableToarray (AjPTable table, void *end);
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
void       ajStrTablePrint  (AjPTable table);
void       ajStrTablePrintC (AjPTable table);
void       ajStrTableTrace (const AjPTable table);

#endif

#ifdef __cplusplus
}
#endif
