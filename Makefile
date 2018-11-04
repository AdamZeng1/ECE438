EXEC_SENDER = minimun_file_sender
EXEC_RECEIVER = receiver_main


COMPILER = g++

all: minimun_file_sender receiver_main

minimun_file_sender: minimun_file_sender.o test_obj.o
	$(COMPILER) -pthread minimun_file_sender.o test_obj.o -o minimun_file_sender

receiver_main: receiver_main.o
	$(COMPILER) -pthread receiver_main.o -o receiver_main

minimun_file_sender.o: src/minimun_file_sender.cpp
	$(COMPILER) -c src/minimun_file_sender.cpp

receiver_main.o: src/receiver_main.cpp
	$(COMPILER) -c src/receiver_main.cpp

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
