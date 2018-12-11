
OBJS = main.o tuple.o rowstore.o
EXECUTABLE = main
CPPFLAGS = -I.

all: $(OBJS)
	g++ $(OBJS) -o $(EXECUTABLE)

clean:
	rm -rf $(OBJS) $(EXECUTABLE)
