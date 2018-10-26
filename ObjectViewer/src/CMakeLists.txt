cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

#some policy stuff
foreach(p
    CMP0071 # 3.10: Let AUTOMOC and AUTOUIC process GENERATED files
    )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

project(ObjectViewer)

# Finding the VTK library.
find_package(VTK REQUIRED NO_MODULE)
include(${VTK_USE_FILE})


# Setting a few variables for us.
set( Srcs main.cxx ObjectViewer.cxx)

# maybe organize headers and cxx in different folders?
set( Hdrs ObjectViewer.h )

set( MOC_Hdrs ObjectViewer.h )

set( UIs ObjectViewer.ui)

set( YAML-CPP_LIB "C:\\YAML\\bin\\Debug\\yaml-cpp.lib")
set( VTK_DIR "C:\\VTK\\build")

# Use the include path and library for Qt that is used by VTK.
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)


# Finding the QT library.
find_package(Qt5 COMPONENTS Widgets)

# We need to wrap our .ui-sources!
qt5_wrap_ui(UI_Srcs ${UIs})

add_executable(ObjectViewer
  MACOSX_BUNDLE  ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})

qt5_use_modules(ObjectViewer Core Gui Widgets)

target_link_libraries(ObjectViewer ${VTK_LIBRARIES} ${YAML-CPP_LIB})

target_include_directories(ObjectViewer PUBLIC "C:\\YAML\\src\\include")