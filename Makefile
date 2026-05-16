# Define the compiler and flags
CXX = clang++
CXXFLAGS = -std=c++17 -stdlib=libc++ -Wall -O2

# Names of all the executables
EXEC_NEFDAN = simulation_NefdaN
EXEC_BTC   = simulation_BTC_DAA

ALL_EXECS = $(EXEC_NEFDAN) $(EXEC_BTC) 

# Default target: compile all three programs when running "make"
all: $(ALL_EXECS)

# Compile rules for each individual executable
$(EXEC_NEFDAN): simulation_NefdaN.cpp
	$(CXX) $(CXXFLAGS) -o $(EXEC_NEFDAN) simulation_NefdaN.cpp

$(EXEC_BTC): simulation_BTC_DAA.cpp
	$(CXX) $(CXXFLAGS) -o $(EXEC_BTC) simulation_BTC_DAA.cpp


# Run original simulation with parameters 1, 2, 3, 4
run: $(EXEC_NEFDAN)
	./$(EXEC_NEFDAN) 1
	./$(EXEC_NEFDAN) 2
	./$(EXEC_NEFDAN) 3
	./$(EXEC_NEFDAN) 4

# Run Bitcoin simulation without args
run_btc: $(EXEC_BTC)
	./$(EXEC_BTC)

# Run everything: the original run targets + btc + eth
run_all: run run_btc

# Clean up all generated executables
clean:
	rm -f $(ALL_EXECS)

.PHONY: all run run_btc run_all clean