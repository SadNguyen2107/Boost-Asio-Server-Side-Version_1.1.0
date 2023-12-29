# Configuration File For Compile And Linking In Linux x86_64 OS And Windows_NT
# Compiler setting: GCC 13.1.0 
# Using std=c++20 up and beyond

# TARGET FILE
TARGET = main

ifeq ($(shell uname -s),Linux)
# Configuration File For Compile And Linking In Linux x86_64 OS 
CXX 		= g++
CXX_FLAGS 	= -std=c++20 -Wall -Werror
STD_LIBS 	= -lsqlite3 -lcrypto -lboost_system -lboost_filesystem

# Current Directory
CURRENT_PATH 	= $(shell pwd)

# Directories To Build
BIN_DIR 		= bin
OBJ_DIR 		= obj
LIB_DIR 		= lib
LIBS_CPP_DIR 	= libs_cpp
KEY_DIR 		= key

# Directories Store Code
INCLUDE_DIR 	= include
SRC_DIR 		= src

# All The .cpp file (Not Link)
SRCS 					= $(wildcard $(SRC_DIR)/*.cpp)
LIBS_CPP 				= $(wildcard $(LIBS_CPP_DIR)/*.cpp)
CUSTOM_DYNAMIC_LIBS		= $(patsubst %, $(BIN_DIR)/lib%.so, $(basename $(notdir $(LIBS_CPP)))) 
CUSTOM_STATIC_LIBS 		= $(patsubst %, $(LIB_DIR)/lib%.a, $(basename $(notdir $(LIBS_CPP))))
OBJS 					= $(patsubst %, $(OBJ_DIR)/%, $(notdir $(SRCS:.cpp=.o)))

# Change the Linker to add the CUSTOM LIBS
LINK_LIBS 		= $(addprefix -l, $(basename $(notdir $(LIBS_CPP)))) 

# Common Commands
MK_DIR 	= mkdir -p
RM_DIR 	= rm -rf

# Build Target
all: build_directories $(TARGET)

#--------------------------------------------------------------------------------------------
# Make Executable Files
$(TARGET): $(OBJS) $(CUSTOM_DYNAMIC_LIBS) $(CUSTOM_STATIC_LIBS)
	LD_LIBRARY_PATH="$(CURRENT_PATH)/$(BIN_DIR)" $(CXX) -o $(BIN_DIR)/$@ $(OBJS) $(LINK_LIBS) $(STD_LIBS) -L"$(CURRENT_PATH)/$(BIN_DIR)"

# Compile .c and .cpp code into object files
# Compile Source Files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@ 

# Compile .cpp code in libs into .o objects files
# Compile libs_cp -> .o file to make static_lib
$(LIB_DIR)/%.o: $(LIBS_CPP_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@ 

# Build Static Libraries
$(LIB_DIR)/lib%.a: $(LIB_DIR)/%.o
	ar rcs $@ $<

# Build Dynamic Libraries
$(BIN_DIR)/lib%.so: $(LIBS_CPP_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -fPIC -shared -o $@ $< 

#--------------------------------------------------------------------------------------------

# Make Directories To Store Library And Info Required
.PHONY: build_directories
build_directories: 
	@$(MK_DIR) $(BIN_DIR) $(OBJ_DIR) $(LIB_DIR) $(KEY_DIR)

# Run this command to run the program
.PHONY: run
run: 
	LD_LIBRARY_PATH="$(CURRENT_PATH)/$(BIN_DIR)" ./$(BIN_DIR)/$(TARGET) $(LINK_LIBS) $(STD_LIBS) -L"$(CURRENT_PATH)/$(BIN_DIR)"

# Run this command to check for memory leak
.PHONY: valgrind
valgrind: 
	LD_LIBRARY_PATH="$(CURRENT_PATH)/$(BIN_DIR)" valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-log.log  ./$(BIN_DIR)/$(TARGET) $(LINK_LIBS) 

# To File all the Linker Libraries Need Before Run
.PHONY: shared
shared: 
	readelf -a $(BIN_DIR)/$(TARGET) | grep Shared

# Clean all the Production Files
.PHONY: clean
clean:
	@echo clean...
	@$(RM_DIR) $(BIN_DIR) $(OBJ_DIR) $(LIB_DIR) $(KEY_DIR)

# Clean all the Production Files And Clear Terminal
.PHONY: clear
clear:
	@$(RM_DIR) $(BIN_DIR) $(OBJ_DIR) $(LIB_DIR) $(KEY_DIR)
	@clear

# For Lazy To Do Git Command
.PHONY: lazy_git
COMMIT_MESSAGE ?= $(shell bash -c 'read -p "Commit Message: " commit_message; echo $$commit_message')
lazy_git:
	git pull
	git add README.md makefile .gitignore $(INCLUDE_DIR) $(SRC_DIR) $(LIBS_CPP)
	git commit -m "$(COMMIT_MESSAGE)"
	git push -u origin 

else ifeq ($(OS),Windows_NT)
# Configuration File For Compile And Linking In Windows_NT 
CXX 		= x86_64-w64-mingw32-g++
CXX_FLAGS 	= -std=c++20 -Wall -Werror
STD_LIBS 	= -lsqlite3 -lcrypto -lboost_system-mt -lboost_filesystem-mt -lwsock32 -lws2_32

# Current Directory
CURRENT_PATH 	= $(shell pwd)

# Directories To Build
BIN_DIR 		= bin
OBJ_DIR 		= obj
LIB_DIR 		= lib
LIBS_CPP_DIR 	= libs_cpp
KEY_DIR 		= key

# Directories Store Code
INCLUDE_DIR 	= include
SRC_DIR 		= src

# All The .cpp file (Not Link)
SRCS 					= $(wildcard $(SRC_DIR)/*.cpp)
LIBS_CPP 				= $(wildcard $(LIBS_CPP_DIR)/*.cpp)
CUSTOM_DYNAMIC_LIBS		= $(patsubst %, $(BIN_DIR)/lib%.dll, $(basename $(notdir $(LIBS_CPP)))) 
CUSTOM_STATIC_LIBS 		= $(patsubst %, $(LIB_DIR)/lib%.a, $(basename $(notdir $(LIBS_CPP))))
OBJS 					= $(patsubst %, $(OBJ_DIR)/%, $(notdir $(SRCS:.cpp=.o)))

# Change the Linker to add the CUSTOM LIBS
LINK_LIBS 		= $(addprefix -l, $(basename $(notdir $(LIBS_CPP))))

# Common Commands
MK_DIR 	= mkdir -p
RM_DIR 	= rm -rf

# Build Target
all: build_directories $(TARGET)

#--------------------------------------------------------------------------------------------
# Make Executable Files
$(TARGET): $(OBJS) $(CUSTOM_DYNAMIC_LIBS) $(CUSTOM_STATIC_LIBS)
	LD_LIBRARY_PATH="$(CURRENT_PATH)/$(BIN_DIR)" $(CXX) -o $(BIN_DIR)/$@ $(OBJS) $(LINK_LIBS) $(STD_LIBS) -L"$(CURRENT_PATH)/$(BIN_DIR)"

# Compile .c and .cpp code into object files
# From src Folder
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@ 

# Compile .cpp code in libs into .o objects files
# Compile into.o files to make static_lib
$(LIB_DIR)/%.o: $(LIBS_CPP_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@ 

# Build Static Libraries
$(LIB_DIR)/lib%.a: $(LIB_DIR)/%.o
	ar rcs $@ $<

$(BIN_DIR)/libencode_decode_base64.dll: $(LIBS_CPP_DIR)/encode_decode_base64.cpp
	$(CXX) $(CXX_FLAGS) -fPIC -shared -o $@ $< 

$(BIN_DIR)/libsimdjson.dll: $(LIBS_CPP_DIR)/simdjson.cpp
	$(CXX) $(CXX_FLAGS) -fPIC -shared -o $@ $< 

$(BIN_DIR)/libServer.dll: $(LIBS_CPP_DIR)/Server.cpp
	$(CXX) $(CXX_FLAGS) -fPIC -shared -o $@ $< -lencode_decode_base64 $(STD_LIBS) -L"$(CURRENT_PATH)/$(BIN_DIR)"

#--------------------------------------------------------------------------------------------

# Make Directories To Store Library And Info Required
.PHONY: build_directories
build_directories: 
	@$(MK_DIR) $(BIN_DIR) $(OBJ_DIR) $(LIB_DIR) $(KEY_DIR)

# Run this command to run the program
.PHONY: run
run: 
	LD_LIBRARY_PATH="$(CURRENT_PATH)/$(BIN_DIR)" ./$(BIN_DIR)/$(TARGET) $(LINK_LIBS) $(STD_LIBS) -L"$(CURRENT_PATH)/$(BIN_DIR)"

# Clean all the Production Files
.PHONY: clean
clean:
	@echo clean...
	@$(RM_DIR) $(BIN_DIR) $(OBJ_DIR) $(LIB_DIR) $(KEY_DIR)

# Clean all the Production Files And Clear Terminal
.PHONY: clear
clear:
	@$(RM_DIR) $(BIN_DIR) $(OBJ_DIR) $(LIB_DIR) $(KEY_DIR)
	@clear

# For Lazy To Do Git Command
.PHONY: lazy_git
COMMIT_MESSAGE ?= $(shell powershell -Command "Read-Host -Prompt 'Commit Message'")
lazy_git:
	git pull
	git add README.md makefile .gitignore $(INCLUDE_DIR) $(SRC_DIR) $(LIBS_CPP)
	git commit -m "$(COMMIT_MESSAGE)"
	git push -u origin
	
endif