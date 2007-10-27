bin_PROGRAMS = wrapper
wrapper_SOURCES = wrapper.C

CXXFLAGS = -I$(BOINC_DIR) \
    -I$(BOINC_API_DIR) \
    -I$(BOINC_LIB_DIR)

wrapper_LDFLAGS = \
    libstdc++.a \
    -pthread

wrapper_LDADD = \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -lboinc_api \
    -lboinc

