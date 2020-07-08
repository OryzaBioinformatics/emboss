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



/* @data AjPTable *******************************************************
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
  int size;
  int (*cmp)(const void *x, const void *y);
  unsigned (*hash)(const void *key, unsigned hashsize);
  int length;
  unsigned timestamp; 
  struct binding **buckets;
} AjOTable, *AjPTable;

AjPTable  ajTableNew (int hint,
		      int cmp(const void *x, const void *y),
		      unsigned hash(const void *key, unsigned hashsize));
void ajTableFree(AjPTable* table);
int   ajTableLength(AjPTable table);
void ajTableTrace   (AjPTable table);
void *ajTablePut   (AjPTable table, const void *key,
		    void *value);
void *ajTableGet   (AjPTable table, const void *key);
void *ajTableRemove(AjPTable table, const void *key);
void   ajTableMap    (AjPTable table,
		      void apply(const void *key, void **value, void *cl),
		      void *cl);
void **ajTableToarray(AjPTable table, void *end);
int        ajStrTableCmp (const void *x, const void *y);
int        ajStrTableCmpC (const void *x, const void *y);
int        ajStrTableCmpCase (const void *x, const void *y);
int        ajStrTableCmpCaseC (const void *x, const void *y);
void       ajStrTableFree (AjPTable *table);
unsigned   ajStrTableHash (const void *key, unsigned hashsize);
unsigned   ajStrTableHashC (const void *key, unsigned hashsize);
unsigned   ajStrTableHashCase (const void *key, unsigned hashsize);
unsigned   ajStrTableHashCaseC (const void *key, unsigned hashsize);
AjPTable   ajStrTableNew (int hint);
AjPTable   ajStrTableNewC (int hint);
AjPTable   ajStrTableNewCase (int hint);
AjPTable   ajStrTableNewCaseC (int hint);
void       ajStrTablePrint (AjPTable table);
void       ajStrTablePrintC (AjPTable table);
#endif

#ifdef __cplusplus
}
#endif
