# The phone link's test client (tools/phone/phonecli.c): the protocol core and Monocypher from src/mp
SRC := ../../src/mp
phonecli: phonecli.c $(SRC)/phone_proto.c $(SRC)/monocypher/monocypher.c $(SRC)/phone_proto.h
	$(CC) -O2 -Wall -I$(SRC) -o $@ phonecli.c $(SRC)/phone_proto.c $(SRC)/monocypher/monocypher.c
clean:
	rm -f phonecli
