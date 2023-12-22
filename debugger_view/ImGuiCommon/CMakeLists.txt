#set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE header_file "*.h" "implot-0.16/*.h")
file(GLOB source_file "*.cpp" "*.cxx" "implot-0.16/*.cpp")
add_library(ImGuiCommon STATIC ${header_file} ${source_file})
target_link_libraries(ImGuiCommon PRIVATE ${CMAKE_PROJECT_NAME})
target_include_directories(ImGuiCommon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}) # need this??
target_include_directories(ImGuiCommon INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/implot-0.16") # need this??