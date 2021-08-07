CC = g++
CCFLAGS = -g -Wall -std=c++11 -pthread -I ../base
CDFLAGS =-lnetlib -L.
OBJECTS = test2
LIB_SRC = Eventloop.cc Channel.cc Poller.cc TimerQueue.cc EventloopThread.cc Acceptor.cc \
		  InetAddress.cc Socketops.cc Socket.cc Tcpserver.cc Tcpconnection.cc Buffer.cc
BASE_SRC= ../base/Logging.cc ../base/LogStream.cc ../base/Thread.cc ../base/Timestamp.cc


ALL_SRCF = $(notdir $(LIB_SRC) $(BASE_SRC))
ALL_OBJS = $(patsubst %.cc,%.o,$(ALL_SRCF))

all: $(OBJECTS)

libnetlib.a : $(LIB_SRC) $(BASE_SRC)
	$(CC) $(CCFLAGS) -c $^
	ar rcs $@ $(ALL_OBJS)

$(OBJECTS):libnetlib.a
	$(CC) $(CCFLAGS) -o $@ $(filter %.cc,$^) $(CDFLAGS)

test2: test2.cc

clean:
	rm -f $(OBJECTS) *.o *.a core