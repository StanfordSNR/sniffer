To compile:

```
$ cmake -S . -B build
$ cmake --build build --parallel
```

To run the test program:

```
$ sudo ./build/src/frontend/fun [device_name]
```

Example output:

```
Got packet:
   direction=outgoing
   IP protocol = UDP
   IPv4 source = 10.5.91.2, dest = 18.9.64.29
   UDP source port = 40914, dest port = 60003

Got packet:
   direction=outgoing
   IP protocol = TCP
   IPv4 source = 10.5.91.2, dest = 35.83.181.24
   not UDP => ignoring

Got packet:
   direction=incoming
   IP protocol = UDP
   IPv4 source = 18.9.64.29, dest = 10.5.91.2
   UDP source port = 60003, dest port = 40914
```