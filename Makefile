##
##  Makefile -- Build procedure for sample vhome Apache module
##  Autogenerated via ``apxs -n vhome -g''.
##

#   the used tools
APXS=apxs
APACHECTL=apachectl

#   additional defines, includes and libraries
#DEF=-Dmy_define=my_value
#INC=-I/usr/include/mysql
#LIB=-L/usr/lib/ -lmysqlclient
#INCLUDE=-I/usr/include/mysql
INCLUDE=-I./
CC=gcc
CFLAGS=$(INCLUDE)
LDLIBS=-Lnetwork_api/ -lnet_api

#   the default target
all: mod_vhome.so vhomed

#   compile the shared object file
mod_vhome.so: mod_vhome.c
	$(APXS) -c $(DEFS) $(INCLUDE) $(LDLIBS) mod_vhome.c

#	build the daemon
vhomed: vhomed.c
	$(MAKE) -C network_api
	$(CC) vhomed.c -g -Wall -I/usr/include -I./ -L/usr/lib/mysql -lz -lmysqlclient -o vhomed network_api/libnet_api.a 

#   install the shared object file into Apache 
install: all
	$(APXS) -i -a -n 'vhome' mod_vhome.so

#   cleanup
clean:
	-rm -f mod_vhome.o mod_vhome.so vhomed

#   simple test
test: reload
	lynx -mime_header http://localhost/vhome

#   install and activate shared object by reloading Apache to
#   force a reload of the shared object file
reload: install restart

#   the general Apache start/restart/stop
#   procedures
start:
	$(APACHECTL) start
restart:
	$(APACHECTL) restart
stop:
	$(APACHECTL) stop

