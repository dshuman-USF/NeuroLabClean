

AM_CFLAGS = -ggdb3 -Wall $(DAQ_CFLAGS) -I$(top_srcdir)  \
-DUI_DIR='"$(pkgdatadir)"' -DG_ERRORCHECK_MUTEXES --std=gnu99


bin_SCRIPTS = clean_rec.sh do_clean_data.sh do_noclean_data.sh make_chan.sh \
				  	 make_label.sh split_all.sh do_clean_data2.m CleanData.m

bin_PROGRAMS = daq2_split daq2_unsplit chans_to_bin daq_to_bin

EXTRA_DIST =  $(bin_SCRIPTS) debian

dist_doc_DATA = Daq2CleanUsersManual.doc Daq2CleanUsersManual.odt Daq2CleanUsersManual.txt Daq2CleanUsersManual.pdf LICENSE COPYING COPYRIGHTS ChangeLog

icondir = $(datadir)/icons/hicolor/48x48/apps
dist_icon_DATA = daq.png

daq2_split_SOURCES = daq2_split.c
daq2_unsplit_SOURCES = daq2_unsplit.c
chans_to_bin_SOURCES = chans_to_bin.c
daq_to_bin_SOURCES = daq_to_bin.c

AM_LDFLAGS = -export-dynamic

checkin_files = $(EXTRA_DIST) $(daq2_split_SOURCES) $(daq2_unsplit_SOURCES) $(chans_to_bin_SOURCES) $(daq_to_bin_SOURCES) $(dist_doc_DATA) $(dist_icon_DATA) Makefile.am configure.ac 

checkin_release:
	git add $(checkin_files) && git commit -uno -S -m "Release files for version $(VERSION)"

checkpoint:
	git add $(checkin_files) && git commit -uno -S -q -m "checkpoint files for version $(VERSION)"

checkpoint_withcomment:
	git add $(checkin_files) && git commit -uno -S -q

files: $(daq2_split_SOURCES) $(bin_SCRIPTS) Makefile.am
	ls $(daq2_split_SOURCES) $(bin_SCRIPTS) | sort -u > files

deb:
	@echo 'Making debian packages'
	make distdir &&\
	cd $(distdir) &&\
	dpkg-buildpackage -Zgzip -uc -us && \
	cd .. && \
	rm -rf $(distdir)

