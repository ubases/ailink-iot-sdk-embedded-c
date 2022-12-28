

ifdef compile
CC := $(compile)-gcc
AR := $(compile)-ar
else
CC := gcc
AR := ar
endif


SRC_DIR := utils
SRC_DIR += coreAilink/src
SRC_DIR += coreMQTT

# ifdef platform_dir
# SRC_DIR += platform/$(platform_dir)
# else
# SRC_DIR += platform/linux
# endif

INC_DIR := coreAilink/include
INC_DIR += coreAilink/interface
INC_DIR += utils
INC_DIR += coreMQTT/include

TARGET := libAilink.a
BUILD_DIR = .build



CFLAGS := $(patsubst %,-I%,$(INC_DIR))
INCLUDE=$(foreach dir, $(INC_DIR), $(wildcard $(dir)/*.h)) #搜索路径下.h文件

SOURCES=$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
OBJS=$(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SOURCES)))
VPATH=$(SRC_DIR) #指定搜索路径

CFLAGS += $(EXTRA_CCFLAGS)
# CFLAGS := -O2 -g -Wall -Wshadow -pedantic -Werror -std=c99

$(TARGET): $(OBJS)
	$(AR) -r -c $@ $^


$(BUILD_DIR)/%.o:%.c $(INCLUDE) | create_folder  #可以实时更新.h文件下进行编译
	$(CC) $(CFLAGS) -c $< -o $@ 


.PHONY:clean create_folder

clean:
	@echo "clean build folder"
	rm -r $(BUILD_DIR)


create_folder:
	@echo "create folder"
	mkdir $(BUILD_DIR)

