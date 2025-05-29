file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS Common/Source/*.cpp)

# This custom target represents a task that copies all demo assets to the output directory
add_custom_target(DemoResources ALL)

# This function generates an executable target from the files in the specified directory
function(define_demo DIR)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${DIR}/*.cpp)
    add_executable(${DIR} WIN32 ${SOURCES} ${COMMON_SOURCES})
    target_include_directories(${DIR} PRIVATE ${DIR}/Include Common/Include)
    target_link_libraries(${DIR} PRIVATE SexyAppFramework)
    add_dependencies(${DIR} DemoResources)
endfunction()

# Define all the demos
define_demo(Demo1)
define_demo(Demo2)
define_demo(Demo3)
define_demo(Demo4)
define_demo(Demo5)
define_demo(V12Demo)
define_demo(XMLDemo)
define_demo(Hungarr)

# Definition for the demo resources copy target
add_custom_command(TARGET DemoResources POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/bass.dll $<TARGET_FILE_DIR:Demo1> # BASS DLL for audio
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Common/Resources/ $<TARGET_FILE_DIR:Demo1> # Resource files
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/dxwrapper/ $<TARGET_FILE_DIR:Demo1> # DXWrapper for compatibility
)
