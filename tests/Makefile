include ../build_config.mk

CFLAGS += -I../src -I../deps/leveldb-1.18/include -std=c++11
OBJS = test_harness.o test_ssdb.o
EXES = test_ssdb

all: $(EXES)

test_harness.o: test_harness.h
	$(CXX) $(CFLAGS) -c test_harness.h -o test_harness.o 2>/dev/null || true

test_ssdb.o: test_ssdb.cpp test_harness.h
	$(CXX) $(CFLAGS) -c test_ssdb.cpp

test_ssdb: test_ssdb.o ../src/ssdb/libssdb.a ../src/util/libutil.a ../src/net/libnet.a
	$(CXX) -o test_ssdb test_ssdb.o ../src/ssdb/libssdb.a ../src/util/libutil.a ../src/net/libnet.a $(CLIBS)

test: test_ssdb
	./test_ssdb

clean:
	rm -f *.o *.exe $(EXES)
	rm -rf test_var*