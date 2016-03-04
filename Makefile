TARGET = Proxy

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),FreeBSD)
	CC = clang++
	STFLAGS =
endif
ifeq ($(UNAME_S),Linux)
	CC = g++
	STFLAGS =
endif

INCLUDE_PATH ?= .
MINILIB_PATH ?= ./LibMiniLib
OBJECT_PATH ?= .obj

#DOPTS = -g -O0
DOPTS = -O1
CXXFLAGS := -Wall $(DOPTS) -I$(INCLUDE_PATH) -I$(MINILIB_PATH)/Include -std=c++1z

LOCKFLAGS := $(MINILIB_PATH)/$(UNAME_S)/LibLock.a
DAEMONFLAGS := $(MINILIB_PATH)/$(UNAME_S)/LibDaemon.a
THREADFLAGS := $(MINILIB_PATH)/$(UNAME_S)/LibThreads.a
MEMORYFLAGS := $(MINILIB_PATH)/$(UNAME_S)/LibMemory.a
LAYERFLAGS := $(MINILIB_PATH)/$(UNAME_S)/LibLayer.a
CRAFTERFLAGS := $(MINILIB_PATH)/$(UNAME_S)/LibCraft.a
IFACEFLAGS := $(MINILIB_PATH)/$(UNAME_S)/LibInterface.a
LDFLAGS := -lpthread -lssl -lcrypto

#DFLAGS  = -DLOGGER
DFLAGS =

_OBJECTS = Main.o Check.o Nix.o Dae.o Proto.o Iface.o
OBJECTS = $(patsubst %,$(OBJECT_PATH)/%,$(_OBJECTS))

$(OBJECT_PATH)/%.o: %.cpp | $(OBJECT_PATH)
	$(CC) -c -o $@ $< $(CXXFLAGS) $(DFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LOCKFLAGS) $(DAEMONFLAGS) $(THREADFLAGS) $(MEMORYFLAGS) $(LAYERFLAGS) $(CRAFTERFLAGS) $(IFACEFLAGS) $(LDFLAGS) $(STFLAGS)

$(OBJECT_PATH):
	@mkdir -p $(OBJECT_PATH)

.PHONY: clean

clean:
	$(RM) $(OBJECT_PATH)/*.o $(TARGET)

