EXEC_SENDER = reliable_sender
EXEC_RECEIVER = reliable_receiver


COMPILER = g++

all: reliable_sender reliable_receiver

reliable_sender: reliable_sender.o test_obj.o
	$(COMPILER) -pthread reliable_sender.o test_obj.o -o reliable_sender

reliable_receiver: reliable_receiver.o
	$(COMPILER) -pthread reliable_receiver.o -o reliable_receiver

reliable_sender.o: src/reliable_sender.cpp
	$(COMPILER) -c src/reliable_sender.cpp

reliable_receiver.o: src/reliable_receiver.cpp
	$(COMPILER) -c src/reliable_receiver.cpp

test_obj.o: src/test_obj.cpp
	$(COMPILER) -c src/test_obj.cpp

# mp3server : mp3.o libcommon.o
# 	$(COMPILER) -pthread mp3server.cpp mp3.o libcommon.o -o mp3server
#
# mp3client: mp3.o libcommon.o
# 	$(COMPILER) -pthread mp3client.cpp mp3.o libcommon.o -o mp3client
#
# mp3.o : mp3.cc
# 	$(COMPILER) -I /usr/lib/x86_64-redhat-linux5E/include mp3.cc -c
#
# mp3client.o : mp3client.cpp
# 	$(COMPILER) libcommon.cpp mp3client.cpp
#
# mp3server.o : mp3server.cpp
# 	$(COMPILER) libcommon.cpp mp3server.cpp

clean:
	-rm *.o
