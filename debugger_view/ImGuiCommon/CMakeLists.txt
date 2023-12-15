#set(CMAKE_CXX_STANDARD 17)
add_library(ImGuiCommon STATIC ImGuiCommon.cpp ImGuiCommon.h)
target_link_libraries(ImGuiCommon PRIVATE ${CMAKE_PROJECT_NAME})
target_include_directories(ImGuiCommon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}) # need this??