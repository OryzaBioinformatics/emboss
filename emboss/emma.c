/* @source emma application
**
** EMBOSS interface to clustal
** @author: Copyright (C) Mark Faller (mfaller@hgmp.mrc.ac.uk)
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




static AjPStr emma_getUniqueFileName();




/* @prog emma *****************************************************************
**
** Multiple alignment program - interface to ClustalW program
**
******************************************************************************/

int main(int argc, char **argv, char **env)
{

    AjPSeqall seqall;
    AjPFile dend_outfile;
    AjPStr tmp_dendfilename = NULL;
    AjPFile tmp_dendfile;

    AjPStr tmp_aln_outfile = NULL;
    AjPSeqset seqset = NULL;
    AjPSeqout seqout = NULL;
    AjPSeqin  seqin  = NULL;

    AjBool only_dend;
    AjBool are_prot = ajFalse;
    AjBool do_slow;
    AjBool use_dend;
    AjPFile dend_file = NULL;
    AjPStr dend_filename = NULL;

    ajint ktup;
    ajint gapw;
    ajint topdiags;
    ajint window;
    AjBool nopercent;

    AjPStr *pw_matrix;
    AjPStr *pw_dna_matrix ;
    AjPFile pairwise_matrix = NULL;
    float pw_gapc;
    float pw_gapv;

    AjPStr pwmstr = NULL;
    char   pwmc   = '\0';
    AjPStr pwdstr = NULL;
    char   pwdc   = '\0';

    AjPStr m1str = NULL;
    AjPStr m2str = NULL;
    AjPStr m3str = NULL;
    char   m1c   = '\0';
    char   m2c   = '\0';

    AjPStr *matrix;
    AjPStr *dna_matrix;
    AjPFile ma_matrix = NULL;
    float gapc;
    float gapv;
    AjBool endgaps;
    AjBool norgap;
    AjBool nohgap;
    ajint gap_dist;
    ajint maxdiv;
    AjPStr hgapres = NULL;


    AjPSeqout fil_file = NULL;
    AjPSeq seq;

    char* prog_default = "clustalw";
    AjPStr cmd = NULL;
    AjPStr tmp;
    AjPStr tmpFilename;
    AjPStr line = NULL;
    ajint nb = 0;


    /* get all the parameters */

    embInit("emma", argc, argv);

    pwmstr = ajStrNew();
    pwdstr = ajStrNew();
    m1str  = ajStrNew();
    m2str  = ajStrNew();
    m3str  = ajStrNew();


    seqall = ajAcdGetSeqall("sequence");
    seqout = ajAcdGetSeqoutset("outseq");

    dend_outfile = ajAcdGetOutfile("dendoutfile");

    only_dend = ajAcdGetToggle("onlydend");
    use_dend  = ajAcdGetToggle("dend");
    dend_file = ajAcdGetInfile("dendfile");
    if (dend_file)
	ajStrAssS(&dend_filename, ajFileGetName(dend_file));
    ajFileClose(&dend_file);

    do_slow = ajAcdGetToggle("slow");

    ktup      = ajAcdGetInt("ktup");
    gapw      = ajAcdGetInt("gapw");
    topdiags  = ajAcdGetInt("topdiags");
    window    = ajAcdGetInt("window");
    nopercent = ajAcdGetBool("nopercent");

    pw_matrix = ajAcdGetList("pwmatrix");
    pwmc = *ajStrStr(*pw_matrix);

    if(pwmc=='b')
	ajStrAssC(&pwmstr,"blosum");
    else if(pwmc=='p')
	ajStrAssC(&pwmstr,"pam");
    else if(pwmc=='g')
	ajStrAssC(&pwmstr,"gonnet");
    else if(pwmc=='i')
	ajStrAssC(&pwmstr,"id");
    else if(pwmc=='o')
	ajStrAssC(&pwmstr,"own");


    pw_dna_matrix = ajAcdGetList("pwdnamatrix");
    pwdc = *ajStrStr(*pw_dna_matrix);

    if(pwdc=='i')
	ajStrAssC(&pwdstr,"iub");
    else if(pwdc=='c')
	ajStrAssC(&pwdstr,"clustalw");
    else if(pwdc=='o')
	ajStrAssC(&pwdstr,"own");

    pairwise_matrix = ajAcdGetInfile("pairwisedata");

    pw_gapc = ajAcdGetFloat( "pwgapopen");
    pw_gapv = ajAcdGetFloat( "pwgapextend");

    matrix = ajAcdGetList( "matrix");
    m1c = *ajStrStr(*matrix);

    if(m1c=='b')
	ajStrAssC(&m1str,"blosum");
    else if(m1c=='p')
	ajStrAssC(&m1str,"pam");
    else if(m1c=='g')
	ajStrAssC(&m1str,"gonnet");
    else if(m1c=='i')
	ajStrAssC(&m1str,"id");
    else if(m1c=='o')
	ajStrAssC(&m1str,"own");


    dna_matrix = ajAcdGetList( "dnamatrix");
    m2c = *ajStrStr(*dna_matrix);

    if(m2c=='b')
	ajStrAssC(&m2str,"iub");
    else if(m2c=='p')
	ajStrAssC(&m2str,"clustalw");
    else if(m2c=='g')
	ajStrAssC(&m2str,"own");


    ma_matrix = ajAcdGetInfile("mamatrix");
    gapc      = ajAcdGetFloat("gapopen");
    gapv      = ajAcdGetFloat("gapextend");
    endgaps   = ajAcdGetBool("endgaps");
    norgap    = ajAcdGetBool("norgap");
    nohgap    = ajAcdGetBool("nohgap");
    gap_dist  = ajAcdGetInt("gapdist");
    hgapres   = ajAcdGetString("hgapres");
    maxdiv    = ajAcdGetInt("maxdiv");

    tmp = ajStrNewC("fasta");

    /*
    ** Start by writing sequences into a unique temporary file
    ** get file pointer to unique file
    */


    fil_file = ajSeqoutNew();
    tmpFilename = ajStrDup( emma_getUniqueFileName());
    if(!ajSeqFileNewOut( fil_file, tmpFilename))
	ajExit();

    /* Set output format to fasta */
    ajSeqOutSetFormat( fil_file, tmp);

    while(ajSeqallNext(seqall, &seq))
    {
        /*
        **  Check sequences are all of the same type
        **  Still to be done
        **  Write out sequences
        */
	if (!nb)
	    are_prot  = ajSeqIsProt(seq);
        ajSeqWrite(fil_file, seq);
	++nb;
    }
    ajSeqWriteClose(fil_file);

    if(nb < 2)
	ajFatal("Multiple alignments need at least two sequences");

    /* Generate clustalw command line */
    if(!ajNamGetValueC("clustalw", &cmd))
      cmd = ajStrNewC(prog_default);

    /* add tmp file containing sequences */
    ajStrAppC(&cmd, " -infile=");
    ajStrAppC(&cmd, ajStrStr( tmpFilename));

    /* add out file name */
    tmp_aln_outfile = ajStrDup(emma_getUniqueFileName());
    ajStrAppC(&cmd, " -outfile=");
    ajStrApp( &cmd, tmp_aln_outfile);


    /* calculating just the nj tree or doing full alignment */
    if(only_dend)
        ajStrAppC(&cmd, " -tree");
    else
        if(!use_dend)
	    ajStrAppC(&cmd, " -align");

    /* Set sequence type from information from acd file */
    if(are_prot)
        ajStrAppC(&cmd, " -type=protein");
    else
        ajStrAppC(&cmd, " -type=dna");


    /*
    **  set output to MSF format - will read in this file later and output
    **  user requested format
    */
    ajStrAppC(&cmd, " -output=");
    ajStrAppC(&cmd, "gcg");

    /* If going to do pairwise alignment */
    if(!use_dend)
    {
        /* add fast pairwise alignments*/
        if(!do_slow)
        {
            ajStrAppC(&cmd, " -quicktree");
            ajStrAppC(&cmd, " -ktuple=");
            ajStrFromInt(&tmp, ktup);
            ajStrApp(&cmd, tmp);
            ajStrAppC(&cmd, " -window=");
            ajStrFromInt(&tmp, window);
            ajStrApp(&cmd, tmp);
            if(nopercent)
                ajStrAppC(&cmd, " -score=percent");
            else
                ajStrAppC(&cmd, " -score=absolute");
            ajStrAppC(&cmd, " -topdiags=");
            ajStrFromInt(&tmp, topdiags);
            ajStrApp(&cmd, tmp);
            ajStrAppC(&cmd, " -pairgap=");
            ajStrFromInt(&tmp, gapw);
            ajStrApp(&cmd, tmp);
        }
        else
        {
            if(!pairwise_matrix)
            {
		if(are_prot)
		{
		    ajStrAppC(&cmd, " -pwmatrix=");
		    ajStrApp(&cmd, pwmstr);
		}
		else
		{
		    ajStrAppC(&cmd, " -pwdnamatrix=");
		    ajStrApp(&cmd, pwdstr);
		}
            }
            else
            {
		if(are_prot)
		    ajStrAppC(&cmd, " -pwmatrix=");
		else
		    ajStrAppC(&cmd, " -pwdnamatrix=");
		ajStrApp(&cmd, ajFileGetName(pairwise_matrix));
            }
            ajStrAppC(&cmd, " -pwgapopen=");
            ajStrFromFloat(&tmp, pw_gapc, 3);
            ajStrApp(&cmd, tmp);
            ajStrAppC(&cmd, " -pwgapext=");
            ajStrFromFloat(&tmp, pw_gapv, 3);
            ajStrApp(&cmd, tmp);
        }
    }

    /* Multiple alignments */

    /* using existing tree or generating new tree? */
    if(use_dend)
    {
        ajStrAppC(&cmd, " -usetree=");
        ajStrApp(&cmd, dend_filename);
    }
    else
    {
	/* use tmp file to hold dend file, will read back in later */
	tmp_dendfilename = ajStrDup(emma_getUniqueFileName());
        ajStrAppC(&cmd, " -newtree=");
        ajStrApp(&cmd, tmp_dendfilename);
    }

    if(!ma_matrix)
    {
	if(are_prot)
	{
	    ajStrAppC(&cmd, " -matrix=");
	    ajStrApp(&cmd, m1str);
	}
	else
	{
	    ajStrAppC(&cmd, " -dnamatrix=");
	    ajStrApp(&cmd, m2str);
	}
    }
    else
    {
	if(are_prot)
	    ajStrAppC(&cmd, " -matrix=");
	else
	    ajStrAppC(&cmd, " -pwmatrix=");
	ajStrApp(&cmd, ajFileGetName(ma_matrix));
    }

    ajStrAppC(&cmd, " -gapopen=");
    ajStrFromFloat(&tmp, gapc, 3);
    ajStrApp(&cmd, tmp);
    ajStrAppC(&cmd, " -gapext=");
    ajStrFromFloat(&tmp, gapv, 3);
    ajStrApp(&cmd, tmp);
    ajStrAppC(&cmd, " -gapdist=");
    ajStrFromInt(&tmp, gap_dist);
    ajStrApp(&cmd, tmp);
    ajStrAppC(&cmd, " -hgapresidues=");
    ajStrApp(&cmd, hgapres);

    if(!endgaps)
	ajStrAppC(&cmd, " -endgaps");

    if(norgap)
	ajStrAppC(&cmd, " -nopgap");

    if(nohgap)
	ajStrAppC(&cmd, " -nohgap");

    ajStrAppC(&cmd, " -maxdiv=");
    ajStrFromInt(&tmp, maxdiv);
    ajStrApp(&cmd, tmp);


    /*  run clustalw */

/*    ajFmtError("..%s..\n\n", ajStrStr( cmd)); */
    ajDebug("Executing '%S'\n", cmd);
    ajSystemEnv(cmd, env);


    /* produce alignment file only if one was produced */
    if(!only_dend)
    {
	/* read in tmp alignment output file to output through EMBOSS output */

	seqin = ajSeqinNew();
	/*
	**  add the Usa format to the start of the filename to tell EMBOSS
	**  format of file
	*/
	ajStrInsertC(&tmp_aln_outfile, 0, "msf::");
	ajSeqinUsa(&seqin, tmp_aln_outfile);
	seqset = ajSeqsetNew();
	if(ajSeqsetRead(seqset, seqin))
	{
	    ajSeqsetWrite(seqout, seqset);


	    ajSeqWriteClose(seqout);
	    ajSeqinDel(&seqin);

	    /* remove the Usa from the start of the string */
	    ajStrTrim(&tmp_aln_outfile, 5);
	}
	else
	    ajFmtError("Problem writing out EMBOSS alignment file");
    }


    /* read in new tmp dend file (if produced) to output through EMBOSS */
    if(tmp_dendfilename!=NULL)
    {
	tmp_dendfile = ajFileNewIn( tmp_dendfilename);

	while(ajFileReadLine(tmp_dendfile, &line))
	    ajFmtPrintF(dend_outfile, "%s\n", ajStrStr( line));

	ajFileClose(&tmp_dendfile);
	ajSysUnlink(tmp_dendfilename);
    }


    ajSysUnlink( tmpFilename);

    if(!only_dend)
	ajSysUnlink(tmp_aln_outfile);

    ajExit();

    return 0;
}




/* @funcstatic emma_getUniqueFileName *****************************************
**
** routine to return a name of a unique file; the  unique file name is the
** process id
**
** @return [AjPStr] Undocumented
** @@
******************************************************************************/

static AjPStr emma_getUniqueFileName()
{
    static char ext[2] = "A";
    AjPStr filename    = NULL;

    /*
    ** A kludge to make filenames greater than 5 characters. This
    ** sometimes, but by no means always, helped bypass a memory
    ** allocation bug in versions of clustalw before 1.83.
    ** You should update your clustalw.
    */
    ajFmtPrintS(&filename, "%08d%s",getpid(), ext);

    if(++ext[0] > 'Z')
	ext[0] = 'A';


    return filename;
}
