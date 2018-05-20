# Makefile for generating the cap'n proto code from the schema
	
logEntry.capnp.h: logEntry.capnp
	capnp compile -oc++ logEntry.capnp


.PHONY: clean
clean:
	rm -f logEntry.capnp.h
	rm -f logEntry.capnp.c++
	rm -f *~
