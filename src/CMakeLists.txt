CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

if (UNIX)
#[[add_definitions(
        -Werror=format
)]]
endif ()

ADD_SUBDIRECTORY("NFrame")
ADD_SUBDIRECTORY("NFTest")
ADD_SUBDIRECTORY("NFServer")


