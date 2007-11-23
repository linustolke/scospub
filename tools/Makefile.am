FILES = \
	checkstyle-all-4.3.jar checkstyle-sun_checks.xml

TOOLDIR = $(PROJECT_DIR)/scospub/tools

install-data-local:
	cwd=`cd $(srcdir) && pwd`; \
	for file in $(FILES); do \
	  cd $(PROJECT_DIR) && cp $${cwd}/$${file} `bin/dir_hier_path $${file}`; \
	done
