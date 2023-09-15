#################################################
#
# Common path and make variable definitions for a MagAO-X build.
#
# This file defines the typical values and manages O/S detection, etc.
# To change these create a local/common.mk and set the values you want to change
# in that directory.
#
# NOTE: do not edit this file, as it will show as a git repo modification.
#
###################################################

SELF_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
-include $(SELF_DIR)/../local/common.mk
include $(SELF_DIR)/config.mk

### Set up what libs to require based on the MAGAOX_ROLE
EDT ?= false
PYLON ?= false
PICAM ?= false
ifeq ($(MAGAOX_ROLE),ICC)
  EDT = true
  PYLON = true
  PICAM = true
else
ifeq ($(MAGAOX_ROLE),RTC)
  EDT = true
endif
endif

CFLAGS += -D_XOPEN_SOURCE=700
CXXFLAGS += -D_XOPEN_SOURCE=700

#Need this on COS-7, doesn't hurt elsewhere.
CXXFLAGS += -DMX_OLD_GSL



LIB_PATH ?= $(PREFIX)/lib
INCLUDE_PATH ?= $(PREFIX)/include
LIB_SOFA ?= $(LIB_PATH)/libsofa_c.a

INCLUDES += -I$(INCLUDE_PATH) -I$(abspath $(SELF_DIR)/../flatlogs/include) 


########################################
## Optimize Flags
#######################################
OPTIMIZE ?= -O3 -fopenmp -ffast-math

########################################
## Libraries
#######################################

EXTRA_LDFLAGS ?=

#the required librarires
EXTRA_LDLIBS ?=  -lmxlib \
  -ludev \
  -lpthread \
  -ltelnet \
  -lcfitsio \
  -lxrif \
  -lfftw3 -lfftw3f -lfftw3l \
  -lgsl \
  -lboost_system \
  -lboost_filesystem \
  $(abspath \
  $(SELF_DIR)/../INDI/libcommon/libcommon.a) \
  $(abspath $(SELF_DIR)/../INDI/liblilxml/liblilxml.a) 
  
HOST_ARCH   := $(shell uname -m)
ifeq ($(HOST_ARCH),x86_64)
  # only enable on x86 because ARM doesn't have quad precision by default
  EXTRA_LDLIBS += -lfftw3q
endif

ifeq ($(NEED_CUDA),yes)
   CXXFLAGS += -DEIGEN_NO_CUDA -DHAVE_CUDA

   CUDA_TARGET_ARCH = $(HOST_ARCH)
   ifneq (,$(filter $(CUDA_TARGET_ARCH),x86_64 aarch64 ppc64le armv7l))
       ifneq ($(CUDA_TARGET_ARCH),$(HOST_ARCH))
           ifneq (,$(filter $(CUDA_TARGET_ARCH),x86_64 aarch64 ppc64le))
               TARGET_SIZE := 64
           else ifneq (,$(filter $(CUDA_TARGET_ARCH),armv7l))
               TARGET_SIZE := 32
           endif
       else
           TARGET_SIZE := $(shell getconf LONG_BIT)
       endif
   else
       $(error ERROR - unsupported value $(CUDA_TARGET_ARCH) for TARGET_ARCH!)
   endif

   # operating system
   HOST_OS   := $(shell uname -s 2>/dev/null | tr "[:upper:]" "[:lower:]")
   TARGET_OS ?= $(HOST_OS)
   ifeq (,$(filter $(TARGET_OS),linux darwin qnx android))
       $(error ERROR - unsupported value $(TARGET_OS) for TARGET_OS!)
   endif

   HOST_COMPILER ?= g++
   NVCC          := nvcc -ccbin $(HOST_COMPILER)

   # internal flags
   NVCCFLAGS   := -m${TARGET_SIZE}
   NVCCFLAGS   +=  -DEIGEN_NO_CUDA -DMXLIB_MKL
   NVCCFLAGS   +=  ${NVCCARCH}
   NVCCFLAGS   +=

   # Debug build flags
   ifeq ($(dbg),1)
         NVCCFLAGS += -g
         BUILD_TYPE := debug
   else
         BUILD_TYPE := release
   endif

	INCLUDES += -I/usr/local/cuda/include

   ALL_CCFLAGS :=
   ALL_CCFLAGS += $(NVCCFLAGS)
   ALL_CCFLAGS += $(EXTRA_NVCCFLAGS)
   ALL_CCFLAGS += $(addprefix -Xcompiler ,$(CXXFLAGS))
   ALL_CCFLAGS += $(addprefix -Xcompiler ,$(EXTRA_CCFLAGS))
   ALL_CCFLAGS += -I/usr/local/cuda/include

   ALL_LDFLAGS :=
   ALL_LDFLAGS += $(ALL_CCFLAGS)
   ALL_LDFLAGS += $(addprefix -Xlinker ,$(LDFLAGS))
   ALL_LDFLAGS += $(addprefix -Xlinker ,$(LDLIBS))

   #build any cu and cpp files through NVCC as needed
%.o : %.cu
	$(NVCC) $(ALL_CCFLAGS) $< -c -o $@

   #Finally we define the cuda libs for linking
   CUDA_LIBS ?= $(CUDA_LIBPATH) -L/usr/local/cuda/lib64/ -lcudart -lcublas -lcufft -lcurand
  
else
   CUDA_LIBS ?=
  
endif

EXTRA_LDLIBS+= $(CUDA_LIBS)

#2021-01-07: added xpa to levmar

CACAO ?= true
ifneq ($(CACAO),false)
  EXTRA_LDLIBS +=  -L/usr/local/milk/lib -lImageStreamIO
  INCLUDES += -I/usr/local/milk/include
endif

### OpenBLAS compiler/linker flags
BLAS_INCLUDES ?= $(shell pkg-config --cflags-only-I openblas)
BLAS_LDFLAGS ?= $(shell pkg-config --libs-only-L openblas)
BLAS_LDLIBS ?= $(shell pkg-config --libs-only-l openblas)

#2nd step in case we need to modify above for other architectures/systems

INCLUDES += $(BLAS_INCLUDES)
EXTRA_LDFLAGS += $(BLAS_INCLUDES) $(BLAS_LDFLAGS)
EXTRA_LDLIBS += $(BLAS_LDLIBS)

### EDT

EDT_PATH=/opt/EDTpdv
EDT_INCLUDES=-I$(EDT_PATH)
EDT_LIBS = -L/opt/EDTpdv -lpdv -lpthread -lm -ldl

ifneq ($(EDT),false)
   INCLUDES += $(EDT_INCLUDES)
   EXTRA_LDLIBS += $(EDT_LIBS)
else
   CXXFLAGS+= -DMAGAOX_NOEDT
endif

#####################################

LDLIBS += $(EXTRA_LDLIBS)
LDFLAGS += $(EXTRA_LDFLAGS)

#Hard-code the paths to system libraries so setuid works
LDLIBRPATH := $(shell echo $$LD_LIBRARY_PATH | sed 's/::/:/g' |  sed 's/:/ -Wl,-rpath,/g')
LDLIBS += -Wl,-rpath,$(LDLIBRPATH)

########################################
## Compilation and linking
#######################################

CFLAGS += -std=c99 -fPIC $(INCLUDES) $(OPTIMIZE)
CXXFLAGS += -std=c++14 -Wall -Wextra -fPIC $(INCLUDES) $(OPTIMIZE)

#This is needed to force use of g++ for linking
LINK.o = $(LINK.cc)

#Create an implicit rule for pre-compiled headers
%.hpp.gch: %.hpp
	$(CXX) $(CXXFLAGS) -c $<
