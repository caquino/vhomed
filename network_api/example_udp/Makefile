CC = cc
AR = ar rc
RM = rm -f

INCLUDE = -I. -I..
CFLAGS = -Wall -O2 $(INCLUDE)

OBJECTS = client server

all:	$(OBJECTS)

clean:
	$(RM) $(OBJECTS) $(NAME)

client:
	$(CC) $(CFLAGS) -o client client.c ../libnet_api.a
	
server:
	$(CC) $(CFLAGS) -o server server.c ../libnet_api.a
