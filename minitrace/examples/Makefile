APPS = \
	get_started \
	synchronous \
	#

.PHONY: all
all: $(APPS)

clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(APPS) *.o

RPATH=$(PWD)/../../target/debug
CFLAGS = -I../include -g -O2
LDFLAGS = -L$(RPATH) -Wl,-rpath,$(RPATH)
LIBS = -lminitrace_c

$(APPS): %: %.o
	$(call msg,BINARY,$@)
	$(Q)$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@
