install-data-local:
	for i in 0 1 2; do \
	  wu=scospub$${i}_wu; \
	  $(INSTALL_DATA) $${wu} $(PROJECT_DIR)/templates; \
	done
	for i in 0 1 2 3; do \
	  res=scospub$${i}_result; \
	  $(INSTALL_DATA) $${res} $(PROJECT_DIR)/templates; \
	done
	$(INSTALL_DATA) checkstyle_wu $(PROJECT_DIR)/templates
