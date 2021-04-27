externsrc = $(wildcard src/extern/*.c)
externobj = $(externsrc:.c=.o)

extern: $(externobj)

$(extenobj): $(externsrc)

libbpf:
	cp src/extern/bpf/* src/extern/libbpf/src
	cp src/include/uapi/linux/* src/extern/libbpf/include/uapi/linux
	cp src/extern/linux/filter.h src/extern/libbpf/include/linux/filter.h
	sed -i 's/libbpf.h btf.h/libbpf.h libbpf_common.h btf.h/' src/extern/libbpf/src/Makefile
	DESTDIR=../../../../lib/ make install -C src/extern/libbpf/src

clean::
	rm -f src/extern/*.o
	rm -f src/extern/*.gcda
	rm -rf lib/*
	make clean -C src/extern/libbpf/src
