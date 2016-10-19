SOURCES = main.c
EXEC = movingSquare
CC = gcc
CFLAGS = `pkg-config --cflags glfw3` -c
LDFLAGS = `pkg-config --static --libs glfw3 glew`
OBJECTS = $(SOURCES:.c=.o)

all:
$(info ******** PICK FROM 3 DIFFERENT BUILDS ********)
$(info LINUX_BUILD    ... build using GLFW and GLEW)

LINUX_BUILD : $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 
	rm -rf $(OBJECTS)
	
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)