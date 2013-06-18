OBJDIR:=./bin
AR:=ar
CC:=gcc #Compiler
EDL:=gcc #Linker
ARFLAGS:=rcs
CCFLAGS:=-Wall -g #Compiler options
EDLFLAGS:=-Wall -g #Linker options
EXE:=$(OBJDIR)/rtdetect
DEFINES:=DEBUG #Preprocessor definitions
ECHO:=@echo

OBJ:=$(OBJDIR)/rtdetect.o

.PHONY: all clean

all: $(EXE)

$(OBJDIR) :
	test -d $(OBJDIR) || mkdir $(OBJDIR)

$(EXE): $(OBJ)
	@echo building $<
	$(EDL) -o $(EXE) $(EDLFLAGS) $(OBJ)
	@echo done

$(OBJDIR)/%.o : %.c *.h $(OBJDIR)
	@echo building $< ...
	$(CC) $(CCFLAGS) -c -D $(DEFINES) -o $@ $<
	@echo done

clean:
	@echo -n cleaning repository...
	@rm -rf *.o
	@rm -rf *.a
	@rm -rf *.so*
	@rm -rf $(OBJDIR)/*.o
	@rm -rf $(OBJDIR)/*.a
	@rm -rf $(OBJDIR)/*.so*
	@rm -f *~
	@echo cleaned.
