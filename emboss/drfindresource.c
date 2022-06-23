/** @source drfindresource
**
** Find public databases by category
**
** @author Copyright (C) 2010 Jon Ison / EMBOSS
** @version 1  First version</replaceable>
** @modified July 2010  Jon Ison First version</replaceable>
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
********************************************************************/


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "emboss.h"

/* Inclusion of system and local header files goes here */



/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */



/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */



/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */

/* Definition of datatypes go here */



/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

/* Function prototypes for public (external) functions go here */




/* @prog drfindresource *******************************************************
**
** Find public databases by category
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPStr   query;
    AjPOutfile  outfile = NULL;    

    AjPResource resource = NULL;
    AjPResourcein resourcein = NULL;
    AjPOboin oboin = NULL;
    AjPObo obo = NULL;
    AjPStr oboqry = NULL;
    AjPStr resourceqry = NULL;
    AjPStr qrystr = NULL;
    AjPTable obotable = NULL;
    AjPTable foundtable = NULL;
    AjBool sensitive = ajFalse;
    AjBool subclasses = ajFalse;

    AjPStrTok handle = NULL;
    AjPList obolist = NULL;
    AjPObo obotest = NULL;

    ajuint i;
    ajuint imax = 3;

    const char* fields[] = {"id", "acc", "nam", "des"};
    
    /* ACD processing */
    embInit("drfindresource", argc, argv);

    query     = ajAcdGetString("query");
    outfile   = ajAcdGetOutresource("outfile");
    sensitive = ajAcdGetBoolean("sensitive");
    subclasses = ajAcdGetBoolean("subclasses");

    /* Application logic */
    /* Check DRCAT data resource catalogue drcat.dat is installed indexed.
       Loop through queryable fields
        :Return list of DRCAT entries with fields matching keyword(s)
       Merge lists of matching entries
       Write output file */

    resourcein = ajResourceinNew();
    resource = ajResourceNew();
    oboin = ajOboinNew();
    obo = ajOboNew();

    obolist = ajListNew();
    obotable = ajTablestrNew(600);
    foundtable = ajTablestrNew(600);

    if(sensitive)
        imax++;

    handle = ajStrTokenNewC(query, ",");
    while(ajStrTokenNextParse(handle, &qrystr))
    {
        for(i=0;i<imax;i++)
        {
            ajFmtPrintS(&oboqry, "edam-%s:%S", fields[i], qrystr);

            ajOboinQryS(oboin, oboqry);

            while(ajOboinRead(oboin, obo))
            {
                ajListPushAppend(obolist, ajOboNewObo(obo));
                if(subclasses)
                    ajOboGetTree(obo, obolist);

                ajDebug("%S '%S' %Lu\n",
                       qrystr, obo->Id, ajListGetLength(obolist));
                while(ajListGetLength(obolist))
                {
                    ajListPop(obolist, (void**) &obotest);

                    if(!ajTableMatchS(obotable, obotest->Id))
                    {
                        ajDebug("edam id '%S' namespace '%S' %d '%S'\n",
                                obotest->Id, obotest->Namespace,
                                fields[i], obotest->Name );
                        ajTablePut(obotable, ajStrNewS(obotest->Id),
                                   (void *) 1);
                        ajFmtPrintS(&resourceqry, "drcat-etpc:%S",
                                    ajOboGetId(obotest));
                        ajResourceinQryS(resourcein, resourceqry);

                        while(ajResourceinRead(resourcein, resource))
                        {
                            if(!ajTableMatchS(foundtable, resource->Id))
                            {
                                ajDebug("drcat id '%S' categories %Lu\n",
                                        resource->Id,
                                        ajListGetLength(resource->Cat));
                                 ajResourceoutWrite(outfile, resource);
                                ajTablePut(foundtable, ajStrNewS(resource->Id),
                                           (void *) 1);
                            }
                        }
                    }
                     ajOboDel(&obotest);
               }
            }
        }
    }

    /* Memory clean-up and exit */

    ajOboDel(&obo);
    ajOboinDel(&oboin);

    ajResourceDel(&resource);
    ajResourceinDel(&resourcein);

    ajListFree(&obolist);

    ajStrTokenDel(&handle);

    ajStrDel(&qrystr);
    ajStrDel(&query);
    ajStrDel(&oboqry);
    ajStrDel(&resourceqry);

    ajTablestrFreeKey(&obotable);
    ajTablestrFreeKey(&foundtable);

    ajOutfileClose(&outfile);
    
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */

