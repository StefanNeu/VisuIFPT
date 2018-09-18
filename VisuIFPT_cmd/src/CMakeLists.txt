cmake_minimum_required(VERSION 2.8)
 
PROJECT(ifptReader)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(ifptReader MACOSX_BUNDLE ifptReader.cxx)
 
if(VTK_LIBRARIES)
  target_link_libraries(ifptReader ${VTK_LIBRARIES})
else()
  target_link_libraries(ifptReader vtkHybrid vtkWidgets)
endif()