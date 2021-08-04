CC = g++
CCFLAGS = -g -Wall -std=c++11 -pthread -I ../base
OBJECTS = test3
LIB_SRC = Eventloop.cc Channel.cc Poller.cc TimerQueue.cc EventloopThread.cc
BASE_SRC= ../base/Logging.cc ../base/LogStream.cc ../base/Thread.cc ../base/Timestamp.cc

all: $(OBJECTS)

$(OBJECTS):
	$(CC) $(CCFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^)

test3: test3.cc

clean:
	rm -f $(OBJECTS) core