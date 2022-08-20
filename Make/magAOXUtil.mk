####################################################
# Makefile for building MagAOX Utilities
#
# Refer to magAOX.mk for the main details of the MagAOX make system.
#
# This file adds utility specific installation.
#
# In principle this works standalone for a single-header application:
#    -- only an <app-name>.hpp and <app-name>.cpp are needed
#    -- in the directory containing those files, invoke make with
#         make -f ../../Make/magAOXUtil.mk t=<app-name>
#
# More complicated builds are also supported:
#    -- In a local Makefile, make the first rule:
#          allall: all
#    -- Then specify rules for building other .o files
#       --- Copy the $(TARGETS).o rule to depend on the pch.
#    -- List those .o files after OTHER_OBJS=
#    -- List any header dependencies after OTHER_HEADERS
#    -- Define TARGET=
#    -- Then finally include this file (magAOXUtil.mk)
#
# To support the top-level build system, each app needs to have a Makefile
# in its folder, with a minimum signature of:
#       TARGET=name
#       include ../../Make/magAOXUtil.mk
#
####################################################

SELF_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
include $(SELF_DIR)/../Make/magAOX.mk

install: all
	sudo install -d $(BIN_PATH)
	sudo install $(TARGET) $(BIN_PATH)
	sudo ln -sf $(BIN_PATH)/$(TARGET) /usr/local/bin/$(TARGET)
