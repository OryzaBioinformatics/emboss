/* @source pepstats application
**
** Calculate protein statistics
**
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** Dayhoff statistic by Rodrigo Lopez (rls@ebi.ac.uk)
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
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define DAYHOFF_FILE "Edayhoff.freq"
#define LAM1 (double)15.43
#define LAM2 (double)-29.56
#define CVDISC (double)1.71

/* @prog pepstats *************************************************************
**
** Protein statistics
**
******************************************************************************/

int main(int argc, char **argv)
{
    char *prop[]=
    {
	"Tiny\t\t(A+C+G+S+T)\t\t","Small\t\t(A+B+C+D+G+N+P+S+T+V)\t",
	"Aliphatic\t(I+L+V)\t\t\t","Aromatic\t(F+H+W+Y)\t\t",
	"Non-polar\t(A+C+F+G+I+L+M+P+V+W+Y)\t",
	"Polar\t\t(D+E+H+K+N+Q+R+S+T+Z)\t","Charged\t\t(B+D+E+H+K+R+Z)\t\t",
	"Basic\t\t(H+K+R)\t\t\t","Acidic\t\t(B+D+E+Z)\t\t"
    };

    AjPSeq   a;
    AjPStr   substr;
    ajint      be;
    ajint      en;
    AjPFile  outf;
    AjBool   termini;

    double molwt;
    double charge;
    double iep;
    double molpc;
    ajint *c;
    ajint i;
    ajint j;
    ajint len;
    ajint sum;
    ajint ngps;
    ajint rk;
    ajint de;
    double cv;
    double psolu;

    float   *dhstat=NULL;
    AjPStr  datafn=NULL;
    AjPFile mfptr=NULL;

    embInit("pepstats", argc, argv);

    a         = ajAcdGetSeq("sequence");
    termini   = ajAcdGetBool("termini");
    outf      = ajAcdGetOutfile("outfile");
    datafn    = ajAcdGetString("aadata");

    substr=ajStrNew();
    be=ajSeqBegin(a);
    en=ajSeqEnd(a);
    ajStrAssSubC(&substr,ajSeqChar(a),be-1,en-1);
    len = en-be+1;


   ajFileDataNew(datafn, &mfptr);
    if(!mfptr)
	ajFatal("%S  not found\n",datafn);

    embPropAminoRead(mfptr);


    if(!embReadAminoDataFloatC(DAYHOFF_FILE,&dhstat,0.001))
	ajFatal("Set the EMBOSS_DATA environment variable");


    AJCNEW (c, EMBIEPSIZE);

    embIepComp(ajStrStr(substr),1,c);
    if(!termini)
	c[EMBIEPAMINO]=c[EMBIEPCARBOXYL]=0;

    ajFmtPrintF(outf,"PEPSTATS of %s from %d to %d\n\n",ajSeqName(a),
		be,en);

    ajFmtPrintF(outf,"Molecular weight = %-10.2f\t\tResidues = %-6d\n",
		(molwt=embPropCalcMolwt(ajStrStr(substr),0,len-1)),len);


    for(i=0,charge=0.0;i<26;++i)
	charge += (double)c[i] * EmbPropTable[i][EMBPROPCHARGE];

    ajFmtPrintF(outf,"Average Residue Weight  = %-7.3f \tCharge   = %-6.1f\n",
		molwt/(double)len,charge);

    if(!embIepIEP(ajStrStr(substr),1,&iep,termini))
	ajFmtPrintF(outf,"Isoelectric Point = None\n\n");
    else
	ajFmtPrintF(outf,"Isoelectric Point = %-6.4lf\n",iep);

    ngps = c['N'-'A'] + c['G'-'A'] + c['P'-'A'] + c['S'-'A'];
    rk   = c['R'-'A'] + c['K'-'A'];
    de   = c['D'-'A'] + c['E'-'A'];

    cv = (double)LAM1*((double)((double)ngps/(double)len)) +
	 (double)LAM2*fabs((double)(((double)(rk-de)/(double)len)-
				    (double)0.03));

    psolu = 0.4934 + 0.276*fabs((double)(cv-CVDISC)) -
	0.0392*(cv-CVDISC)*(cv-CVDISC);

    if(cv-CVDISC >= 0.0)
	ajFmtPrintF(outf,"Imp");
    else
	ajFmtPrintF(outf,"P");
    ajFmtPrintF(outf,
		"robability of expression in inclusion bodies = %.3lf\n\n",
		psolu);


    ajFmtPrintF(outf,"Residue\t\tNumber\t\tMole%%\t\tDayhoffStat\n");
    for(i=0;i<26;++i)
    {
	if(!EmbPropTable[i][EMBPROPMOLWT])
	    continue;
	molpc=(100.0 * (double)c[i]/(double)len);
	ajFmtPrintF(outf,"%c = %s\t\t%d\t\t%-7.3f\t\t%-7.3f\t\n",i+'A',
		    embPropIntToThree(i),c[i],molpc,molpc/dhstat[i]);
    }


    ajFmtPrintF(outf,"\nProperty\tResidues\t\tNumber\t\tMole%%\n");
    for(i=1;i<10;++i)
    {
	ajFmtPrintF(outf,"%s",prop[i-1]);
	for(j=0,sum=0;j<26;++j)
	    if(EmbPropTable[j][i])
		sum += c[j];
	ajFmtPrintF(outf,"%d\t\t%6.3f\n",sum,100.0 * (double)sum/(double)len);
    }


    AJFREE(dhstat);
    AJFREE(c);

    ajSeqDel(&a);
    ajStrDel(&substr);
    ajFileClose(&outf);
    ajFileClose(&mfptr);

    ajExit();
    return 0;
}
