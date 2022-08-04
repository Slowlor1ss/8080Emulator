#Common build file
#add_library(MinecraftTool SHARED MinecraftTool.cpp)
add_library(commonCode 
8080/CPU.cpp 8080/CPU.h 
8080/Display.cpp 8080/Display.h 
8080/i8080Emulator.cpp 8080/i8080Emulator.h 
8080/Keyboard.cpp 8080/Keyboard.h 
8080/ConsoleWindow.cpp 8080/ConsoleWindow.h 
)

#set a variable in parent scope with the dir of include file/header files
#CMAKE_CURRENT_SOURCE_DIR
set(i8080IncludeDir "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
target_compile_features(commonCode PUBLIC cxx_std_23)
