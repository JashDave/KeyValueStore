# Make for version 2

# Dependencies

INSTALL_PATH = /usr/local
LIB_NAME = libkvstore_v2.a
G++ = g++ -O3 -std=c++11 -pthread
BIN = bin

all:	install_deps init install


install_deps:
	sudo apt-get install libboost-serialization-dev

init:
	mkdir -p $(BIN)

install:	$(BIN)/KVStoreHeader_v2.o $(BIN)/$(LIB_NAME)
	cp -f $(BIN)/$(LIB_NAME) $(INSTALL_PATH)/lib
	mkdir -p $(INSTALL_PATH)/include/kvstore
	cp -f *.h $(INSTALL_PATH)/include/kvstore/

$(BIN)/KVStoreHeader_v2.o:	KVStoreHeader_v2.h KVStoreHeader_v2.cpp KVImpl.h
	$(G++) -c KVStoreHeader_v2.cpp -o $(BIN)/KVStoreHeader_v2.o

$(BIN)/$(LIB_NAME):	$(BIN)/KVStoreHeader_v2.o
	ar -cvq $(BIN)/$(LIB_NAME) $(BIN)/*.o

clean:
	rm -r $(BIN)

uninstall:
	rm -r $(INSTALL_PATH)/include/kvstore/
	rm $(INSTALL_PATH)/lib/$(LIB_NAME)
