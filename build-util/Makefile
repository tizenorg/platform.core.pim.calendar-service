CC = gcc

REQUIRED_PKG =
CFLAGS = -g -Wall #-fprofile-arcs -ftest-coverage
LDFLAGS =
ifdef REQUIRED_PKG
	CFLAGS += $(shell pkg-config --cflags $(REQUIRED_PKG))
	LDFLAGS += -L$(pkg-config --libs $(REQUIRED_PKG))
	LDFLAGS += $(shell pkg-config --libs $(REQUIRED_PKG))
endif

SRCS = $(wildcard *.c)
OBJECTS = $(SRCS:.c=.o)
TARGETS = $(OBJECTS:.o=)

all: $(TARGETS)

$(TARGETS) : $(SRCS)
	$(CC) $(CFLAGS) $<  $(LDFLAGS)  -o $@

clean:
	rm -rf $(OBJECTS) $(TARGETS)

