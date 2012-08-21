C = gcc

REQUIRED_PKG = calendar-service
CFLAGS = -g -Wall
LDFLAGS = # -L../ -lefence -pthread
ifdef REQUIRED_PKG
	CFLAGS += `pkg-config --cflags $(REQUIRED_PKG)`
	LDFLAGS += `pkg-config --libs $(REQUIRED_PKG)`
endif

SRCS = recur-add.c eve-add.c recur-add.c eve-see.c
TIMESRC = timetest.c
OBJECTS = $(SRCS:.c=.o)
TIMEOBJ = $(TIMESRC:.c=.o)
TARGETS = $(OBJECTS:.o=)
#A:.c=.o  //A안에 있는 .c를 .o로 바꿔라


all: $(OBJECTS) $(TARGETS)
#-mv test1 testlocal /usr/

$(TARGETS): $(TIMEOBJ)
$(TIMEOBJ): timetest.h

% : %.o
	$(CC) -o $@ $< $(TIMEOBJ) $(LDFLAGS)

clean:
	rm -rf $(OBJECTS) $(TARGETS) $(TIMEOBJ)

