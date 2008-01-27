FILES = svn java checkstyle-java

MYSQL_CXXFLAGS = @MYSQL_CXXFLAGS@
MYSQL_LDFLAGS = @MYSQL_LDFLAGS@

RUNAPP=scospubapp
APPDIR = $(PROJECT_DIR)/apps/$(RUNAPP)
APPVERDIR = $(APPDIR)/$(RUNAPP)_$(VERSION)_$(host)

noinst_PROGRAMS = wrapper \
	scospubapp \
	scospub_work_generator \
	scospub_assimilator

install-data-local:
	$(INSTALL_PROGRAM) scospub_work_generator $(PROJECT_DIR)/bin
	$(INSTALL_PROGRAM) scospub_assimilator $(PROJECT_DIR)/bin
	mkdir -p $(APPVERDIR)
	$(INSTALL_PROGRAM) scospubapp $(APPVERDIR)/scospubapp_$(VERSION)_$(host)

AM_CPPFLAGS = \
    -fno-inline \
    -I$(BOINC_DIR)/api \
    -I$(BOINC_DIR)/lib

SERVER_SIDE_CPPFLAGS = \
    $(AM_CPPFLAGS) \
    -I$(BOINC_DIR)/db \
    -I$(BOINC_DIR)/tools \
    -I$(BOINC_DIR)/sched \
    $(MYSQL_CXXFLAGS)

BOINC_LIB = \
    -L$(BOINC_DIR)/lib \
    -lboinc

wrapper_LDFLAGS = \
    -pthread

wrapper_SOURCES = wrapper.C

wrapper_LDADD = \
    -L$(BOINC_DIR)/api \
    -lboinc_api \
    $(BOINC_LIB) \
    libstdc++.a

scospubapp_LDFLAGS = \
    -pthread

scospubapp_SOURCES = scospubapp.C scospub_parse.C

scospubapp_LDADD = \
    -L$(BOINC_DIR)/api \
    -lboinc_api \
    $(BOINC_LIB) \
    libstdc++.a

scospub_work_generator_SOURCES = scospub_work_generator.C scospub_db.C
scospub_work_generator_CPPFLAGS = $(SERVER_SIDE_CPPFLAGS)

scospub_work_generator_LDADD = \
    -L$(BOINC_DIR)/sched \
    -lsched \
    $(MYSQL_LDFLAGS) \
    $(BOINC_LIB) \
    $(PTHREAD_LIBS) \
    -lcrypto \
    libstdc++.a

scospub_assimilator_SOURCES = scospub_assimilator.C scospub_db.C
scospub_assimilator_CPPFLAGS = $(SERVER_SIDE_CPPFLAGS)

scospub_assimilator_LDADD = \
    $(BOINC_DIR)/sched/assimilator.o \
    $(BOINC_DIR)/sched/validate_util.o \
    -L$(BOINC_DIR)/sched \
    -lsched \
    $(MYSQL_LDFLAGS) \
    $(BOINC_LIB) \
    libstdc++.a

