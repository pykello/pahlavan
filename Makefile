
OBJS = src/tuple.o src/rowstore.o
EXECUTABLE = main
CPPFLAGS = -Isrc -Ilib

all: $(OBJS) src/main.cc 
	g++ $(CPPFLAGS) $(OBJS) src/main.cc -o $(EXECUTABLE)

clean:
	rm -rf $(OBJS) $(EXECUTABLE)
