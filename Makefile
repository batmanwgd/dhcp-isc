# Uncomment the library definitions for the platform you are using.
# If there is no entry for your O.S., you may not need any special
# libraries.

# SunOS 4.1
#LIBS = -lresolv

# Solaris 2.5 (with gcc)
#LIBS = -lresolv
#CC=gcc -Wall -Wstrict-prototypes -Wno-unused -Wno-implicit -Wno-comment \
#	  -Wno-uninitialized -Werror

# DEC Alpha/OSF1
LIBS=

CSRC = options.c errwarn.c convert.c conflex.c confpars.c \
       tree.c memory.c alloc.c print.c hash.c tables.c inet.c db.c \
       dispatch.c bpf.c packet.c raw.c nit.o
COBJ = options.o errwarn.o convert.o conflex.o confpars.o \
       tree.o memory.o alloc.o print.o hash.o tables.o inet.o db.o \
       dispatch.o bpf.o packet.o raw.o nit.o
SRCS = dhcpd.c socket.c dhcp.c bootp.c
OBJS = dhcpd.o socket.o dhcp.o bootp.o
PROG = dhcpd
MAN=dhcpd.8 dhcpd.conf.5

all:	dhcpd

DEBUG=-g
CFLAGS=$(DEBUG)

dhcpd:	$(OBJS) $(COBJ)
	$(CC) -o dhcpd $(OBJS) $(COBJ) $(LIBS)

dhclient:	dhclient.o $(COBJ)
	$(CC) -o dhclient dhclient.o $(COBJ) $(LIBS)
