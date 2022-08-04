cmake_minimum_required(VERSION 3.14)
option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
project(i8080GUI LANGUAGES CXX)
set( QtBaseDir "c:/program files (x86)/Qt")
LIST(APPEND CMAKE_PREFIX_PATH "${QtBaseDir}/lib/cmake/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(i8080GUI
    i8080GUI.cpp i8080GUI.h QtProj.ui
    main.cpp
)
set_target_properties(i8080GUI PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(i8080GUI PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

message(STATUS "linking with directory : ${i8080IncludeDir}")
target_include_directories(i8080GUI PUBLIC ${i8080IncludeDir}) 
#install(FILES ${i8080IncludeDir}/$<CONFIG>/commonCode.dll DESTINATION bin)
target_link_libraries(i8080GUI PUBLIC commonCode)
#add_library(commonCode MinecraftTool.cpp MinecraftTool.h)

install(TARGETS i8080GUI DESTINATION bin)

set( DLLS "Qt6Core" "Qt6Gui" "Qt6Widgets" )
foreach(dll IN LISTS DLLS )
install(FILES "${QtBaseDir}/bin/${dll}d.dll" DESTINATION bin)
endforeach()
		
install(FILES "${QtBaseDir}/plugins/platforms/qwindowsd.dll"
		DESTINATION bin/platforms )

include(InstallRequiredSystemLibraries)
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")

#if(WIN32)
#	set( QtBaseDir "C:/Program Files (x86)/Qt" )
#	set( DLLS "Qt6Core" "Qt6Gui" "Qt6Widgets" "Qt6OpenGL" "Qt6OpenGLWidgets" )
#	
#	foreach(dll IN LISTS DLLS)
#		install(FILES "${QtBaseDir}/bin/${dll}.dll" DESTINATION bin)
#	endforeach()
#	
#	install(FILES "${QtBaseDir}/plugins/platforms/qwindowsd.dll" DESTINATION bin)
#endif()

set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
