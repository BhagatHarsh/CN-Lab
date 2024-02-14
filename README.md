# CN-Lab
 Just backing up my codes from the lab


## Lab 6

To run this Server and Client program.

## In server

```
cd Server
```

```
gcc server.c -o server
```

```
./server -p 1234
```

## In client

```
cd Client
```

```
gcc client.c -o client
```

```
./client -p 1234 -h 0.0.0.0 -f file.txt
```

### Run the server first and then the client.

```
Whenever we type GET in the client, the server will send the file to the client.
```