#
# Copyright (c) 2020, Rafael Santiago
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.
#
native_src_dir = $(shell uname -s | tr '[:upper:]' '[:lower:]')
ifeq ($(native_src_dir),linux)
    aegis_gorgon_dir=pthread
else ifeq ($(native_src_dir),freebsd)
    aegis_gorgon_dir=pthread
else ifeq ($(native_src_dir),netbsd)
    aegis_gorgon_dir=pthread
else ifeq ($(native_src_dir),openbsd)
    aegis_gorgon_dir=pthread
endif
main: mkdirs aegis_native.o aegis_gorgon.o
	@ar -r ../lib/libaegis.a o/aegis_native.o o/aegis_gorgon.o
	@echo info: ../lib/libaegis.a was built.
aegis_native.o: aegis.h $(native_src_dir)/aegis_native.c
	@cc -c $(native_src_dir)/aegis_native.c -I. -oo/aegis_native.o
aegis_gorgon.o: $(aegis_gorgon_dir)/aegis_gorgon.c
	@cc -c $(aegis_gorgon_dir)/aegis_gorgon.c -I. -oo/aegis_gorgon.o
mkdirs:
	$(shell mkdir o >/dev/null 2>&1)
	$(shell mkdir ../lib>/dev/null 2>&1)
clean:
	@rm o/*.o
	@rm ../lib/libaegis.a
	@echo info: clean.
