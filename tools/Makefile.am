FILES = \
	checkstyle-all-4.3.jar checkstyle-sun_checks.xml

TOOLDIR = $(PROJECT_DIR)/scospub/tools

install-data-local:
	for file in $(FILES); do \
	  cp $${file} `$(PROJECT_DIR)/bin/db_hier $${file}`; \
	done
