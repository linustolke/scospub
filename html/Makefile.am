SUBDIRS = languages scosconf scospres

APACHECFG = /etc/apache2/sites-available/scospub.httpd.conf

install-data-local:
	@if test -f $(APACHECFG) && cmp scospub.httpd.conf $(APACHECFG); \
	then \
	    : This is already installed.; \
	else \
	    echo On Debian with Apache2, as root:; \
	    echo copy scospub.httpd.conf to /etc/apache2/sites-available; \
	    echo and link to /etc/apache2/sites-enabled; \
	fi
