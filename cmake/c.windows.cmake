SET(Boost_COMPILER "-mgw44")
SET(BOOST_ROOT ${CMAKE_PREFIX_PATH})
SET(Boost_USE_STATIC_RUNTIME TRUE)

# disable rsp files to allow ccache to work
SET(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES OFF)