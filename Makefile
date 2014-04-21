PKG = k

PKG_LIB_DIR = ./lib
PKG_SRC_DIR = ./src
PKG_BIN_DIR = ./bin
PKG_DEMO_DIR = ./demo
PKG_INC_DIR = ./include

PKG_INC_SRCS = $(wildcard $(PKG_INC_DIR)/*.h $(PKG_INC_DIR)/*/*.h)

PKG_DEMO_SRCS = $(wildcard $(PKG_DEMO_DIR)/*.c)
PKG_DEMO_OBJS = $(patsubst %.c,%.o,$(PKG_DEMO_SRCS))
PKG_DEMO_EXEC =  $(patsubst %.c,%,$(PKG_DEMO_SRCS))

PKG_BIN_SRCS = $(wildcard $(PKG_BIN_DIR)/*.c)
PKG_BIN_OBJS = $(patsubst %.c,%.o,$(PKG_BIN_SRCS))
PKG_BIN_EXEC =  $(patsubst %.c,%,$(PKG_BIN_SRCS))

PKG_LIB_NAME = $(PKG)
PKG_LIB_A = $(PKG_LIB_DIR)/lib$(PKG_LIB_NAME).a
PKG_LIB_SO = $(PKG_LIB_DIR)/lib$(PKG_LIB_NAME).so

PKG_LIB_PART = sys os net dev gui
PKG_LIB_SRCS = $(wildcard $(PKG_SRC_DIR)/*.c) $(wildcard $(patsubst %,%/*.c,$(PKG_SRC_DIR)/$(PKG_LIB_PART)))
PKG_LIB_OBJS = $(patsubst %.c,%.o,$(PKG_LIB_SRCS))

ifdef USE_DYNAMIC_LIBRARY
PKG_LIB_FILE = $(PKG_LIB_SO)
else # USE_STATIC_LIBRARY
PKG_LIB_FILE = $(PKG_LIB_A)
endif

all: lib bin demo

lib: $(PKG_LIB_FILE)

bin: $(PKG_BIN_EXEC)

demo: $(PKG_DEMO_EXEC)

$(PKG_BIN_EXEC): $(PKG_BIN_OBJS) $(PKG_LIB_FILE)

$(PKG_DEMO_EXEC): $(PKG_DEMO_OBJS) $(PKG_LIB_FILE)

%: %.o $(PKG_LIB_FILE)
	@printf "\033[034m%s\033[033m %s\033[0m" "[CCLD]" "$< --> $@"
	@ $(CC) $< -o $@ -L $(PKG_LIB_DIR) -l$(PKG_LIB_NAME) -I $(PKG_INC_DIR)
	@ printf "\r\033[75C\033[032m[ok]\033[0m\n"

$(PKG_LIB_A): $(PKG_LIB_OBJS)
	@ printf "\033[034m%s\033[033m %s\033[0m" "[CCAR]" "*.o --> $@"
	@ $(AR) -r $@ $^
	@ printf "\r\033[75C\033[032m[ok]\033[0m\n"

$(PKG_LIB_SO): $(PKG_LIB_OBJS)
	@ printf "\033[034m%s\033[033m %s\033[0m" "[FPIC]" "*.o --> $@"
	@ $(CC) -shared -fPIC $^ -o $@
	@ printf "\r\033[75C\033[032m[ok]\033[0m\n"

%.o: %.c $(PKG_INC_SRCS)
	@ printf "\033[034m%s\033[033m %s\033[0m" "[GCC ]" "$@"
	@ $(CC) $(CFLAGS) -I $(PKG_INC_DIR) -c $< -o $@ 
	@ printf "\r\033[75C\033[032m[ok]\033[0m\n"

install: $(PKG_LIB_FILE)

uninstall: clean

clean:
	@$(RM) -v $(PKG_LIB_OBJS) $(PKG_DEMO_OBJS) $(PKG_BIN_OBJS)
	@$(RM) -v $(PKG_LIB_A) $(PKG_LIB_SO)
	@$(RM) -v $(PKG_DEMO_EXEC) $(PKG_BIN_EXEC)

.PHONY: all lib demo clean install uninstall
