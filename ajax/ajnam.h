#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajnam_h
#define ajnam_h

AjBool ajNamDatabase(const AjPStr name);
AjBool ajNamDbAccess (const AjPStr dbname, const AjPSeqQuery qry,
		      AjPStr* method);
AjBool ajNamDbDetails (const AjPStr name, AjPStr* type, AjBool* id,
		       AjBool* qry, AjBool* all,
		       AjPStr* comment, AjPStr* release);
AjBool ajNamDbGetAlias (const AjPStr dbname, AjPStr* dbalias);
AjBool ajNamDbGetDbalias (const AjPStr dbname, AjPStr* dbalias);
AjBool ajNamDbGetUrl (const AjPStr dbname, AjPStr* url);
AjBool ajNamDbData (AjPSeqQuery qry);
AjBool ajNamDbQuery (AjPSeqQuery qry);
AjBool ajNamDbTest (const AjPStr dbname);
void   ajNamDebugOrigin (void);
void   ajNamDebugDatabases (void);
void   ajNamDebugResources (void);
void   ajNamDebugVariables (void);
void   ajNamExit (void);
AjBool ajNamGetenv (const AjPStr name, AjPStr* value);
AjBool ajNamGetenvC (const char* name, AjPStr* value);
AjBool ajNamGetValue (const AjPStr name, AjPStr* value);
AjBool ajNamGetValueC (const char *name, AjPStr* value);
void   ajNamInit (const char* prefix);
void   ajNamListDatabases(void); /* test routine */
void   ajNamListListDatabases (AjPList dbnames);
void   ajNamListListResources (AjPList dbnames);
void   ajNamListOrigin (void);
void   ajNamPrintDbAttr (AjPFile outf, AjBool full);
void   ajNamPrintRsAttr (AjPFile outf, AjBool full);
AjBool ajNamResolve (AjPStr* name);
AjBool ajNamRoot (AjPStr* root);
AjBool ajNamRootBase (AjPStr* rootbase);
AjBool ajNamRootInstall (AjPStr* root);
AjBool ajNamRootPack (AjPStr* pack);
AjBool ajNamRootVersion (AjPStr* version);
AjBool ajNamSetControl (const char* optionName);
void   ajNamVariables (void); /* "" "" may want to delete later */
#endif

#ifdef __cplusplus
}
#endif

