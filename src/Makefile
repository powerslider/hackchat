IDIR =../include
CC = gcc
CFLAGS = -I$(IDIR)
ODIR = obj
LDIR = ../lib
LIBS = -lm

_DEPS = easysocket.h chat.h chat_server.h chat_client.h util.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

SERVER_FILES = easysocket.o chat.o util.o chat_server.o
SERVER_OBJ = $(patsubst %,$(ODIR)/%,$(SERVER_FILES))

CLIENT_FILES = easysocket.o chat.o util.o chat_client.o
CLIENT_OBJ = $(patsubst %,$(ODIR)/%,$(CLIENT_FILES))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(SERVER_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

client: $(CLIENT_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

hackchat: server client

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
