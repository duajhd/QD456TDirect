# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\appQT456TDirect_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appQT456TDirect_autogen.dir\\ParseCache.txt"
  "appQT456TDirect_autogen"
  )
endif()
