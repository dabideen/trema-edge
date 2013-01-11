#
# Copyright (C) 2012 NEC Corporation
#

CC = gcc
AR = ar
RANLIB = ranlib

CFLAGS = -I../../lib/ -g -std=gnu99 -D_GNU_SOURCE -fno-strict-aliasing -Wall -Werror -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith
LDFLAGS = -ltrema -lpthread -lsqlite3 -ldl -lrt

LIBDIR = ../../lib

TARGET_OFPGEN = pGen
SRCS_OFPGEN = pGen.c
OBJS_OFPGEN = $(SRCS_OFPGEN:.c=.o)

TARGETS = $(TARGET_OFPGEN)
SRCS = $(SRCS_OFPGEN)
OBJS = $(OBJS_OFPGEN)

DEPENDS = .depends

.PHONY: all clean

.SUFFIXES: .c .o

all: depend $(TARGETS)

$(TARGET_OFPGEN): $(OBJS_OFPGEN)
	$(CC) -L$(LIBDIR) -o $(TARGET_OFPGEN) $(OBJS_OFPGEN) $(LIBDIR)/libtrema.a $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

depend: 
	$(CC) -MM $(CFLAGS) $(SRCS) > $(DEPENDS)

clean:
	@rm -rf $(DEPENDS) $(OBJS) $(TARGETS) *~

-include $(DEPENDS)
