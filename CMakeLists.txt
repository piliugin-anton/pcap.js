cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0042 NEW)

project(pcapjs)

add_definitions(-DNAPI_VERSION=7)

file(GLOB SOURCE_FILES "src/*.cc")

include(ExternalProject)

set(DEPS_INSTALL_DIR ${CMAKE_BINARY_DIR}/install)
set(DEPS_BIN ${DEPS_INSTALL_DIR}/bin)
set(DEPS_LIBS ${DEPS_INSTALL_DIR}/lib)
set(DEPS_INCLUDE ${DEPS_INSTALL_DIR}/include)

ExternalProject_Add(flex
    PREFIX flex
	URL https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz
	URL_MD5 2882e3179748cc9f9c23ec593d6adc8d
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${DEPS_INSTALL_DIR}
)

ExternalProject_Add(bison
    PREFIX bison
	URL https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.gz
	URL_MD5 1e541a097cda9eca675d29dd2832921f
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${DEPS_INSTALL_DIR}
)

ExternalProject_Add(libnl
    PREFIX libnl
	URL https://github.com/thom311/libnl/releases/download/libnl3_7_0/libnl-3.7.0.tar.gz
	URL_MD5 b381405afd14e466e35d29a112480333
	CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PATH=${DEPS_BIN}:$ENV{PATH} <SOURCE_DIR>/configure --prefix=${DEPS_INSTALL_DIR}
	BUILD_COMMAND ${CMAKE_COMMAND} -E env PATH=${DEPS_BIN}:$ENV{PATH} $(MAKE)
)
ExternalProject_Add_StepDependencies(libnl install flex bison)

ExternalProject_Add(libpcap
    PREFIX libpcap
	URL https://github.com/the-tcpdump-group/libpcap/archive/refs/tags/libpcap-1.10.2.tar.gz
	URL_MD5 248498d9f46deae82ed14315d3e2922e
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PATH=${DEPS_BIN}:$ENV{PATH} <SOURCE_DIR>/configure --prefix=${DEPS_INSTALL_DIR} --with-libnl=yes "LDFLAGS=-Wl,-rpath,${DEPS_LIBS}" "PKG_CONFIG_PATH=${DEPS_LIBS}/pkgconfig"
	BUILD_COMMAND ${CMAKE_COMMAND} -E env PATH=${DEPS_BIN}:$ENV{PATH} $(MAKE)
)
ExternalProject_Add_StepDependencies(libpcap install libnl)
set(LIBPCAP_LIBRARY ${DEPS_LIBS}/libpcap.so)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_compile_options(${PROJECT_NAME} PRIVATE -fexceptions PRIVATE -fvisibility=hidden)
target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_DISABLE_CPP_EXCEPTIONS PRIVATE NODE_ADDON_API_ENABLE_MAYBE PRIVATE NODE_ADDON_API_DISABLE_DEPRECATED)

execute_process(COMMAND node -p "require('node-addon-api').include" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REGEX REPLACE "[\r\n\"]" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${DEPS_INCLUDE} PRIVATE ${NODE_ADDON_API_DIR} PRIVATE ${CMAKE_JS_INC})
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} ${LIBPCAP_LIBRARY})