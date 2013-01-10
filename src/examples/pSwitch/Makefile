#
# Copyright (C) 2012 NEC Corporation
#

CC = gcc
AR = ar
RANLIB = ranlib

CFLAGS = -I../../lib/ -g -std=gnu99 -D_GNU_SOURCE -fno-strict-aliasing -Wall -Werror -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith
LDFLAGS = -ltrema -lpthread -lsqlite3 -ldl -lrt

LIBDIR = ../../lib

TARGET_PSWITCH = pSwitch
SRCS_PSWITCH = pSwitch.c
OBJS_PSWITCH = $(SRCS_PSWITCH:.c=.o)

TARGETS = $(TARGET_PSWITCH)
SRCS = $(SRCS_PSWITCH)
OBJS = $(OBJS_PSWITCH)

DEPENDS = .depends

.PHONY: all clean

.SUFFIXES: .c .o

all: depend $(TARGETS)

$(TARGET_PSWITCH): $(OBJS_PSWITCH)
	$(CC) -L$(LIBDIR) -o $(TARGET_PSWITCH) $(OBJS_PSWITCH) $(LIBDIR)/libtrema.a $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

depend: 
	$(CC) -MM $(CFLAGS) $(SRCS) > $(DEPENDS)

clean:
	@rm -rf $(DEPENDS) $(OBJS) $(TARGETS) *~

-include $(DEPENDS)
