FILES = svn java checkstyle-all-4.3.jar

TOOL = checkstyle

TOOLDIR = $(PROJECT_DIR)/scospub/$(TOOL)

install-data-local:
	mkdir -p $(TOOLDIR)/$(TOOL)_$(host)
	$(INSTALL_DATA) wu $(TOOLDIR)/$(TOOL)_wu
	$(INSTALL_DATA) result $(TOOLDIR)/$(TOOL)_result

	$(INSTALL_DATA) job.xml $(TOOLDIR)/$(TOOL)_$(host)/job.xml=job_$(VERSION).xml
	for file in $(FILES); do \
	  cp $${file} $(TOOLDIR)/$(TOOL)_$(host)/$${file}=$${file}_$(VERSION)_$(host); \
	done
	$(INSTALL_PROGRAM) ../../application/wrapper $(TOOLDIR)/$(TOOL)_$(host)/wrapper_$(VERSION)_$(host)
