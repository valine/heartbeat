# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lukas/Desktop/zpath"
  "/home/lukas/Desktop/zpath/buildlocal"
  "/home/lukas/Desktop/heartbeat/cmake-build-debug-local-deps/zpath-prefix"
  "/home/lukas/Desktop/heartbeat/cmake-build-debug-local-deps/zpath-prefix/tmp"
  "/home/lukas/Desktop/heartbeat/cmake-build-debug-local-deps/zpath-prefix/src/zpath-stamp"
  "/home/lukas/Desktop/heartbeat/cmake-build-debug-local-deps/zpath-prefix/src"
  "/home/lukas/Desktop/heartbeat/cmake-build-debug-local-deps/zpath-prefix/src/zpath-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/lukas/Desktop/heartbeat/cmake-build-debug-local-deps/zpath-prefix/src/zpath-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/lukas/Desktop/heartbeat/cmake-build-debug-local-deps/zpath-prefix/src/zpath-stamp${cfgdir}") # cfgdir has leading slash
endif()
