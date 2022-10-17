TARGET  = liblifx.dylib
CFLAGS  += -O1 -Wall -g -fstack-protector-all -Iinclude
LDFLAGS += -shared
SOURCES = lifx.c
HEADERS = lifx_internal.h lifx_products.h lifx_protocol.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

clean: clean_samples
	rm -f -- $(TARGET)
	rm -rf -- $(TARGET).dSYM

.PHONY: samples clean_samples

samples: $(TARGET)
	$(MAKE) -C samples/discovery

clean_samples:
	$(MAKE) -C samples/discovery clean
