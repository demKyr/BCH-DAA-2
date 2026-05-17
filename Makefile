# Define the compiler and flags
CXX = clang++
CXXFLAGS = -std=c++23 -stdlib=libc++ -Wall -O2

# Names of all the executables
EXEC_NEFDAN = simulation_NefdaN
EXEC_BTC   = simulation_BTC_DAA
EXEC_LWMA  = simulation_LWMA_DAA
EXEC_DIGISHIELD = simulation_Digishield_DAA

ALL_EXECS = $(EXEC_NEFDAN) $(EXEC_BTC) $(EXEC_LWMA) $(EXEC_DIGISHIELD)

# Default target: compile all programs when running "make"
all: $(ALL_EXECS)

# Compile rules for each individual executable
# FIX: Removed hashrate.cpp from the compilation call, but kept it as a tracking dependency
$(EXEC_NEFDAN): simulation_NefdaN.cpp hashrate.cpp hashrate.h
	$(CXX) $(CXXFLAGS) -o $(EXEC_NEFDAN) simulation_NefdaN.cpp

$(EXEC_BTC): simulation_BTC_DAA.cpp hashrate.cpp hashrate.h
	$(CXX) $(CXXFLAGS) -o $(EXEC_BTC) simulation_BTC_DAA.cpp

$(EXEC_LWMA): simulation_LWMA_DAA.cpp hashrate.cpp hashrate.h
	$(CXX) $(CXXFLAGS) -o $(EXEC_LWMA) simulation_LWMA_DAA.cpp

$(EXEC_DIGISHIELD): simulation_Digishield_DAA.cpp hashrate.cpp hashrate.h
	$(CXX) $(CXXFLAGS) -o $(EXEC_DIGISHIELD) simulation_Digishield_DAA.cpp


# Run original simulation with parameters 1, 2, 3, 4
run: $(EXEC_NEFDAN)
	./$(EXEC_NEFDAN) 1
	./$(EXEC_NEFDAN) 2
	./$(EXEC_NEFDAN) 3
	./$(EXEC_NEFDAN) 4

# Run Bitcoin simulation without args
run_btc: $(EXEC_BTC)
	./$(EXEC_BTC)

# Run LWMA simulation without args
run_lwma: $(EXEC_LWMA)
	./$(EXEC_LWMA)

# Run DigiShield simulation without args
run_digishield: $(EXEC_DIGISHIELD)
	./$(EXEC_DIGISHIELD)

# Run everything: the original run targets + btc + lwma + digishield
run_all: run run_btc run_lwma run_digishield

# Clean up all generated executables
clean:
	rm -f $(ALL_EXECS)

.PHONY: all run run_btc run_lwma run_digishield run_all clean