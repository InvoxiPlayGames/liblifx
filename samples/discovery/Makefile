TARGET  = lifx_discovery
CFLAGS  += -O1 -Wall -g -fstack-protector-all -I../../include
LDFLAGS += -L../.. -llifx
SOURCES = discovery.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

clean:
	rm -f -- $(TARGET)
	rm -rf -- $(TARGET).dSYM
