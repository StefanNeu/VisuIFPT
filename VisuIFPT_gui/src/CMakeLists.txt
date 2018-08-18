cmake_minimum_required(VERSION 3.3...3.12 FATAL_ERROR)
foreach(p
    CMP0071 # 3.10: Let AUTOMOC and AUTOUIC process GENERATED files
    )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

project(VisuIFPT_gui)

find_package(VTK REQUIRED NO_MODULE)
include(${VTK_USE_FILE})

set( Srcs main.cxx GUI.cxx Reader.cxx )

set( Hdrs GUI.h Reader.h)

set( MOC_Hdrs GUI.h )

set( UIs GUI.ui )

# Use the include path and library for Qt that is used by VTK.
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets)
qt5_wrap_ui(UI_Srcs ${UIs})

add_executable(VisuIFPT_gui
  MACOSX_BUNDLE  ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})

qt5_use_modules(VisuIFPT_gui Core Gui Widgets)

target_link_libraries(VisuIFPT_gui ${VTK_LIBRARIES})