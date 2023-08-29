# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.1/components/bootloader/subproject"
  "E:/Sotalab/SHT3x_Powersave/build/bootloader"
  "E:/Sotalab/SHT3x_Powersave/build/bootloader-prefix"
  "E:/Sotalab/SHT3x_Powersave/build/bootloader-prefix/tmp"
  "E:/Sotalab/SHT3x_Powersave/build/bootloader-prefix/src/bootloader-stamp"
  "E:/Sotalab/SHT3x_Powersave/build/bootloader-prefix/src"
  "E:/Sotalab/SHT3x_Powersave/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Sotalab/SHT3x_Powersave/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/Sotalab/SHT3x_Powersave/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
