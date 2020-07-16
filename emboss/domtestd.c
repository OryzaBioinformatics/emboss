/* @source domtestd application
**
** Create some typical XML and demonstrate searching for element
**
** @author Copyright (C) Alan Bleasby
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
******************************************************************************/

#include "emboss.h"




/* @prog domtestd *************************************************************
**
** Create some typical XML and demonstrate searching for element
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPDomDocument doc = NULL;
    AjPDomDocumentType type  = NULL;

    AjPDomElement query = NULL;
    AjPDomElement dataset = NULL;

    AjPDomElement element = NULL;

    AjPDomElement e = NULL;
    AjPStr attval = NULL;
    
    AjPDomNodeList list = NULL;
    
    AjPFile outf = NULL;

    ajint len;
    ajint i;
    
    embInit("domtestd", argc, argv);

    outf = ajAcdGetOutfile("outfile");

    doc = ajDomImplementationCreateDocument(NULL,NULL,NULL);

    type = ajDomImplementationCreateDocumentTypeC("Query",NULL,NULL);
    ajDomNodeAppendChild(doc, type);

    query = ajDomDocumentCreateElementC(doc, "Query");
    ajDomNodeAppendChild(doc, query);
    ajDomElementSetAttributeC(query, "virtualSchemaName", "default");
    ajDomElementSetAttributeC(query, "formatter", "TSV");
    ajDomElementSetAttributeC(query, "header", "0");
    ajDomElementSetAttributeC(query, "uniqueRows", "0");
    ajDomElementSetAttributeC(query, "datasetConfigVersion", "0.7");

    dataset = ajDomDocumentCreateElementC(doc, "Dataset");
    ajDomElementSetAttributeC(dataset, "name", "drerio_gene_ensembl");
    ajDomElementSetAttributeC(dataset, "interface", "default");
    ajDomNodeAppendChild(query,dataset);

    element = ajDomDocumentCreateElementC(doc, "Filter");
    ajDomElementSetAttributeC(element, "name", "with_transmembrane_domain");
    ajDomElementSetAttributeC(element, "excluded", "0");
    ajDomNodeAppendChild(dataset,element);

    element = ajDomDocumentCreateElementC(doc, "Attribute");
    ajDomElementSetAttributeC(element, "name", "ensembl_gene_id");
    ajDomNodeAppendChild(dataset,element);

    element = ajDomDocumentCreateElementC(doc, "Attribute");
    ajDomElementSetAttributeC(element, "name", "ensembl_transcript_id");
    ajDomNodeAppendChild(dataset,element);


    /*
    ** GetElementsBytagNameC performs a traversal of element nodes
    ** therefore 'query' rather than 'doc' is supplied as the top
    ** of the hierarchy.
    */
    list = ajDomElementGetElementsByTagNameC(query, "Dataset");

    if(list)
    {
        len = list->length;

        ajFmtPrintF(outf,"Found %d match(es)\n", len);

        for (i = 0; i < len; i++)
        {
            e = ajDomNodeListItem(list, i);
            ajFmtPrintF(outf,"Element Name = %S\n", e->name);
            attval = ajDomElementGetAttributeC(e, "name");
            ajFmtPrintF(outf,"Attribute name = %S\n",attval);
            ajStrDel(&attval);
        }
    }
        
    ajDomDocumentDestroyNodeList(doc,list,0);

    ajDomDocumentDestroyNode(doc,doc);

    ajFileClose(&outf);

    embExit();

    return 0;
}
