CC ?= gcc
RM := rm -f
CP := cp
MV := mv
SHELL := bash

ifeq ($(OS),Windows_NT)
USR := mingw64
LOCALE := LC_ALL=C LANG=C
else
USR := usr
LOCALE :=
endif


BASE_FLAGS := -std=gnu23 -Wall -flto -Iinclude -D_GNU_SOURCE

ifeq ($(VALGRIND),1)
ifeq ($(OS),Windows_NT)
CC := clang
BASE_FLAGS += -fsanitize=address,undefined
else
VALGRIND_CALL := valgrind --leak-check=full --show-leak-kinds=all \
	--suppressions=../val_sup.txt --log-file="log.txt"
endif
endif

ifeq ($(KCACHEGRIND),1)
VALGRIND_CALL := valgrind --tool=callgrind
endif

ifeq ($(RELEASE),2)
BASE_FLAGS += -O3 -DNDEBUG -march=native -mtune=native
else
ifeq ($(RELEASE),1)
BASE_FLAGS += -O3 -DNDEBUG
else
BASE_FLAGS += -O0 -g3 -ggdb -D_FORTIFY_SOURCE=3
endif
endif

ifneq ($(OS),Windows_NT)
BASE_FLAGS += -rdynamic
endif


BASE_LD_FLAGS := -lm

ifneq ($(OS),Windows_NT)
BASE_LD_FLAGS += -pthread
endif


FREETYPE_LD_FLAGS := -I/$(USR)/include/freetype2 -lfreetype


SHARED_FLAGS := -Wno-address-of-packed-member
SHARED_NOLIB_FLAGS := -fwhole-program
SHARED_LD_FLAGS :=


CLIENT_FLAGS := $(SHARED_FLAGS) $(SHARED_NOLIB_FLAGS)
CLIENT_LD_FLAGS := $(SHARED_LD_FLAGS) $(FREETYPE_LD_FLAGS) -lm -lSDL3 -lharfbuzz -lzstd -lutf8proc

ifeq ($(OS),Windows_NT)
CLIENT_LD_FLAGS += -lvulkan-1 -lws2_32
FILE_EXT := .exe
else
CLIENT_LD_FLAGS += -lvulkan
endif


SERVER_FLAGS := $(SHARED_FLAGS) $(SHARED_NOLIB_FLAGS)
SERVER_LD_FLAGS := $(SHARED_LD_FLAGS)


TEX_FLAGS := -fwhole-program
TEX_LD_FLAGS := -Lbin -lshared


SHARED_LIB_FLAGS := -shared
SHARED_LIB_LD_FLAGS :=

ifeq ($(OS),Windows_NT)
SHARED_LIB_FLAGS += -Wl,--out-implib,bin/libshared.lib
endif


SHARED_FILES := alloc base debug event file threads rand \
	bit_buffer hash time
CLIENT_FILES := tex/base tex/font ui/base ui/container \
	ui/checkbox color dds main volk window settings base64
SERVER_FILES := main quadtree sort

SHARED_FILES := $(SHARED_FILES:%=src/shared/%.c)
CLIENT_FILES := $(CLIENT_FILES:%=src/client/%.c) $(SHARED_FILES)
SERVER_FILES := $(SERVER_FILES:%=src/server/%.c) $(SHARED_FILES)


TEX_DIRS := $(wildcard tex/img/[0-9]*/)
TEX_FILES := $(TEX_DIRS:tex/img/%/=tex/dds/%.dds)

SRC_TEX_DIRS := src/client/tex include/DiepDesktop/client/tex


SHARED_CALL := LD_LIBRARY_PATH=bin:$$LD_LIBRARY_PATH


.PHONY: all
all:
	@printf "Specify one (or more) of the following:\n\
	\n\
	font_build  generates font textures and sources\n\
	font_gen    generates font sources from textures\n\
	font_clean  removes font textures\n\
	font_wipe   above + sources\n\
	\n\
	var_build   generates variable textures\n\
	var_clean   removes variable textures\n\
	\n\
	tex_build   combines all textures\n\
	tex_gen     generates texture sources\n\
	tex_clean   removes all combined textures\n\
	tex_wipe    above + sources\n\
	\n\
	dds_build   packs and compresses combined textures\n\
	dds_clean   removes raw dds files\n\
	dds_wipe    above + compressed dds files\n\
	\n\
	tex_reset   does all of the above to fulfill client prerequisites\n\
	\n\
	client      generates the client, requires dds and all sources\n\
	server      generates the server, no prerequisites\n\
	\n\
	clean       removes any built executables\n\
	wipe        above + all textures + all sources (= everything)\n\
	\n\
	You can use the clean targets to remove intermediate files.\n\
	Valid use case:\n\
		make font_build tex_build font_clean   # <- tex_build uses font\n\
	Invalid use case:\n\
		make font_build font_clean             # <- destroyed without using it\n\
	Not all commands are compatible with each other in a chain like above.\n\
	Unless you deeply know how they work, use one command per a \`make\` call.\n\
	\n\
	Specify RELEASE=1 for a production build\n\
	Specify RELEASE=2 for a native build (faster than production but not portable)\n\
	You can't mix various build types, if you change it you gotta \`make clean\` first\n"


bin tex/font tex/var tex/img tex/dds_raw tex/dds_bc1 tex/dds $(SRC_TEX_DIRS):
	mkdir -p $@

.PHONY: clean
clean:
	$(RM) -r bin/

.PHONY: wipe
wipe: clean font_wipe var_clean tex_wipe dds_wipe


bin/libshared.a: $(SHARED_FILES) | bin
	for file in $^; do \
		$(CC) $(BASE_FLAGS) $(SHARED_FLAGS) $(SHARED_LIB_FLAGS) -c -o bin/$$(basename $${file%.*}).o \
			$$file $(BASE_LD_FLAGS) $(SHARED_LD_FLAGS) $(SHARED_LIB_LD_FLAGS); \
	done
	$(AR) rcs $@ bin/*.o

bin/font_gen: tex/font_gen.c | bin/libshared.a
	$(CC) $(BASE_FLAGS) $(TEX_FLAGS) -o $@ $^ $(BASE_LD_FLAGS) $(TEX_LD_FLAGS) $(FREETYPE_LD_FLAGS)

bin/sort: tex/sort.c | bin/libshared.a
	$(CC) $(BASE_FLAGS) $(TEX_FLAGS) -o $@ $^ $(BASE_LD_FLAGS) $(TEX_LD_FLAGS) -lpng

bin/tex_gen: tex/tex_gen.c | bin/libshared.a
	$(CC) $(BASE_FLAGS) $(TEX_FLAGS) -o $@ $^ $(BASE_LD_FLAGS) $(TEX_LD_FLAGS)


.PHONY: font_build
font_build: bin/font_gen | tex/font $(SRC_TEX_DIRS)
	$(RM) -r tex/font/*
	$(SHARED_CALL) WRITE_IMG=1 ./bin/font_gen

.PHONY: font_gen
font_gen: bin/font_gen | tex/font $(SRC_TEX_DIRS)
	$(SHARED_CALL) ./bin/font_gen

.PHONY: font_clean
font_clean:
	$(RM) -r tex/font/

.PHONY: font_wipe
font_wipe: font_clean
	$(RM) -r include/DiepDesktop/client/tex/font.h src/client/tex/font.c


.PHONY: var_build
var_build: | tex/var
	$(RM) -r tex/var/*
	python3 tex/bg_gen.py; \
	python3 tex/cs_gen.py; \
	python3 tex/hue_gen.py; \
	python3 tex/rect_gen.py; \
	python3 tex/text_cursor_gen.py

.PHONY: var_clean
var_clean:
	$(RM) -r tex/var/


.PHONY: tex_build
tex_build: bin/sort | tex/img
	$(RM) -r tex/img/*
	$(CP) -r tex/const/* tex/img/
	$(CP) -r tex/font/* tex/img/
	$(CP) -r tex/var/* tex/img/
	$(SHARED_CALL) ./bin/sort

.PHONY: tex_gen
tex_gen: bin/tex_gen | tex/img $(SRC_TEX_DIRS)
	$(SHARED_CALL) ./bin/tex_gen
	TEX_NUM=$$(grep -oP '#define TEXTURES_NUM \K\d+' include/DiepDesktop/client/tex/base.h); \
	sed -i "s/inTex\[[0-9]\{1,\}\];/inTex[$$TEX_NUM];/g" shaders/frag.glsl

.PHONY: tex_clean
tex_clean:
	$(RM) -r tex/img/

.PHONY: tex_wipe
tex_wipe: tex_clean
	$(RM) -r include/DiepDesktop/client/tex/base.h src/client/tex/base.c \
		include/DiepDesktop/client/tex/tex_*.h


.PRECIOUS: tex/dds_raw/%.dds
tex/dds_raw/%.dds: tex/img/% | tex/dds_raw
	pngs=$$(ls -1 "$<" | wc -l); \
	if [ "$$pngs" -eq 1 ]; then \
		$(CP) $</* $(shell dirname $@)/$*.png; \
		$(LOCALE) texconv -w $* -h $* -m 1 -f BGRA -o \
			$(shell dirname $@) -l -y -- $(shell dirname $@)/$*.png; \
	else \
		$(LOCALE) texassemble array -w $* -h $* -f BGRA -o $@ -l -y -stripmips -- $</*.png; \
	fi

.PRECIOUS: tex/dds_bc1/%.dds
tex/dds_bc1/%.dds: tex/dds_raw/%.dds | tex/dds_bc1
	$(LOCALE) texconv -w $* -h $* -m 1 -f BC1_UNORM_SRGB -o $(shell dirname $@) -l -y -- $<

.PRECIOUS: tex/dds/%.dds
tex/dds/%.dds: tex/dds_bc1/%.dds | tex/dds
	-zstd -T0 --ultra -20 -o $@ $<

.PHONY: dds_build
dds_build: $(TEX_FILES)

.PHONY: dds_clean
dds_clean:
	$(RM) -r tex/dds_raw/ tex/dds_bc1/

.PHONY: dds_wipe
dds_wipe: dds_clean
	$(RM) -r tex/dds/


.PHONY: tex_reset
tex_reset:
	$(MAKE) wipe
	RELEASE=2 $(MAKE) font_build var_build tex_build tex_gen
	$(MAKE) dds_build


bin/%.spv: shaders/%.glsl | bin
	glslc -O -fshader-stage=$* $< -o $@

.PHONY: shaders
shaders: bin/vert.spv bin/frag.spv | bin

.PHONY: client
client: shaders dds_build | bin
	$(CC) $(BASE_FLAGS) $(CLIENT_FLAGS) -o bin/client$(FILE_EXT) $(CLIENT_FILES) $(BASE_LD_FLAGS) $(CLIENT_LD_FLAGS)

	$(RM) -r DiepDesktop
	mkdir DiepDesktop

	$(CP) bin/client$(FILE_EXT) DiepDesktop/
	$(CP) bin/*.spv DiepDesktop/
	$(CP) tex/dds/* DiepDesktop/
	$(CP) tex/Ubuntu.ttf DiepDesktop/

ifeq ($(OS),Windows_NT)
	$(CP) "C:\\msys64\\mingw64\\bin\\SDL3.dll" DiepDesktop/
	$(CP) "C:\\msys64\\mingw64\\bin\\libwinpthread-1.dll" DiepDesktop/
else
	$(CP) \
		$$(ls -1 /usr/lib/x86_64-linux-gnu/libvulkan.so.1.3.* | sort -V | tail -n 1) \
		DiepDesktop/libvulkan.so.1
	$(CP) \
		$$(ls -1 /usr/local/lib/libSDL3.so.0.1.* | sort -V | tail -n 1) \
		DiepDesktop/
endif

	cd DiepDesktop; $(VALGRIND_CALL) ./client$(FILE_EXT)


.PHONY: server
server: | bin
	$(CC) $(BASE_FLAGS) $(SERVER_FLAGS) -o bin/server$(FILE_EXT) $(SERVER_FILES) $(BASE_LD_FLAGS) $(SERVER_LD_FLAGS)
