#Defines
CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic
EXE=flexxon_tool
SOURCES=block.c xmask_cmd.c main.c
OBJECTS=$(SOURCES:.c=.o)

#Build Rules

all: $(SOURCES) $(EXE)

.c:
	$(CC) $(CFLAGS) $< -o $@

$(EXE): $(OBJECTS)

	$(CC) $(OBJECTS) -o $(EXE)

clean: 
	rm -f *.o $(EXE)
