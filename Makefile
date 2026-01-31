TARGET_EXEC := allocator

CC := gcc

SRC_DIR := ./src
BUILD_DIR := ./build


SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

WARN_FLAGS += -Wall -Wno-comment   -Wextra -Wpedantic
MAKE_FLAGS += -j
CFLAGS += $(WARN_FLAGS)


.MAIN: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

all: $(TARGET_EXEC) $(OBJ) 


$(OBJS) : $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c 
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET_EXEC)
	$(RM) -rd $(BUILD_DIR)


.PHONY: help
help:
	@echo  'Targets:'
	@echo  "  $(TARGET_EXEC)         - Compiles the allocator (default)"
	@echo  '  all             - Compiles the shell along with the kernel module'
	@echo  ''
	@echo  '  clean           - Removes build files'
