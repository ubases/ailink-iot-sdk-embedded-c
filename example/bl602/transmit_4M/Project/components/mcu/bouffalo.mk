# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS :=  	../../include/ \
								../../internal_include/ \
								include \


## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=
## This component's src
COMPONENT_SRCS :=   mcu_process.c \
					mcu_system.c \
					mcu_log.c \




COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_SRCDIRS :=    ./

##
#CPPFLAGS +=


