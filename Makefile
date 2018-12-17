CPPFLAGS = -Isrc -Ilib
OBJS = src/tuple.o src/rowstore.o src/datetime.o
EXECUTABLE = main
TEST_EXECUTABLE = run_tests
TEST_OBJS = tests/tests_main.o \
			tests/test_tuples.o \
			tests/test_exprs.o

all: $(OBJS) src/main.cc 
	g++ $(CPPFLAGS) $(OBJS) src/main.cc -o $(EXECUTABLE)

tests: $(OBJS) $(TEST_OBJS)
	g++ $(CPPFLAGS) $(OBJS) $(TEST_OBJS) -o $(TEST_EXECUTABLE)

clean:
	rm -rf $(OBJS) $(TEST_OBJS) $(EXECUTABLE) $(TEST_EXECUTABLE)
