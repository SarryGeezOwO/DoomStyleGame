CXX 		:= g++
CXXVER      := -std=c++17
CXX_COMMON_FLAGS := -pipe
DEBUG_CXXFLAGS := $(CXXVER) $(CXX_COMMON_FLAGS) \
    -Wall -Wextra -Wpedantic \
    -Wno-float-equal \
    -Wshadow -Wcast-align \
    -Wformat=2 -Wno-unused-parameter \
    -g \
	-DGZ_BUILD_DEBUG \
    -fno-omit-frame-pointer \
    -fno-inline       

RELEASE_CXXFLAGS := $(CXXVER) \
    -O2 -DNDEBUG \
    -march=native \
    -fomit-frame-pointer \
    -ffunction-sections -fdata-sections \
    -fno-rtti -fno-exceptions \
    -s

SRC_DIR     := app/src
OBJ_DIR     := out/build
BIN_DIR     := out/bin
INCLUDE_DIR	:= vendor/include
LIBS_DIR 	:= vendor/lib

APP_NAME	:= DoomStyleGame
EXE         := $(BIN_DIR)/$(APP_NAME).exe

# Put all src sub directories in SRC_DIRS
SRC_DIRS 	:= $(SRC_DIR) $(SRC_DIR)/core $(SRC_DIR)/renderer $(SRC_DIR)/util $(SRC_DIR)/resource $(SRC_DIR)/gmp $(SRC_DIR)/physics
SRC 		:= $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))
OBJ 		:= $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(subst $(SRC_DIR)/,,$(SRC)))

# All sub folder names under the "external/include" directory
# SDL3 SDL3_image Eigen imgui 								<- example on INCLUDE_DIRS
# $(foreach dir,$(INCLUDE_DIRS),-I$(INCLUDE_DIR)/$(dir)) 	<- add this on INCLUDE
# INCLUDE_DIRS := 			
INCLUDE_DIRS := SDL3 GL glm SDL3_mixer earcut
INCLUDE_SRC_DIRS := core renderer util resource gmp physics
LIB_DIRS 	 := SDL3 glew32 opengl32 SDL3_mixer
INCLUDE      := -I$(INCLUDE_DIR) $(foreach dir, $(INCLUDE_DIRS), -I$(INCLUDE_DIR)/$(dir)) -I$(SRC_DIR) $(foreach dir, $(INCLUDE_SRC_DIRS), -I$(SRC_DIR)/$(dir))
LIB	         := -L$(LIBS_DIR) $(foreach dir, $(LIB_DIRS), -l$(dir))

.PHONY: all clean run help

all: debug ## Build (default -> debug)

# Deletes .exe and Object files
clean: ## Remove build artifacts
	rm -rf $(OBJ_DIR)/*.o $(OBJ_DIR)/*/*.o $(EXE)

# Debug build
debug: CXXFLAGS := $(DEBUG_CXXFLAGS)
debug: $(EXE) ## Build debug binary (with GZ_BUILD_DEBUG)

# Release build
release: CXXFLAGS := $(RELEASE_CXXFLAGS)
release: $(EXE) ## Build release binary

$(EXE) : $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIB)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE) $(LIB)

run: ## Run the built executable
	$(EXE)

help: ## Show this help
	@printf "\nUsage: make [target]\n\n"
	@printf "Available targets:\n\n"
	@awk 'BEGIN {FS = ":.*?## "}; /^[a-zA-Z0-9._-]+:.*##/ { printf "  %-12s %s\n", $$1, $$2 }' $(MAKEFILE_LIST) | sort
	@printf "\n"