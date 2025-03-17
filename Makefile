#
# Copyright (c) 2013 No Face Press, LLC
# License http://opensource.org/licenses/mit-license.php MIT License
#


PROG = liteWMS
OBJS = liteWMS.o

OBJS := $(addprefix obj/,$(OBJS))

CFLAGS =  -DUSE_WEBSOCKET -DUSE_IPV6 -DNO_SSL
LIBS = -lpthread -ldl -lrt


all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS) 

obj/liteWMS.o: src/liteWMS.cpp
	$(CC) $< -c -o $@ $(CFLAGS) -Iwebview2

obj/%.o: src/%.c
	$(CC) $< -c -o $@ $(CFLAGS)


test.o: test.c
	$(CC) $< -c -o $@ $(CFLAGS)

civetweb.o: civetweb.c
	$(CC) $< -c -o $@  $(CFLAGS)

clean:
	rm -f $(CIVETWEB_LIB) $(PROG)


.PHONY: all clean
