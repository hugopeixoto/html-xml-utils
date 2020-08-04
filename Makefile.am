# Copyright © 2000-2018 World Wide Web Consortium
# See http://www.w3.org/Consortium/Legal/copyright-software
#
# Author: Bert Bos <bert@w3.org>
# Created: 31 Mar 2000

bin_PROGRAMS =		hxaddid hxcite hxcount hxextract hxclean\
			hxcopy hxprune hxnsxml hxprintlinks\
			hxincl hxindex hxmkbib hxmultitoc hxname2id\
			hxnormalize hxnum hxpipe hxremove\
			hxselect hxtabletrans hxtoc hxuncdata\
			hxunent hxunpipe hxunxmlns hxwls hxxmlns hxref\
			xml2asc asc2xml

noinst_PROGRAMS =	cexport
bin_SCRIPTS = 		hxcite-mkbib

man_MANS =		hxaddid.1 asc2xml.1 hxcite.1\
			hxcite-mkbib.1 hxcopy.1\
			hxcount.1 hxextract.1 hxclean.1\
			hxprune.1 hxincl.1 hxindex.1 hxmkbib.1 hxmultitoc.1\
			hxname2id.1 hxnormalize.1 hxnum.1 hxpipe.1\
			hxprintlinks.1 hxremove.1 hxtabletrans.1\
			hxtoc.1 hxuncdata.1 hxunent.1\
			hxunpipe.1 hxunxmlns.1 hxwls.1 xml2asc.1 hxxmlns.1\
			hxref.1 hxselect.1 hxnsxml.1

EXTRA_DIST =		config.rpath\
			$(man_MANS) dtd.hash unent.hash export.h fopencookie.h\
			$(bin_SCRIPTS) $(BUILT_SOURCES) cexport.1\
			$(TESTS)

doc_DATA =		COPYING

LDADD =			@LIBOBJS@ @LIBCURL@ @LIBICONV@
AM_CPPFLAGS =		@LIBCURL_CPPFLAGS@
AM_YFLAGS =		-d
AM_LFLAGS =		@lex_opt_flags@
ACLOCAL_AMFLAGS =	-I m4

EXPORTS =		dict.e heap.e types.e headers.e connectsock.e\
			dtd.e errexit.e tree.e genid.e html.e url.e\
			openurl.e scan.e textwrap.e unent.e class.e\
			selector.e hash.e selmatch.e langinfo.e

BUILT_SOURCES =		$(EXPORTS) scan.c html.c html.h dtd.c unent.c

asc2xml_SOURCES =	asc2xml.c
hxaddid_SOURCES =	hxaddid.c html.y scan.l dtd.c openurl.c errexit.c\
			url.c connectsock.c heap.c tree.c types.c genid.c\
			fopencookie.h\
			class.c hash.c headers.c dict.c fopencookie.c
cexport_SOURCES =	cexport.c
hxcite_SOURCES =	heap.c errexit.c hxcite.c
hxcount_SOURCES =	hxcount.c html.y scan.l types.c errexit.c heap.c\
			openurl.c url.c connectsock.c headers.c dict.c\
			fopencookie.h fopencookie.c
hxextract_SOURCES =	hxextract.c html.y scan.l openurl.c url.c\
			connectsock.c heap.c errexit.c class.c headers.c\
			dict.c types.c fopencookie.h fopencookie.c
hxclean_SOURCES =	hxclean.c html.y tree.c types.c heap.c dtd.c\
			scan.l errexit.c
hxprune_SOURCES =	hxprune.c tree.c scan.l html.y errexit.c dtd.c\
			heap.c types.c openurl.c url.c connectsock.c class.c\
			headers.c dict.c fopencookie.h fopencookie.c
hxincl_SOURCES =	hxincl.c scan.l html.y openurl.c url.c heap.c\
			errexit.c connectsock.c types.c headers.c dict.c\
			fopencookie.h fopencookie.c
hxindex_SOURCES =	hxindex.c scan.l html.y openurl.c url.c heap.c class.c\
			errexit.c connectsock.c types.c tree.c genid.c dtd.c\
			headers.c dict.c fopencookie.h fopencookie.c
hxmkbib_SOURCES =	errexit.c heap.c hxmkbib.c hash.c
hxmultitoc_SOURCES =	hxmultitoc.c html.y scan.l openurl.c url.c\
			connectsock.c heap.c errexit.c class.c headers.c\
			dict.c types.c fopencookie.h fopencookie.c
hxnormalize_SOURCES =	hxnormalize.c html.y scan.l openurl.c url.c\
			tree.c connectsock.c heap.c dtd.c types.c\
			fopencookie.h langinfo.c\
			textwrap.c errexit.c headers.c dict.c fopencookie.c
hxnum_SOURCES =		hxnum.c html.y scan.l openurl.c url.c errexit.c\
			heap.c connectsock.c headers.c dict.c types.c class.c\
			fopencookie.h fopencookie.c
hxpipe_SOURCES =	hxpipe.c html.y scan.l types.c errexit.c heap.c\
			openurl.c url.c connectsock.c headers.c dict.c\
			fopencookie.h fopencookie.c
hxremove_SOURCES =	hxremove.c types.c errexit.c heap.c html.y scan.l\
			tree.c selector.c dtd.c selmatch.c
hxselect_SOURCES =	hxselect.c types.c errexit.c heap.c html.y scan.l\
			tree.c selector.c dtd.c selmatch.c
hxtabletrans_SOURCES =	hxtabletrans.c scan.l tree.c heap.c openurl.c html.y\
			errexit.c dtd.c types.c dict.c connectsock.c\
			headers.c url.c fopencookie.h fopencookie.c
hxtoc_SOURCES =		html.y scan.l dtd.c openurl.c errexit.c url.c class.c\
			connectsock.c heap.c tree.c types.c genid.c hxtoc.c\
			hash.c headers.c dict.c fopencookie.h fopencookie.c
hxuncdata_SOURCES =	hxuncdata.c
hxunent_SOURCES =	unent.c hxunentmain.c
hxunpipe_SOURCES =	hxunpipe.c heap.c errexit.c openurl.c url.c\
			fopencookie.h\
			connectsock.c headers.c dict.c types.c fopencookie.c
hxunxmlns_SOURCES =	hxunxmlns.c html.y scan.l openurl.c url.c\
			connectsock.c heap.c errexit.c types.c headers.c\
			dict.c fopencookie.h fopencookie.c
hxwls_SOURCES =		hxwls.c html.y scan.l openurl.c url.c\
			connectsock.c heap.c errexit.c types.c headers.c\
			dict.c fopencookie.h fopencookie.c unent.c
hxxmlns_SOURCES =	hxxmlns.c html.y scan.l openurl.c url.c\
			connectsock.c heap.c errexit.c types.c headers.c\
			dict.c fopencookie.h fopencookie.c
xml2asc_SOURCES =	xml2asc.c
hxref_SOURCES =		html.y scan.l dtd.c openurl.c errexit.c url.c\
			connectsock.c heap.c tree.c types.c genid.c hxref.c\
			hash.c headers.c dict.c fopencookie.h fopencookie.c
hxname2id_SOURCES =	html.y scan.l dtd.c openurl.c errexit.c url.c\
			connectsock.c heap.c tree.c types.c hxname2id.c\
			headers.c dict.c fopencookie.h fopencookie.c
hxcopy_SOURCES =	html.y scan.l types.c url.c openurl.c errexit.c\
			dict.c headers.c heap.c connectsock.c hxcopy.c\
			fopencookie.h fopencookie.c
hxnsxml_SOURCES =	hxnsxml.c html.y scan.l types.c errexit.c heap.c\
			openurl.c url.c connectsock.c headers.c dict.c\
			fopencookie.h fopencookie.c
hxprintlinks_SOURCES =	hxprintlinks.c openurl.c headers.c\
			types.c heap.c errexit.c\
			dict.c html.y scan.l url.c connectsock.c\
			fopencookie.h fopencookie.c

HTML_MANS =		$(man_MANS:.1=.html)
CLEANFILES =		$(HTML_MANS)

SUFFIX =		.c:sC .l:sC .y:sC .e:h .h:h
SUFFIXES =		.1 .e .html

$(EXPORTS): $(cexport_SOURCES)

.c.e:
	$(MAKE) cexport
	$(top_builddir)/cexport -I. -I$(srcdir) $(CPPFLAGS) -c "@CPP@" <$< >$@

.1.html:
	groff -mandoc -Txhtml $< >$@
#	man2html -r $< | sed -e '1,/^$$/d' >$@

dtd.c: dtd.hash
	gperf -a -c -C -o -t -p -T -k '1,2,$$' -N lookup_element $< >$@

unent.c: unent.hash
	gperf -a -c -C -o -t -p -k '1,2,$$' -D -N lookup_entity $< >$@

scan.o: html.h html.e scan.c

sel.tab.c sel.tab.h: sel.y
	bison -p sel_ -d -v sel.y

Overview.html: $(HTML_MANS)
	echo "<title>Manual pages</title><ul>" >$@
	for f in $(HTML_MANS); do\
	 echo "<li><a href=\""$$f"\">"`basename $$f .html`"</a>";\
	done | sort >>$@
	echo "</ul>" >>$@

# This is inconvenient. In automake version 1.11, $(wildcard) worked,
# but not in version 1.14. :-(
# TESTS = $(wildcard $(top_srcdir)/tests/*.sh)
TESTS = tests/addid1.sh tests/addid1.sh tests/ascxml.sh\
	tests/cdata1.sh tests/cite1.sh tests/cite2.sh tests/cite3.sh\
	tests/cite4.sh\
	tests/clean1.sh tests/copy1.sh tests/copy2.sh tests/copy3.sh\
	tests/copy4.sh tests/copy5.sh tests/copy6.sh\
	tests/copy7.sh tests/copy8.sh tests/extract1.sh\
	tests/hxnsxml1.sh tests/hxnsxml2.sh tests/hxnsxml3.sh\
	tests/hxnsxml4.sh tests/incl1.sh tests/incl10.sh\
	tests/incl11.sh tests/incl2.sh tests/incl3.sh tests/incl4.sh\
	tests/incl5.sh tests/incl6.sh tests/incl7.sh tests/incl8.sh\
	tests/incl9.sh tests/index.sh tests/index2.sh tests/index3.sh\
	tests/index4.sh tests/index5.sh\
	tests/index6.sh tests/index7.sh tests/index8.sh tests/index9.sh\
	tests/index10.sh\
	tests/mkbib1.sh tests/mkbib2.sh\
	tests/normalize1.sh tests/normalize2.sh\
	tests/normalize3.sh tests/normalize4.sh tests/normalize5.sh\
	tests/normalize6.sh tests/normalize7.sh tests/normalize8.sh\
	tests/normalize9.sh tests/normalize10.sh tests/normalize11.sh\
	tests/normalize12.sh tests/normalize13.sh tests/normalize14.sh\
	tests/num1.sh tests/num2.sh tests/num3.sh tests/num4.sh\
	tests/num5.sh tests/num6.sh\
	tests/pipe1.sh tests/pipe2.sh tests/pipe3.sh tests/pipe4.sh\
	tests/printlinks1.sh tests/printlinks2.sh tests/printlinks3.sh\
	tests/printlinks4.sh tests/ref1.sh\
	tests/ref2.sh tests/ref3.sh tests/relurl1.sh tests/relurl2.sh\
	tests/relurl3.sh tests/remove1.sh tests/remove2.sh\
	tests/remove3.sh tests/remove4.sh tests/remove5.sh tests/remove6.sh\
	tests/remove7.sh\
	tests/select01.sh tests/select02.sh tests/select03.sh\
	tests/select04.sh tests/select05.sh tests/select06.sh\
	tests/select07.sh tests/select08.sh tests/select09.sh\
	tests/select10.sh tests/select11.sh tests/select12.sh\
	tests/select13.sh tests/select14.sh tests/select15.sh\
	tests/select16.sh tests/select17.sh tests/select18.sh\
	tests/select19.sh tests/select20.sh tests/select21.sh\
	tests/select22.sh tests/select23.sh tests/select24.sh\
	tests/tabletrans1.sh tests/tabletrans2.sh tests/tabletrans3.sh\
	tests/tabletrans4.sh tests/tabletrans5.sh\
	tests/toc1.sh tests/toc2.sh tests/toc3.sh tests/toc4.sh\
	tests/toc5.sh tests/toc6.sh\
	tests/uncdata1.sh\
	tests/unent1.sh tests/unent2.sh tests/unent3.sh tests/unent4.sh\
	tests/unent5.sh tests/unent6.sh\
	tests/unpipe1.sh\
	tests/unpipe2.sh tests/unpipe3.sh tests/unpipe4.sh tests/unpipe5.sh\
	tests/unpipe6.sh\
	tests/wls1.sh tests/wls2.sh\
	tests/wls3.sh tests/wls4.sh tests/wls5.sh tests/wls6.sh\
	tests/xmlasc1.sh tests/xmlasc2.sh tests/xmlasc3.sh\
	tests/xmlasc4.sh tests/xmlasc5.sh tests/xmlasc6.sh\
	tests/xmlasc7.sh tests/xmlns1.sh tests/xref1.sh tests/xref2.sh\
	tests/xref3.sh tests/xref4.sh tests/xref5.sh tests/xref6.sh\
	tests/xref7.sh
