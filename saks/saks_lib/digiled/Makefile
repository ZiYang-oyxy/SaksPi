DIGILEDD_OBJ=digiledd.o utils.o
LIBDIGILED_OBJ=libdigiled.o
CLI_OBJ=cli.o

LDLIBS += -lusock
LDFLAGS += -Wl,-export-dynamic
LIB_LDFLAGS += $(LDFLAGS) -shared

all: digiledd libdigiled cli

digiledd: $(DIGILEDD_OBJ)

libdigiled: $(LIBDIGILED_OBJ)
	$(CC) $^ -o libdigiled.so $(LIB_LDFLAGS)

cli: $(CLI_OBJ) $(LIBDIGILED_OBJ)

clean:
	rm -f $(DIGILEDD_OBJ) $(LIBDIGILED_OBJ) $(CLIOBJ) digiledd libdigiled.so cli
