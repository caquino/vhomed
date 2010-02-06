CC = cc
AR = ar rc
RM = rm -f
MAKE = make

NAME = libnet_api.a

INCLUDE = -I.
CFLAGS = -g -Wall -O2 $(INCLUDE)

OBJECTS = net_udp.o net_tcp.o net_domain.o

.SUFFIXES: .c
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	$(OBJECTS)
	$(RM) $(NAME)
	$(AR) $(NAME) $(OBJECTS)

	$(MAKE) -C example_udp
	$(MAKE) -C example_tcp
	$(MAKE) -C example_domain

clean:
	$(RM) $(OBJECTS) $(NAME)
	$(MAKE) -C example_udp clean
	$(MAKE) -C example_tcp clean
	$(MAKE) -C example_domain clean
