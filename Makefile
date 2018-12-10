
OBJS = main.o rowstore.o tuple.o
EXECUTABLE = main
CPPFLAGS = -I.

all: $(OBJS)
	g++ $(OBJS) -o $(EXECUTABLE)

clean:
	rm -rf $(OBJS) $(EXECUTABLE)
