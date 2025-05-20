# Define the compiler and flags
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra

# Name of the executable and source file
EXEC = simulation_with_virtual_difficulties_and_noise
SRC = simulation_with_virtual_difficulties_and_noise.cpp
# EXEC = simulation
# SRC = simulation.cpp

# Default target: compile the program
all: $(EXEC)

# Compile the program
$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(SRC)

# Run the program with parameters 1, 2, 3, and 4
run: $(EXEC)
	./$(EXEC) 1
	./$(EXEC) 2
	./$(EXEC) 3
	./$(EXEC) 4

# Clean up generated files
clean:
	rm -f $(EXEC)
