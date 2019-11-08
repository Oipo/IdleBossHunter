cmake_minimum_required(VERSION 3.12)
project(ibh_frontend)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE CXX STANDARD REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING
            "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
            FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_CXX_COMPILER em++)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/frontend/bin)
set(TOP_DIR ${CMAKE_SOURCE_DIR})
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external/)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# only support linux/g++ for now
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXXH_INLINE_ALL -DXXH_CPU_LITTLE_ENDIAN=1 -DRAPIDJSON_SSE42 -DSPDLOG_COMPILED_LIB -DCATCH_CONFIG_FAST_COMPILE -DSPDLOG_NO_EXCEPTIONS -Wall -Wextra -Wno-unused-variable -Wno-long-long -Wno-unused-parameter -Wnull-dereference -pedantic -std=c++17 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"bmp\",\"png\",\"xpm\"]' -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -s FULL_ES2=1 -s FULL_ES3=1 -s USE_WEBGL2=1") #-fsanitize=undefined -fsanitize=thread -fstack-protector-strong -fno-omit-frame-pointer ")
set(CMAKE_CXX_FLAGS_DEBUG "-g4 --source-map-base http://localhost/")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g4 -Os --source-map-base http://localhost/")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

file(GLOB_RECURSE PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB SPDLOG_SOURCE ${EXTERNAL_DIR}/spdlog/src/*.cpp)
file(GLOB ZstdCommonSources ${LIBRARY_DIR}/common/*.c)
file(GLOB ZstdCompressSources ${LIBRARY_DIR}/compress/*.c)
file(GLOB ZstdDecompressSources ${LIBRARY_DIR}/decompress/*.c)
file(GLOB ZstdDictBuilderSources ${LIBRARY_DIR}/dictBuilder/*.c)
add_executable(ibh_frontend.html ${SPDLOG_SOURCE} ${PROJECT_SOURCES} ${ZstdCommonSources} ${ZstdCompressSources} ${ZstdDecompressSources} ${ZstdDictBuilderSources})

file(GLOB_RECURSE PROJECT_SOURCES_WITHOUT_MAIN ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM PROJECT_SOURCES_WITHOUT_MAIN "${PROJECT_SOURCE_DIR}/src/main.cpp")
file(GLOB_RECURSE PROJECT_TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/*.cpp)
add_executable(ibh_frontend_test ${SPDLOG_SOURCE} ${PROJECT_SOURCES_WITHOUT_MAIN} ${PROJECT_TEST_SOURCES})
add_test(NAME MyTest COMMAND ibh_frontend_test)

file(GLOB_RECURSE PROJECT_SOURCES_WITHOUT_MAIN ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM PROJECT_SOURCES_WITHOUT_MAIN "${PROJECT_SOURCE_DIR}/src/main.cpp")
file(GLOB PROJECT_BENCHMARK_SOURCES ${PROJECT_SOURCE_DIR}/benchmark/*.cpp ${PROJECT_SOURCE_DIR}/benchmark/benchmark_helpers/*.cpp)
add_executable(ibh_frontend_benchmark ${SPDLOG_SOURCE} ${PROJECT_SOURCES_WITHOUT_MAIN} ${PROJECT_BENCHMARK_SOURCES})

target_compile_definitions(ibh_frontend_test PRIVATE TEST_CODE=1)

include_directories("${TOP_DIR}/frontend/src")
include_directories("${EXTERNAL_DIR}/glm")
include_directories("${EXTERNAL_DIR}/zstd/lib")
include_directories("${EXTERNAL_DIR}/zstd/lib/common")
include_directories("${EXTERNAL_DIR}/spdlog/include")
include_directories("${EXTERNAL_DIR}/rapidjson/include")
include_directories("${EXTERNAL_DIR}/range-v3/include")
include_directories("${EXTERNAL_DIR}/Catch2/single_include")
include_directories("${EXTERNAL_DIR}/xxHash")
include_directories("${EXTERNAL_DIR}/robin-hood-hashing/src/include")
include_directories("${EXTERNAL_DIR}/entt/src")
include_directories("${EXTERNAL_DIR}/pcg-cpp/include")
include_directories("${EXTERNAL_DIR}/concurrentqueue")

#set_target_properties(ibh_frontend.html PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"bmp\",\"png\",\"xpm\"] -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 --pre-js pre-module.js --post-js post-module.js -s ASSERTIONS=1 -s GL_ASSERTIONS=1")
target_link_libraries(ibh_frontend.html PUBLIC -lpthread)
target_link_libraries(ibh_frontend.html PUBLIC ${ZLIB_LIBRARIES} )
target_link_libraries(ibh_frontend.html PUBLIC ${OPENSSL_LIBRARIES})
target_link_libraries(ibh_frontend.html PUBLIC ${XXHASH_LIBRARY})
target_link_libraries(ibh_frontend.html PUBLIC -static-libgcc -static-libstdc++)

target_link_libraries(ibh_frontend_test PUBLIC -lpthread)
target_link_libraries(ibh_frontend_test PUBLIC ${ZLIB_LIBRARIES} )
target_link_libraries(ibh_frontend_test PUBLIC ${OPENSSL_LIBRARIES})
target_link_libraries(ibh_frontend_test PUBLIC ${XXHASH_LIBRARY})
target_link_libraries(ibh_frontend_test PUBLIC -static-libgcc -static-libstdc++)

target_link_libraries(ibh_frontend_benchmark PUBLIC -lpthread)
target_link_libraries(ibh_frontend_benchmark PUBLIC ${ZLIB_LIBRARIES} )
target_link_libraries(ibh_frontend_benchmark PUBLIC ${OPENSSL_LIBRARIES})
target_link_libraries(ibh_frontend_benchmark PUBLIC ${XXHASH_LIBRARY})
target_link_libraries(ibh_frontend_benchmark PUBLIC -static-libgcc -static-libstdc++)

add_custom_command(
        TARGET ibh_frontend.html POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/
        ${CMAKE_BINARY_DIR}/assets)

add_custom_command(
        TARGET ibh_frontend_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/
        ${CMAKE_BINARY_DIR}/assets)