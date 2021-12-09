OUT = lib
CC = g++
ODIR = obj
SDIR = src
INC = -Iinc
CFLAGS = -Wall -g
GPU = false
MPI = false
GPUNAME = GPU_Util.cpp
MAINFILE = main
IS_MPI = false

AGENT_TYPE = cpp
ifeq ($(MPI),true)
MAINFILE = main_mpi
OUT = lib_mpi
ODIR = obj_mpi
CC = mpic++
ADD_MPI = $(ODIR)/MPI_Util.o
IS_MPI = true
CFLAGS = -g -DIS_MPI
endif
ifeq ($(GPU),true)
AGENT_TYPE = cu
CC = nvcc
OUT = lib_gpu
ODIR = obj_gpu
CFLAGS = -g -dc
GPUNAME = GPU_Util.cu
endif
ifeq ($(OMP),true)
OUT = lib_omp
ODIR = obj_omp
CFLAGS = -g -fopenmp
FLAGSRUNME = -g -fopenmp
endif

# $(SDIR)/%.o : $(SDIR)/%.cpp
# 	$(CC) -c $(SDIR)
# edit : $(ODIR)/%.o
$(OUT)/run_me : $(ODIR)/$(MAINFILE).o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
       $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/Serial_Util.o $(ODIR)/Agent.o $(ODIR)/GPU_Util.o $(ODIR)/MPI_Util.o
	$(CC) $(FLAGSRUNME) -o $(OUT)/run_me $(ODIR)/$(MAINFILE).o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
        $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/Serial_Util.o $(ODIR)/Agent.o $(ODIR)/GPU_Util.o $(ODIR)/MPI_Util.o

$(ODIR)/$(MAINFILE).o : $(SDIR)/$(MAINFILE).cpp $(SDIR)/Agent.h $(SDIR)/Util.h $(SDIR)/FileWriter.h $(SDIR)/Serial_Util.h $(SDIR)/GPU_Util.h
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/$(MAINFILE).cpp  
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
$(ODIR)/Agent.o : $(SDIR)/Agent.$(AGENT_TYPE)
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/Agent.$(AGENT_TYPE)
$(ODIR)/MPI_Util.o : $(SDIR)/MPI_Util.cpp
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/MPI_Util.cpp
$(ODIR)/GPU_Util.o : $(SDIR)/$(GPUNAME)
	$(CC) -c $(CFLAGS) -o $@ $(SDIR)/$(GPUNAME)
clean :
	rm $(OUT)/run_me $(ODIR)/$(MAINFILE).o $(ODIR)/Point.o $(ODIR)/Map.o $(ODIR)/Properties.o \
           $(ODIR)/Util.o $(ODIR)/FileWriter.o $(ODIR)/Serial_Util.o $(ODIR)/Agent.o $(ODIR)/GPU_Util.o $(ODIR)/MPI_Util.o