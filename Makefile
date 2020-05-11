# Predefined constants
CC      = gcc
TARGET  = gimp-plugin-ris
TARGETLIB = libris.a
SRC_DIR = src
OBJ_DIR = obj
GTKCFLAGS  = $(shell pkg-config --cflags gtk+-2.0) \
             $(shell pkg-config --cflags gimp-2.0)
GTKLFLAGS  = $(shell pkg-config --libs glib-2.0) \
             $(shell pkg-config --libs gtk+-2.0) \
             $(shell pkg-config --libs gimp-2.0) \
             $(shell pkg-config --libs gimpui-2.0)
TEST    = test
AR      = ar
MKDIR   = mkdir -p
INSTALL = install
RM      = rm -f
DEST    = $(DESTDIR)$(exec_prefix)/lib/gimp/2.0/plug-ins

# File definitions
LIB_SRC_FILES=$(wildcard $(SRC_DIR)/lib/*.c)
LIB_OBJ_FILES=$(LIB_SRC_FILES:$(SRC_DIR)/lib/%.c=$(OBJ_DIR)/lib/%.o)
GTK_SRC_FILES=$(wildcard $(SRC_DIR)/gtk2/*.c)
GTK_OBJ_FILES=$(GTK_SRC_FILES:$(SRC_DIR)/gtk2/%.c=$(OBJ_DIR)/gtk2/%.o)

$(TARGET): $(OBJ_DIR) $(TARGETLIB) $(GTK_OBJ_FILES)
	$(CC) $(GTK_OBJ_FILES) $(TARGETLIB) -o $(TARGET) $(GTKLFLAGS)

$(TARGETLIB): $(OBJ_DIR) $(LIB_OBJ_FILES)
	$(AR) rcs $@ $(LIB_OBJ_FILES)

$(OBJ_DIR)/lib/%.o: $(SRC_DIR)/lib/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/gtk2/%.o: $(SRC_DIR)/gtk2/%.c
	$(CC) -c $< -o $@ $(GTKCFLAGS)

$(OBJ_DIR):
	$(TEST) -d $(OBJ_DIR) || $(MKDIR) $(OBJ_DIR)
	$(TEST) -d $(OBJ_DIR)/lib || $(MKDIR) $(OBJ_DIR)/lib
	$(TEST) -d $(OBJ_DIR)/gtk2 || $(MKDIR) $(OBJ_DIR)/gtk2

clean:
	$(RM) -r $(OBJ_DIR)
	$(RM) $(TARGETLIB) $(TARGET)

install:
	$(INSTALL) -d $(DEST)
	$(INSTALL) -m 0755  $(TARGET) $(DEST)

uninstall:
	$(RM) $(DEST)/$(TARGET)

.PHONY: clean install uninstall
