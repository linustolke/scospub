FILES = svn java checkstyle-java

RUNAPP=wrapper
APPDIR = $(PROJECT_DIR)/apps/$(RUNAPP)
APPVERDIR = $(APPDIR)/$(RUNAPP)_$(VERSION)_$(host)

noinst_PROGRAMS = wrapper scospub_work_generator scospub_assimilator
install-data-local:
	$(INSTALL_PROGRAM) scospub_work_generator $(PROJECT_DIR)/bin
	$(INSTALL_PROGRAM) scospub_assimilator $(PROJECT_DIR)/bin
	mkdir -p $(APPVERDIR)
	for file in $(FILES); do \
	  $(INSTALL_SCRIPT) $${file} $(APPVERDIR)/$${file}=$${file}_$(VERSION)_$(host); \
	done
	$(INSTALL_PROGRAM) wrapper $(APPVERDIR)/wrapper_$(VERSION)_$(host)

AM_CPPFLAGS = \
    -fno-inline \
    -I$(BOINC_DIR) \
    -I$(BOINC_DIR)/api \
    -I$(BOINC_DIR)/lib \
    -I$(BOINC_DIR)/db \
    -I$(BOINC_DIR)/tools \
    -I$(BOINC_DIR)/sched

BOINC_LIB = \
    -L$(BOINC_DIR)/lib \
    -lboinc

wrapper_SOURCES = wrapper.C

wrapper_LFLAGS = \
    -pthread

wrapper_LDADD = \
    -L$(BOINC_DIR)/api \
    -lboinc_api \
    $(BOINC_LIB) \
    libstdc++.a

scospub_work_generator_SOURCES = scospub_work_generator.C scospub_db.C

scospub_work_generator_LDADD = \
    -L$(BOINC_DIR)/sched \
    -lsched \
    $(MYSQL_LIBS) \
    $(BOINC_LIB) \
    $(PTHREAD_LIBS) \
    -lcrypto \
    libstdc++.a

scospub_assimilator_SOURCES = scospub_assimilator.C scospub_db.C

scospub_assimilator_LDADD = \
    $(BOINC_DIR)/sched/assimilator.o \
    $(BOINC_DIR)/sched/validate_util.o \
    -L$(BOINC_DIR)/sched \
    -lsched \
    $(BOINC_LIB) \
    libstdc++.a

