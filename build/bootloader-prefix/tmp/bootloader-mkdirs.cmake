# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/chrst/esp/v5.2.2/esp-idf/components/bootloader/subproject"
  "F:/Code/Esp32s3/ws2812b_lib/build/bootloader"
  "F:/Code/Esp32s3/ws2812b_lib/build/bootloader-prefix"
  "F:/Code/Esp32s3/ws2812b_lib/build/bootloader-prefix/tmp"
  "F:/Code/Esp32s3/ws2812b_lib/build/bootloader-prefix/src/bootloader-stamp"
  "F:/Code/Esp32s3/ws2812b_lib/build/bootloader-prefix/src"
  "F:/Code/Esp32s3/ws2812b_lib/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/Code/Esp32s3/ws2812b_lib/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/Code/Esp32s3/ws2812b_lib/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
