FILES = \
	checkstyle-all-4.3.jar checkstyle-sun_checks.xml \
	findbugs-0.7.4.jar \
	findbugs-1.3.1.jar

TOOLDIR = $(PROJECT_DIR)/scospub/tools

install-data-local:
	cwd=`cd $(srcdir) && pwd`; \
	for file in $(FILES); do \
	  cd $(PROJECT_DIR) && ( echo "cp $${cwd}/$${file} download/$${file}" | sh -x); \
	done
