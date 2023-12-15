set(ProId Picker)

file(GLOB_RECURSE header_file "*.h")
file(GLOB source_file "*.cpp" "*.cxx")
add_executable(${ProId} ${header_file} ${source_file})

target_link_libraries(${ProId} PRIVATE ${CMAKE_PROJECT_NAME} ImGuiCommon)

if (VTK_VERSION VERSION_LESS "9.0.0")
include(${VTK_USE_FILE})
else()
# vtk_module_autoinit is needed
vtk_module_autoinit(
TARGETS ${ProId}
MODULES ${VTK_LIBRARIES}
)
endif()

set_target_properties(${ProId}
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${Bin_Root}
ARCHIVE_OUTPUT_DIRECTORY ${Lib_Root}
VS_DEBUGGER_WORKING_DIRECTORY ${Bin_Root}/$<CONFIG>
)