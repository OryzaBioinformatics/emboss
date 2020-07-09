#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embdata_h
#define embdata_h



void     embDataListDel( AjPList data);
AjPTable embDataListGetTable( AjPList fullList, unsigned int required);
void     embDataListGetTables( AjPList fullList, AjPList returnList,
				 unsigned int required);
void     embDataListInit( AjPList data, AjPStr file_name);

#endif

#ifdef __cplusplus
}
#endif
