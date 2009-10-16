ifeq ($(origin SOLUTION_DIR), undefined)
	SOLUTION_DIR := $(shell pwd)/
endif
PROJECT_DIR_LIBDWARF := $(dir $(lastword $(MAKEFILE_LIST)))
include $(PROJECT_DIR_LIBDWARF)/../libelf/Unix/Makefile
CFLAGS_DWARF = -I../libelf/lib -D__LIBELF_INTERNAL__

$(PROJECT_DIR_LIBDWARF)/../libdwarf/config.h : libelf
	cd $(PROJECT_DIR_LIBDWARF)/../libdwarf && ./configure "CFLAGS=$(CFLAGS_DWARF)" "CPPFLAGS=$(CFLAGS_DWARF)"

clean::
	rm -f $(PROJECT_DIR_LIBDWARF)/../libdwarf/config.h
	rm -f libdwarf.so*
	rm -f libdwarf

libdwarf : $(PROJECT_DIR_LIBDWARF)/../libdwarf/config.h
	cd $(PROJECT_DIR_LIBDWARF)/../libdwarf && make libdwarf.a libdwarf.so
	cp $(PROJECT_DIR_LIBDWARF)/../libdwarf/libdwarf.so* .
	cd $(PROJECT_DIR_LIBDWARF)/../libdwarf && make -i clean
	touch libdwarf

