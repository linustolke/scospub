install-data-local:
	for i in 0 1 2; do \
	  wu=scospub$${i}_wu; \
	  res=scospub$${i}_result; \
	  $(INSTALL_DATA) $${wu} $(PROJECT_DIR)/templates; \
	  $(INSTALL_DATA) $${res} $(PROJECT_DIR)/templates; \
	done