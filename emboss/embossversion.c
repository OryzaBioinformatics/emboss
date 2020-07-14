/* @source embossversion application
**
** Writes the current EMBOSS version number
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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




/* @prog embossversion ********************************************************
**
** Writes the current EMBOSS version number
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outfile = NULL;
    AjBool full = AJFALSE;
    AjPStr tmpstr = NULL;

    embInit("embossversion", argc, argv);

    full = ajAcdGetBool("full");
    outfile = ajAcdGetOutfile("outfile");

    if (!full) {
	ajFmtPrintF(outfile,"%s\n", VERSION);
    }
    else {
	tmpstr = ajStrNewL(128);

	if (!ajNamRootPack(&tmpstr))
	    ajStrAssC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "PackageName: %S\n", tmpstr);

	if (!ajNamRootVersion(&tmpstr))
	    ajStrAssC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "LibraryVersion: %S\n", tmpstr);

	if (!ajNamRootInstall(&tmpstr))
	    ajStrAssC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "InstallDirectory: %S\n", tmpstr);

	if (!ajNamRoot(&tmpstr))
	    ajStrAssC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "RootDirectory: %S\n", tmpstr);
	    
	if (!ajNamRootBase(&tmpstr))
	    ajStrAssC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "BaseDirectory: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("acdroot", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_AcdRoot: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("data", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Data: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("filter", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Filter: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("format", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Format: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("graphics", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Graphics: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("httpversion", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_HttpVersion: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("language", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Language: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("options", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Options: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("outdirectory", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_OutDirectory: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("outfeatformat", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_OutFeatFormat: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("outformat", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_OutFormat: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("proxy", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Proxy: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("stdout", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Stdout: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("warnrange", &tmpstr))
	    ajStrAssC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_WarnRange: %S\n", tmpstr);
	    
    }

    ajFileClose(&outfile);

    ajExit();

    return 0;
}
