# CS438 MP1: HTTP Simple Client and Server
### Due: Oct.3rd, 2018, 11:59PM CDT

## 438 Time-line:

#### 9-29:

server - generate_http_request() finished

given filename it can read the file and write it to buffer and compose a response string

#### 9-31:

server - multi-process enabled, cycling send and recv 

server - debugged

clinet - input parser
parse standard input like https://xxx.x.x.x:xx/xxx

client - input parse debugged

client - cycling send and recv(not debugged )

#### 10-2

debugged client.c's send function

fixed the HTTP HEADER in server.c

done with Makefile

First autograder test gives a 75/100 score

## TO-DO: 

Find reason for not passing the testcase - failed to get a medium size file from the server, -25%



## How to test:

1. put [input] and [server.c] in same folder and compile. -vm1

2. put [client.c] in a file and compile.  -vm2

3. start server.exe in vm1 [./server ]

4. start client.exe in vm2 [./client http://hostname[:port]/input]

5. expected result: ./input in vm1 is the exactly the same as ./input vm2

