
cmake_minimum_required(VERSION 3.10)
project(Stm32Fingerprint)

file(GLOB_RECURSE SOURCES
        "src/*.*"
)

add_library(Stm32Fingerprint STATIC ${SOURCES})

target_include_directories(Stm32Fingerprint PUBLIC
        src
)
