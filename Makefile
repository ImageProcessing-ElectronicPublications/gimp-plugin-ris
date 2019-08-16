# Predefined constants
CC      = gcc
TARGET  = gimp-plugin-ris
SRC_DIR = src
OBJ_DIR = obj
CFLAGS  = $(shell pkg-config --cflags gtk+-2.0) \
          $(shell pkg-config --cflags gimp-2.0)
LFLAGS  = $(shell pkg-config --libs glib-2.0) \
          $(shell pkg-config --libs gtk+-2.0) \
          $(shell pkg-config --libs gimp-2.0) \
          $(shell pkg-config --libs gimpui-2.0)
TEST    = test
MKDIR   = mkdir -p
INSTALL = install
RM      = rm -f
DEST    = $(DESTDIR)$(exec_prefix)/lib/gimp/2.0/plug-ins

# File definitions
SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJ_FILES=$(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(TARGET): $(OBJ_DIR) $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(TARGET) $(LFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR):
	$(TEST) -d $(OBJ_DIR) || $(MKDIR) $(OBJ_DIR)

clean:
	$(RM) -r $(OBJ_DIR)
	$(RM) $(TARGET)

install:
	$(INSTALL) -d $(DEST)
	$(INSTALL) -m 0755  $(TARGET) $(DEST)

uninstall:
	$(RM) $(DEST)/$(TARGET)

.PHONY: clean install uninstall
