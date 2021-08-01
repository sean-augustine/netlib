CC = g++
CCFLAGS = -g -Wall -std=c++11 -pthread -I ../base
OBJECTS = test
LIB_SRC = Eventloop.cc Channel.cc Poller.cc
BASE_SRC= ../base/Logging.cc ../base/LogStream.cc ../base/Thread.cc ../base/Timestamp.cc

all: $(OBJECTS)

$(OBJECTS):
	$(CC) $(CCFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^)

test: test.cc

clean:
	rm -f $(OBJECTS) core