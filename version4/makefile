CC = g++
CCFLAGS = -g -Wall -std=c++11 -pthread -I ../base
CDFLAGS =-lnetlib -L.
OBJECTS = test3
LIB_SRC = Eventloop.cc Channel.cc Poller.cc TimerQueue.cc EventloopThread.cc Acceptor.cc InetAddress.cc Socketops.cc Socket.cc
BASE_SRC= ../base/Logging.cc ../base/LogStream.cc ../base/Thread.cc ../base/Timestamp.cc


ALL_SRCF = $(notdir $(LIB_SRC) $(BASE_SRC))
ALL_OBJS = $(patsubst %.cc,%.o,$(ALL_SRCF))

all: $(OBJECTS)

libnetlib.a : $(LIB_SRC) $(BASE_SRC)
	$(CC) $(CCFLAGS) -c $^
	ar rcs $@ $(ALL_OBJS)

$(OBJECTS):libnetlib.a
	$(CC) $(CCFLAGS) -o $@ $(filter %.cc,$^) $(CDFLAGS)

test3: test3.cc

clean:
	rm -f $(OBJECTS) *.o *.a core