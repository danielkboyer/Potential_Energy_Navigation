OUT = lib
CC = g++
ODIR = obj
SDIR = src
INC = -Iinc
CFLAGS = -Wall -g


# $(SDIR)/%.o : $(SDIR)/%.cpp
# 	$(CC) -c $(SDIR)
# edit : $(ODIR)/%.o
$(OUT)/run_me : $(ODIR)/cpu_main.o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
       $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/CPU_Util.o $(ODIR)/Agent.o
	$(CC) -o $(OUT)/run_me $(ODIR)/cpu_main.o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
        $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/CPU_Util.o $(ODIR)/Agent.o

$(ODIR)/cpu_main.o : $(SDIR)/cpu_main.cpp $(SDIR)/Agent.h $(SDIR)/Util.h $(SDIR)/FileWriter.h $(SDIR)/CPU_Util.h
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/cpu_main.cpp  
$(ODIR)/Point.o : $(SDIR)/Point.cpp
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/Point.cpp
$(ODIR)/Map.o : $(SDIR)/Map.cpp $(SDIR)/Point.h
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/Map.cpp
$(ODIR)/Properties.o : $(SDIR)/Properties.cpp
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/Properties.cpp
$(ODIR)/Util.o : $(SDIR)/Util.cpp
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/Util.cpp
$(ODIR)/FileWriter.o : $(SDIR)/FileWriter.cpp $(SDIR)/Agent.h
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/FileWriter.cpp
$(ODIR)/CPU_Util.o : $(SDIR)/CPU_Util.cpp $(SDIR)/Agent.h $(SDIR)/Map.h $(SDIR)/Properties.h $(SDIR)/Util.h $(SDIR)/CPU_Util.h
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/CPU_Util.cpp
$(ODIR)/Agent.o : $(SDIR)/Agent.cpp
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/Agent.cpp
clean :
	rm $(OUT)/run_me $(ODIR)/cpu_main.o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
           $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/CPU_Util.o $(ODIR)/Agent.o