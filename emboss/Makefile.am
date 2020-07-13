
SUBDIRS = acd data

EXTRA_DIST = emboss.default.template

if ISSHARED
if ISAIXIA64
AM_CFLAGS = -Wl,-bdynamic -Wl,-brtl -L../ajax/.libs -lajax
else
AM_CFLAGS =
endif
endif

check_PROGRAMS = ajtest \
complex corbatest \
demoalign demofeatures demolist demoreport demosequence demostring demotable \
entrails histogramtest intconv \
patmattest \
prima primers \
seqinfo seqretall seqretallfeat seqretset seqretsingle \
sigplot \
testplot treetypedisplay

bin_PROGRAMS = aaindexextract abiview acdc antigenic \
backtranseq banana biosed btwisted \
cai chaos charge checktrans chips cirdna codcmp coderet \
contacts compseq cons cpgplot cpgreport cusp cutgextract cutseq \
dan dbiblast dbifasta dbiflat dbigcg \
degapseq descseq diffseq digest distmat domainer \
dotmatcher dotpath dottup dreg \
einverted embossdata embossversion emma emowse entret \
eprimer3 equicktandem est2genome etandem extractfeat extractseq \
findkm fraggle freak funky fuzznuc fuzzpro fuzztran \
garnier geecee getorf hetparse helixturnhelix hmmgen hmoment \
iep infoalign infoseq interface isochore jembossctl lindna listor \
marscan maskfeat maskseq matcher megamerger merger msbar \
mwcontam mwfilter \
needle newcpgreport newcpgseek newseq \
noreturn notseq nthseq \
octanol oddcomp \
palindrome pasteseq patmatdb patmatmotifs \
pdbparse pdbtosp pepcoil pepinfo pepnet pepstats \
pepwheel pepwindow pepwindowall \
plotcon plotorf polydot preg \
prettyplot prettyseq primersearch printsextract \
profgen profit prophecy prophet \
prosextract pscan \
rebaseextract recoder redata remap restover restrict revseq scopnr scopparse \
scopseqs scoprep scopreso seealso seqalign seqmatchall seqnr seqret seqretsplit seqsearch seqsort seqwords \
showalign showdb showfeat showorf showseq \
shuffleseq sigcleave siggen sigscan silent skipseq \
splitter scopalign stretcher \
stssearch supermatcher syco \
textsearch tfextract tfm tfscan \
tmap tranalign transeq trimest trimseq \
union vectorstrip \
water whichdb wobble wordcount wordmatch \
wossname yank

INCLUDES = -I$(top_srcdir)/nucleus -I$(top_srcdir)/ajax -I$(top_srcdir)/plplot

aaindexextract_SOURCES = aaindexextract.c
abiview_SOURCES = abiview.c
acdc_SOURCES = acdc.c
antigenic_SOURCES = antigenic.c
backtranseq_SOURCES = backtranseq.c
banana_SOURCES = banana.c
biosed_SOURCES = biosed.c
btwisted_SOURCES = btwisted.c
chaos_SOURCES = chaos.c
cai_SOURCES = cai.c
charge_SOURCES = charge.c
checktrans_SOURCES = checktrans.c
chips_SOURCES = chips.c
cirdna_SOURCES = cirdna.c
codcmp_SOURCES = codcmp.c
coderet_SOURCES = coderet.c
complex_SOURCES = complex.c
compseq_SOURCES = compseq.c
cons_SOURCES = cons.c
contacts_SOURCES = contacts.c
corbatest_SOURCES = corbatest.c
cpgplot_SOURCES = cpgplot.c
cpgreport_SOURCES = cpgreport.c
cusp_SOURCES = cusp.c
cutgextract_SOURCES = cutgextract.c
cutseq_SOURCES = cutseq.c
dan_SOURCES = dan.c
dbiblast_SOURCES = dbiblast.c
dbifasta_SOURCES = dbifasta.c
dbiflat_SOURCES = dbiflat.c
dbigcg_SOURCES = dbigcg.c
degapseq_SOURCES = degapseq.c
descseq_SOURCES = descseq.c
diffseq_SOURCES = diffseq.c
digest_SOURCES = digest.c
distmat_SOURCES = distmat.c
domainer_SOURCES = domainer.c
dotmatcher_SOURCES = dotmatcher.c
dottup_SOURCES = dottup.c
dreg_SOURCES = dreg.c
einverted_SOURCES = einverted.c
embossdata_SOURCES = embossdata.c
emma_SOURCES = emma.c
emowse_SOURCES = emowse.c
entrails_SOURCES = entrails.c
entret_SOURCES = entret.c
eprimer3_SOURCES = eprimer3.c
equicktandem_SOURCES = equicktandem.c
est2genome_SOURCES = est2genome.c
etandem_SOURCES = etandem.c
embossversion_SOURCES = embossversion.c
extractfeat_SOURCES = extractfeat.c
extractseq_SOURCES = extractseq.c
findkm_SOURCES = findkm.c
fraggle_SOURCES = fraggle.c
freak_SOURCES = freak.c
funky_SOURCES = funky.c
fuzznuc_SOURCES = fuzznuc.c
fuzzpro_SOURCES = fuzzpro.c
fuzztran_SOURCES = fuzztran.c
garnier_SOURCES = garnier.c
geecee_SOURCES = geecee.c
getorf_SOURCES = getorf.c
helixturnhelix_SOURCES = helixturnhelix.c
hetparse_SOURCES = hetparse.c
hmmgen_SOURCES = hmmgen.c
hmoment_SOURCES = hmoment.c
iep_SOURCES = iep.c
infoalign_SOURCES = infoalign.c
infoseq_SOURCES = infoseq.c
intconv_SOURCES = intconv.c
interface_SOURCES = interface.c
isochore_SOURCES = isochore.c
jembossctl_SOURCES = jembossctl.c
lindna_SOURCE = lindna.c
listor_SOURCE = listor.c
marscan_SOURCE = marscan.c
maskfeat_SOURCES = maskfeat.c
maskseq_SOURCES = maskseq.c
matcher_SOURCES = matcher.c
msbar_SOURCES = msbar.c
megamerger_SOURCES = megamerger.c
merger_SOURCES = merger.c
mwcontam_SOURCES = mwcontam.c
mwfilter_SOURCES = mwfilter.c
needle_SOURCES = needle.c
newcpgreport_SOURCES = newcpgreport.c
newcpgseek_SOURCES = newcpgseek.c
newseq_SOURCES = newseq.c
noreturn_SOURCES = noreturn.c
notseq_SOURCES = notseq.c
nthseq_SOURCES = nthseq.c
octanol_SOURCES = octanol.c
oddcomp_SOURCES = oddcomp.c
palindrome_SOURCES = palindrome.c
pasteseq_SOURCES = pasteseq.c
patmatdb_SOURCES = patmatdb.c
patmatmotifs_SOURCES = patmatmotifs.c
pdbparse_SOURCES = pdbparse.c
pdbtosp_SOURCES = pdbtosp.c
pepcoil_SOURCES = pepcoil.c
pepinfo_SOURCE = pepinfo.c
pepnet_SOURCES = pepnet.c
pepstats_SOURCES = pepstats.c
pepwheel_SOURCES = pepwheel.c
pepwindow_SOURCES = pepwindow.c
pepwindowall_SOURCES = pepwindowall.c
plotcon_SOURCES = plotcon.c
plotorf_SOURCES = plotorf.c
polydot_SOURCES = polydot.c
preg_SOURCES = preg.c
prettyplot_SOURCES = prettyplot.c
prettyseq_SOURCES = prettyseq.c
prima_SOURCES = prima.c
primers_SOURCES = primers.c
primersearch_SOURCES = primersearch.c
printsextract_SOURCES = printsextract.c
profgen_SOURCES = profgen.c
profit_SOURCES = profit.c
prophecy_SOURCES = prophecy.c
prophet_SOURCES = prophet.c
prosextract_SOURCES = prosextract.c
pscan_SOURCES = pscan.c
rebaseextract_SOURCES = rebaseextract.c
recoder_SOURCES = recoder.c
redata_SOURCES = redata.c
remap_SOURCES = remap.c
restover_SOURCES = restover.c
restrict_SOURCES = restrict.c
revseq_SOURCES = revseq.c
scopnr_SOURCES = scopnr.c
scopparse_SOURCES = scopparse.c
scopseqs_SOURCES = scopseqs.c
scoprep_SOURCES = scoprep.c
scopreso_SOURCES = scopreso.c
seealso_SOURCES = seealso.c
seqalign_SOURCES = seqalign.c
seqmatchall_SOURCES = seqmatchall.c
seqnr_SOURCES = seqnr.c
seqret_SOURCES = seqret.c
seqsearch_SOURCES = seqsearch.c
seqsort_SOURCES = seqsort.c
seqwords_SOURCES = seqwords.c
seqinfo_SOURCES = seqinfo.c
seqretall_SOURCES = seqretall.c
seqretallfeat_SOURCES = seqretallfeat.c
seqretset_SOURCES = seqretset.c
seqretsplit_SOURCES = seqretsplit.c
showalign_SOURCES = showalign.c
showdb_SOURCES = showdb.c
showfeat_SOURCES = showfeat.c
showorf_SOURCES = showorf.c
showseq_SOURCES = showseq.c
shuffleseq_SOURCES = shuffleseq.c
sigcleave_SOURCES = sigcleave.c
siggen_SOURCES = siggen.c
silent_SOURCES = silent.c
splitter_SOURCES = splitter.c
scopalign_SOURCES = scopalign.c
stretcher_SOURCES = stretcher.c
stssearch_SOURCES = stssearch.c
supermatcher_SOURCES = supermatcher.c
syco_SOURCES = syco.c
textsearch_SOURCES = textsearch.c
tfextract_SOURCES = tfextract.c
tfm_SOURCES = tfm.c
tfscan_SOURCES = tfscan.c
tmap_SOURCES = tmap.c
tranalign_SOURCES = tranalign.c
transeq_SOURCES = transeq.c
trimest_SOURCES = trimest.c
trimseq_SOURCES = trimseq.c
union_SOURCES = union.c
vectorstrip_SOURCES = vectorstrip.c
water_SOURCES = water.c
whichdb_SOURCES = whichdb.c
wobble_SOURCES = wobble.c
wordcount_SOURCES = wordcount.c
wordmatch_SOURCES = wordmatch.c
wossname_SOURCES = wossname.c
yank_SOURCES = yank.c


ajtest_SOURCES = ajtest.c
demoalign_SOURCES = demoalign.c
demofeatures_SOURCES = demofeatures.c
demolist_SOURCES = demolist.c
demoreport_SOURCES = demoreport.c
demosequence_SOURCES = demosequence.c
demostring_SOURCES = demostring.c
demotable_SOURCES = demotable.c
histogramtest_SOURCES = histogramtest.c
patmattest_SOURCES = patmattest.c
seqretsingle_SOURCES = seqretsingle.c
sigplot_SOURCES = sigplot.c
sigscan_SOURCES = sigscan.c
testplot_SOURCES = testplot.c
treetypedisplay_SOURCES = treetypedisplay.c

LDADD = ../nucleus/libnucleus.la ../ajax/libajaxg.la ../ajax/libajax.la ../plplot/libplplot.la $(XLIB)

pkgdata_DATA = emboss.default.template
pkgdatadir=$(prefix)/share/$(PACKAGE)

#if PURIFY
#  LINK = purify $(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(LDFLAGS) -o $@
#else
#
#endif
