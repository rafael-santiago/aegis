#
# Copyright (c) 2020, Rafael Santiago
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.
#
native_src_dir = $(shell uname -s | tr '[:upper:]' '[:lower:]')
main: mkdirs aegis_native.o
	@ar -r ../lib/libaegis.a o/aegis_native.o
	@echo info: ../lib/libaegis.a was built.
aegis_native.o: aegis.h $(native_src_dir)/aegis_native.c
	@cc -c $(native_src_dir)/aegis_native.c -I. -oo/aegis_native.o
mkdirs:
	$(shell mkdir o >/dev/null 2>&1)
	$(shell mkdir ../lib>/dev/null 2>&1)
clean:
	@rm o/*.o
	@rm ../lib/libaegis.a
	@echo info: clean.
