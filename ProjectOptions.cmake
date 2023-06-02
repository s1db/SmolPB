include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(SmolPB_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(SmolPB_setup_options)
  option(SmolPB_ENABLE_HARDENING "Enable hardening" ON)
  option(SmolPB_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    SmolPB_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    SmolPB_ENABLE_HARDENING
    OFF)

  SmolPB_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR SmolPB_PACKAGING_MAINTAINER_MODE)
    option(SmolPB_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(SmolPB_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(SmolPB_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(SmolPB_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(SmolPB_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(SmolPB_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(SmolPB_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(SmolPB_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(SmolPB_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(SmolPB_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(SmolPB_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(SmolPB_ENABLE_PCH "Enable precompiled headers" OFF)
    option(SmolPB_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(SmolPB_ENABLE_IPO "Enable IPO/LTO" ON)
    option(SmolPB_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(SmolPB_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(SmolPB_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(SmolPB_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(SmolPB_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(SmolPB_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(SmolPB_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(SmolPB_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(SmolPB_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(SmolPB_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(SmolPB_ENABLE_PCH "Enable precompiled headers" OFF)
    option(SmolPB_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      SmolPB_ENABLE_IPO
      SmolPB_WARNINGS_AS_ERRORS
      SmolPB_ENABLE_USER_LINKER
      SmolPB_ENABLE_SANITIZER_ADDRESS
      SmolPB_ENABLE_SANITIZER_LEAK
      SmolPB_ENABLE_SANITIZER_UNDEFINED
      SmolPB_ENABLE_SANITIZER_THREAD
      SmolPB_ENABLE_SANITIZER_MEMORY
      SmolPB_ENABLE_UNITY_BUILD
      SmolPB_ENABLE_CLANG_TIDY
      SmolPB_ENABLE_CPPCHECK
      SmolPB_ENABLE_COVERAGE
      SmolPB_ENABLE_PCH
      SmolPB_ENABLE_CACHE)
  endif()

  SmolPB_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (SmolPB_ENABLE_SANITIZER_ADDRESS OR SmolPB_ENABLE_SANITIZER_THREAD OR SmolPB_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(SmolPB_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(SmolPB_global_options)
  if(SmolPB_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    SmolPB_enable_ipo()
  endif()

  SmolPB_supports_sanitizers()

  if(SmolPB_ENABLE_HARDENING AND SmolPB_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR SmolPB_ENABLE_SANITIZER_UNDEFINED
       OR SmolPB_ENABLE_SANITIZER_ADDRESS
       OR SmolPB_ENABLE_SANITIZER_THREAD
       OR SmolPB_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${SmolPB_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${SmolPB_ENABLE_SANITIZER_UNDEFINED}")
    SmolPB_enable_hardening(SmolPB_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(SmolPB_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(SmolPB_warnings INTERFACE)
  add_library(SmolPB_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  SmolPB_set_project_warnings(
    SmolPB_warnings
    ${SmolPB_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(SmolPB_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(SmolPB_options)
  endif()

  include(cmake/Sanitizers.cmake)
  SmolPB_enable_sanitizers(
    SmolPB_options
    ${SmolPB_ENABLE_SANITIZER_ADDRESS}
    ${SmolPB_ENABLE_SANITIZER_LEAK}
    ${SmolPB_ENABLE_SANITIZER_UNDEFINED}
    ${SmolPB_ENABLE_SANITIZER_THREAD}
    ${SmolPB_ENABLE_SANITIZER_MEMORY})

  set_target_properties(SmolPB_options PROPERTIES UNITY_BUILD ${SmolPB_ENABLE_UNITY_BUILD})

  if(SmolPB_ENABLE_PCH)
    target_precompile_headers(
      SmolPB_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(SmolPB_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    SmolPB_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(SmolPB_ENABLE_CLANG_TIDY)
    SmolPB_enable_clang_tidy(SmolPB_options ${SmolPB_WARNINGS_AS_ERRORS})
  endif()

  if(SmolPB_ENABLE_CPPCHECK)
    SmolPB_enable_cppcheck(${SmolPB_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(SmolPB_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    SmolPB_enable_coverage(SmolPB_options)
  endif()

  if(SmolPB_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(SmolPB_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(SmolPB_ENABLE_HARDENING AND NOT SmolPB_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR SmolPB_ENABLE_SANITIZER_UNDEFINED
       OR SmolPB_ENABLE_SANITIZER_ADDRESS
       OR SmolPB_ENABLE_SANITIZER_THREAD
       OR SmolPB_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    SmolPB_enable_hardening(SmolPB_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
