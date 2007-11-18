noinst_PROGRAMS = wrapper scospub_work_generator
install-data-local:
	$(INSTALL_PROGRAM) scospub_work_generator $(PROJECT_DIR)/bin

wrapper_SOURCES = wrapper.C

AM_CPPFLAGS = \
    -I$(BOINC_DIR) \
    -I$(BOINC_DIR)/api \
    -I$(BOINC_DIR)/lib \
    -I$(BOINC_DIR)/db \
    -I$(BOINC_DIR)/tools \
    -I$(BOINC_DIR)/sched

BOINC_LIB = \
    -L$(BOINC_DIR)/lib \
    -lboinc

wrapper_LFLAGS = \
    -pthread

wrapper_LDADD = \
    -L$(BOINC_DIR)/api \
    -lboinc_api \
    $(BOINC_LIB) \
    libstdc++.a

scospub_work_generator_LDADD = \
    -L$(BOINC_DIR)/sched \
    -lsched \
    $(MYSQL_LIBS) \
    $(BOINC_LIB) \
    $(PTHREAD_LIBS) \
    -lcrypto \
    libstdc++.a


