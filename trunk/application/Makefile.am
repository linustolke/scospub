RUNAPP=scospubapp
APPDIR = $(PROJECT_DIR)/apps/$(RUNAPP)
APPVERDIR = $(APPDIR)/$(RUNAPP)_$(VERSION)_$(host)

noinst_PROGRAMS = scospubapp

install-data-local:
	mkdir -p $(APPVERDIR)
	$(INSTALL_PROGRAM) scospubapp $(APPVERDIR)/scospubapp_$(VERSION)_$(host)

AM_CPPFLAGS = \
    -fno-inline \
    -I$(BOINC_DIR)/api \
    -I$(BOINC_DIR)/lib

scospubapp_LDFLAGS = \
    -pthread

scospubapp_SOURCES = scospubapp.C scospub_parse.C

scospubapp_LDADD = \
    -L$(BOINC_DIR)/api \
    -lboinc_api \
    -L$(BOINC_DIR)/lib \
    -lboinc \
    libstdc++.a
