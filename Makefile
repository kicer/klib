PKG = k

PKG_LIB_DIR = ./lib
PKG_SRC_DIR = ./src
PKG_DEMO_DIR = ./demo
PKG_INC_DIR = ./include

PKG_DEMO = $(PKG_DEMO_DIR)/$(PKG)-test
PKG_DEMO_SRCS = $(wildcard $(PKG_DEMO_DIR)/*.c)

PKG_LIB_NAME = $(PKG)
PKG_LIB_A = $(PKG_LIB_DIR)/lib$(PKG_LIB_NAME).a
PKG_LIB_SO = $(PKG_LIB_DIR)/lib$(PKG_LIB_NAME).so
PKG_LIB_PART = sys os net dev gui
PKG_LIB_SRCS = $(wildcard $(PKG_SRC_DIR)/*.c) $(wildcard $(patsubst %,%/*.c,$(PKG_SRC_DIR)/$(PKG_LIB_PART)))
PKG_LIB_OBJS = $(patsubst %.c,%.o,$(PKG_LIB_SRCS))

all: lib demo

demo: lib $(PKG_DEMO)

$(PKG_DEMO): $(PKG_DEMO_SRCS)
	$(CC) $^ -o $@ -L $(PKG_LIB_DIR) -l$(PKG_LIB_NAME) -I $(PKG_INC_DIR)

ifdef USE_DYNAMIC_LIBRARY
lib: $(PKG_LIB_SO)
else # USE_STATIC_LIBRARY
lib: $(PKG_LIB_A)
endif

$(PKG_LIB_A): $(PKG_LIB_OBJS)
	$(AR) -r $@ $^

$(PKG_LIB_SO): $(PKG_LIB_OBJS)
	$(CC) -shared -fPIC $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -I $(PKG_INC_DIR) -c $< -o $@ 

install: lib

uninstall: clean

clean:
	@$(RM) -v $(PKG_LIB_OBJS)
	@$(RM) -v $(PKG_LIB_A) $(PKG_LIB_SO)
	@$(RM) -v $(PKG_DEMO)

.PHONY: all lib demo clean install uninstall
