FILES = svn java checkstyle-all-4.3.jar

TOOL = checkstyle

TOOLDIR = $(PROJECT_DIR)/scospub/$(TOOL)

install-data-local:
	mkdir -p $(TOOLDIR)/$(TOOL)_$(host)
	cp wu $(TOOLDIR)/$(TOOL)_wu
	cp result $(TOOLDIR)/$(TOOL)_result

	cp job.xml $(TOOLDIR)/$(TOOL)_$(host)/job.xml=job_$(VERSION).xml
	for file in $(FILES); do \
	  cp $${file} $(TOOLDIR)/$(TOOL)_$(host)/$${file}=$${file}_$(VERSION)_$(host); \
	done
	cp ../../application/wrapper $(TOOLDIR)/$(TOOL)_$(host)/wrapper_$(VERSION)_$(host)
