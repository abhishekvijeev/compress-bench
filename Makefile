# compiler
CC = g++

# binary name
APP = compress-bench

# all sources are stored in SRCS-y
SRCS-y := compress-bench.cc

PKGCONF ?= pkg-config
# PC_FILE := $(shell $(PKGCONF) --path libisal 2>/dev/null)
CFLAGS += -O3
CFLAGS += $(shell $(PKGCONF) --cflags libisal)
CFLAGS += $(shell $(PKGCONF) --cflags zlib)
CFLAGS += $(shell $(PKGCONF) --cflags libdeflate)

LDFLAGS = $(shell $(PKGCONF) --libs libisal)
LDFLAGS += $(shell $(PKGCONF) --libs zlib)
LDFLAGS += $(shell $(PKGCONF) --libs libdeflate)

# Build using pkg-config variables if possible
ifneq ($(shell $(PKGCONF) --exists libisal && echo 0),0)
$(error "no installation of Intel ISA-L found")
endif
ifneq ($(shell $(PKGCONF) --exists zlib && echo 0),0)
$(error "no installation of ZLIB found")
endif
ifneq ($(shell $(PKGCONF) --exists libdeflate && echo 0),0)
$(error "no installation of libdeflate found")
endif

all: build/$(APP)

build/$(APP): | build
	$(CC) $(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS)

build:
	@mkdir -p $@

.PHONY: clean
clean:
	rm -f build/$(APP)
	test -d build && rmdir -p build || true

