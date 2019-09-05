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
ifeq ($(MAGAOX_ROLE),icc)
  EDT = true
  PYLON = true
  PICAM = true
else
ifeq ($(MAGAOX_ROLE),rtc)
  EDT = true
endif
endif

CFLAGS += -D_XOPEN_SOURCE=700
CXXFLAGS += -D_XOPEN_SOURCE=700

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
EXTRA_LDLIBS ?=  -lsofa_c \
  -lboost_system \
  -lboost_filesystem \
  -ludev \
  -lpthread \
  -ltelnet \
  -lcfitsio \
  -lxrif \
  $(abspath \
  $(SELF_DIR)/../INDI/libcommon/libcommon.a) \
  $(abspath $(SELF_DIR)/../INDI/liblilxml/liblilxml.a)

CACAO ?= true
ifneq ($(CACAO),false)
  EXTRA_LDLIBS +=  -lImageStreamIO
endif

### MKL BLAS

BLAS_INCLUDES ?= -DMXLIB_MKL -m64 -I${MKLROOT}/include
BLAS_LDFLAGS ?= -L${MKLROOT}/lib/intel64 -Wl,--no-as-needed
BLAS_LDLIBS ?= -lmkl_intel_lp64 -lmkl_gnu_thread -lmkl_core -lgomp -lpthread -lm -ldl

#2nd step in case we need to modify above for other architectures/systems
INCLUDES += $(BLAS_INCLUDES)
EXTRA_LDFLAGS += $(BLAS_INCLUDES)
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
