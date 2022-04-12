CC=gcc
LINK=-lm -lSDL2
CFLAGS=-Wall -Wextra -std=c99 -Wno-missing-braces -Isrc/

SRCDIR=.
MKDIR=mkdir -p
BUILDDIR=build
NAME=fy
RM=rm -f
RMDIR=rm -rf

OBJECTS=main.o

.PHONY: clean all debug

debug: CFLAGS+=-g
debug: clean $(BUILDDIR)/$(NAME)

all: CFLAGS+=-DNDEBUG
all: clean $(BUILDDIR)/$(NAME)

$(BUILDDIR):
	$(MKDIR) $@

$(BUILDDIR)/$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(BUILDDIR)/* -o $@ $(LINK)

%.o: %.c $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $(BUILDDIR)/$@ $<

clean:
	$(RMDIR) $(BUILDDIR)
