CXX = g++ -O3
CXXFLAGS = -std=c++11

KEY_LOGGER_SRC = klog_main.cpp
KEY_LOGGER = klog1.exe

# Path to sqlite3.c file
SQLITE3_DIR = sqlite-amalgamation-3450200

SQLITE3_SRC = $(SQLITE3_DIR)/sqlite3.c
SQLITE3_OBJ = $(SQLITE3_DIR)/sqlite3.o

CXXFLAGS += -I $(SQLITE3_DIR)


all: $(KEY_LOGGER)

$(KEY_LOGGER): $(KEY_LOGGER_SRC) $(SQLITE3_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(KEY_LOGGER_SRC) $(SQLITE3_OBJ)

$(SQLITE3_OBJ): $(SQLITE3_SRC)
	gcc -c -O3 $< -o $@

clean:
	rm -f $(KEY_LOGGER) #$(OBJS)

