cmake_minimum_required(VERSION 3.2)

project(hl_mai_lab_05 C CXX)

SET (USER_SERVICE "user_service")
SET (CONFERENCE_SERVICE "conference_service")
SET (LECTURE_SERVICE "lecture_service")
SET (LOAD_INIT_DATA "load_data")
SET (WRITER_BINARY  "user_writer")

SET (STD_CXX "c++17")
SET (REDISCPP_FLAGS "-DREDISCPP_HEADER_ONLY=ON")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=${STD_CXX} ${REDISCPP_FLAGS}")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -std=${STD_CXX} -Wall -DNDEBUG ${REDISCPP_FLAGS}")

find_package(OpenSSL)
find_package(Threads)
find_package(ZLIB)
find_package(CppKafka REQUIRED)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL)


if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})
include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
link_directories("/usr/local/lib")

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${USER_SERVICE} services/user/main.cpp 
                                 config/config.cpp 
                                 database/cache.cpp
                                 database/database.cpp
                                 database/user.cpp
                                 database/conference.cpp
                                 database/lecture.cpp)


target_include_directories(${USER_SERVICE} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${USER_SERVICE} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${USER_SERVICE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${USER_SERVICE} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(${USER_SERVICE} PRIVATE 
                             ${CMAKE_THREAD_LIBS_INIT} 
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "mysqlclient"
                             "rdkafka++"
                             CppKafka::cppkafka  
                             ZLIB::ZLIB)




add_executable(${CONFERENCE_SERVICE} services/conference/main.cpp
                                 config/config.cpp 
                                 database/database.cpp
                                 database/cache.cpp
                                 database/user.cpp
                                 database/conference.cpp
                                 database/lecture.cpp)

target_include_directories(${CONFERENCE_SERVICE} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${CONFERENCE_SERVICE} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${CONFERENCE_SERVICE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${CONFERENCE_SERVICE} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(${CONFERENCE_SERVICE} PRIVATE 
                             ${CMAKE_THREAD_LIBS_INIT} 
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "mysqlclient"
                             "rdkafka++"
                             CppKafka::cppkafka  
                             ZLIB::ZLIB)




add_executable(${LECTURE_SERVICE} services/lecture/main.cpp
                                 config/config.cpp 
                                 database/database.cpp
                                 database/cache.cpp
                                 database/user.cpp
                                 database/lecture.cpp
                                 database/conference.cpp)

target_include_directories(${LECTURE_SERVICE} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${LECTURE_SERVICE} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${LECTURE_SERVICE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${LECTURE_SERVICE} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(${LECTURE_SERVICE} PRIVATE 
                         ${CMAKE_THREAD_LIBS_INIT} 
                         ${Poco_LIBRARIES}
                         "PocoData"
                         "PocoDataMySQL"
                         "mysqlclient"
                         "rdkafka++"
                         CppKafka::cppkafka  
                         ZLIB::ZLIB)




add_executable(${LOAD_INIT_DATA} load_data/load_data.cpp
                         config/config.cpp
                         database/cache.cpp 
                         database/database.cpp
                         database/user.cpp)

target_include_directories(${LOAD_INIT_DATA} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${LOAD_INIT_DATA} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${LOAD_INIT_DATA} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${LOAD_INIT_DATA} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(${LOAD_INIT_DATA} PRIVATE 
                     ${CMAKE_THREAD_LIBS_INIT} 
                     ${Poco_LIBRARIES}
                     "PocoData"
                     "PocoDataMySQL"
                     "mysqlclient"
                     "rdkafka++"
                     CppKafka::cppkafka
                     ZLIB::ZLIB)                        




add_executable(${WRITER_BINARY} writer/main.cpp 
                     config/config.cpp 
                     database/cache.cpp
                     database/database.cpp
                     database/user.cpp)

target_link_libraries(${WRITER_BINARY} PRIVATE 
                     ${CMAKE_THREAD_LIBS_INIT} 
                     ${Poco_LIBRARIES}
                     "PocoData"
                     "PocoDataMySQL"
                     "mysqlclient"
                     "rdkafka++"
                     CppKafka::cppkafka
                     ZLIB::ZLIB)      

target_include_directories(${WRITER_BINARY} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${WRITER_BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${WRITER_BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${WRITER_BINARY} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)                    
