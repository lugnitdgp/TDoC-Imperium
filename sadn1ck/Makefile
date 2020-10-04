CXX := g++
CXX_FLAGS := -std=c++17 -ggdb

BIN := bin
SRC := src
BASEDIR := ~/imperium
INCLUDE :=
LIB := LIB
LIBRARIES :=
EXECUTABLE := main

all: $(BIN)/$(EXECUTABLE)

run: clean all
	@echo "Run called"
	$(BASEDIR)/$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $ $(BASEDIR)/$(BIN)/$(EXECUTABLE) $(LIBRARIES) -lssl -lcrypto
clean:
	@echo "Clean called"
	@rm -rf $(BASEDIR)/$(BIN)/*