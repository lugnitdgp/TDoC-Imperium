CXX      := g++
CXX_FLAGS := -std=c++17 -ggdb
BIN := bin
SRC := src
INCLUDE :=
LIB := lib
LIBRARIES :=
EXECUTABLE := main

all: $(BIN)/$(EXECUTABLE)
run: clean all
	@echo "run called"
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $ ~/imperium/$(BIN)/$(EXECUTABLE) $(LIBRARIES) -lssl -lcrypto

clean:
	@echo "clean called"
	@rm -rf -d ~/imperium/$(BIN)/*