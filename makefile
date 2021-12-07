OUT = lib
CC = g++
ODIR = obj
SDIR = src
INC = -Iinc
CFLAGS = -Wall -g
GPU = false
MPI = false
GPUNAME = GPU_Util.cpp

IS_MPI = false

ifeq ($(MPI),true)
OUT = lib_mpi
ODIR = obj_mpi
CC = mpicc
ADD_MPI = $(ODIR)/MPI_Util.o
IS_MPI = true
CFLAGS = -g -DIS_MPI
endif
ifeq ($(GPU),true)
CC = nvcc
OUT = lib_gpu
ODIR = obj_gpu
CFLAGS = -g
GPUNAME = GPU_Util.cu
endif
# $(SDIR)/%.o : $(SDIR)/%.cpp
# 	$(CC) -c $(SDIR)
# edit : $(ODIR)/%.o
$(OUT)/run_me : $(ODIR)/main.o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
       $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/Serial_Util.o $(ODIR)/Agent.o $(ODIR)/GPU_Util.o $(ODIR)/MPI_Util.o
	$(CC) -o $(OUT)/run_me $(ODIR)/main.o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
        $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/Serial_Util.o $(ODIR)/Agent.o $(ODIR)/GPU_Util.o $(ODIR)/MPI_Util.o

$(ODIR)/main.o : $(SDIR)/main.cpp $(SDIR)/Agent.h $(SDIR)/Util.h $(SDIR)/FileWriter.h $(SDIR)/Serial_Util.h $(SDIR)/GPU_Util.h
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/main.cpp  
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
$(ODIR)/Serial_Util.o : $(SDIR)/Serial_Util.cpp $(SDIR)/Agent.h $(SDIR)/Map.h $(SDIR)/Properties.h $(SDIR)/Util.h $(SDIR)/Serial_Util.h
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/Serial_Util.cpp
$(ODIR)/Agent.o : $(SDIR)/Agent.cpp
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/Agent.cpp
$(ODIR)/MPI_Util.o : $(SDIR)/MPI_Util.cpp
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/MPI_Util.cpp
$(ODIR)/GPU_Util.o : $(SDIR)/$(GPUNAME)
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/$(GPUNAME)
clean :
	rm $(OUT)/run_me $(ODIR)/main.o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
           $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/Serial_Util.o $(ODIR)/Agent.o $(ODIR)/GPU_Util.o $(ODIR)/MPI_Util.o