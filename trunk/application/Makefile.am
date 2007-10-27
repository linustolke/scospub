bin_PROGRAMS = wrapper
wrapper_SOURCES = wrapper.C
wrapper_EXTRALIBS = stdc++ boinc_api boinc

CXXFLAGS = -I$(BOINC_DIR) \
    -I$(BOINC_API_DIR) \
    -I$(BOINC_LIB_DIR)

wrapper_LDFLAGS = \
    -pthread

wrapper_LDADD = \
    -lstdc++ \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -lboinc_api \
    -lboinc

