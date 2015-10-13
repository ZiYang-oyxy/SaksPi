USOBJ=usock.o

all: libusock.a

libusock.a: $(USOBJ)
	$(AR) rcs $@ $(USOBJ)

clean:
	rm -f $(USOBJ) libusock.a
