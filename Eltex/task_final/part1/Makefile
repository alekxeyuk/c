SUBDIRS = $(wildcard */.)

all: $(SUBDIRS)
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@ || true; \
	done

install: all $(SUBDIRS)
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@ || true; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@ || true; \
	done

.PHONY: all clean