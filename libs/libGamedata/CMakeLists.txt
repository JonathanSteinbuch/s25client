set(SOURCES_SUBDIRS )
macro(AddDirectory dir)
    file(GLOB SUB_FILES ${dir}/*.cpp ${dir}/*.h ${dir}/*.hpp ${dir}/*.tpp)
    set(SOURCES_SUBDIRS ${SOURCES_SUBDIRS} ${SUB_FILES})
    source_group(${dir} FILES ${SUB_FILES})
endmacro()

AddDirectory(gameData)
AddDirectory(lua)

file(GLOB SOURCES_OTHER *.cpp *.h)
source_group(other FILES ${SOURCES_OTHER})

find_package(Lua 5.1 REQUIRED)

include(GatherDll)
gather_dll(Lua)

add_library(gamedata STATIC ${SOURCES_OTHER} ${SOURCES_SUBDIRS})
target_include_directories(gamedata PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gamedata SYSTEM PUBLIC ${LUA_INCLUDE_DIR})
target_link_libraries(gamedata PUBLIC
    s25util::common
    mygettext
    s25Common
    kaguya
    ${LUA_LIBRARIES}
    PRIVATE rttrConfig Boost::nowide
)
include(EnableWarnings)
enable_warnings(gamedata)
