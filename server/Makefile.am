MYSQL_CXXFLAGS = @MYSQL_CFLAGS@
MYSQL_LDFLAGS = @MYSQL_LIBS@

noinst_PROGRAMS = \
	scospub_work_generator \
	scospub_assimilator

install-data-local:
	$(INSTALL_PROGRAM) scospub_work_generator $(PROJECT_DIR)/bin
	$(INSTALL_PROGRAM) scospub_assimilator $(PROJECT_DIR)/bin

AM_CPPFLAGS = \
    -fno-inline \
    -I../application \
    -I$(BOINC_DIR)/api \
    -I$(BOINC_DIR)/lib \
    -I$(BOINC_DIR)/db \
    -I$(BOINC_DIR)/tools \
    -I$(BOINC_DIR)/sched \
    $(MYSQL_CXXFLAGS)

BOINC_LIB = \
    -L$(BOINC_DIR)/sched \
    -lsched \
    $(MYSQL_LDFLAGS) \
    -L$(BOINC_DIR)/lib \
    -lboinc

scospub_work_generator_SOURCES = scospub_work_generator.C scospub_db.C

scospub_work_generator_LDADD = \
    $(BOINC_LIB) \
    $(PTHREAD_LIBS) \
    -lcrypto \
    libstdc++.a

scospub_assimilator_SOURCES = scospub_assimilator.C scospub_db.C

scospub_assimilator_LDADD = \
    $(BOINC_DIR)/sched/assimilator.o \
    $(BOINC_DIR)/sched/validate_util.o \
    $(BOINC_LIB) \
    libstdc++.a

