add_executable(SexyTools Source/main.cpp Source/SexyTools.cpp Source/PakTool.cpp)
target_include_directories(SexyTools PRIVATE Include ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(SexyTools PRIVATE ${wxWidgets_LIBRARIES} SexyAppFramework)