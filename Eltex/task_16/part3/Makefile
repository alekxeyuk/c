SUBDIRS = $(wildcard */.)

all: $(SUBDIRS)
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir all || true; \
	done


clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean || true; \
	done

.PHONY: all clean