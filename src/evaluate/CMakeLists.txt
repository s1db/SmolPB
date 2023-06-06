include(GenerateExportHeader)


add_library(evaluate constraint.cpp)



add_library(SmolPB::evaluate ALIAS evaluate)

target_link_libraries(evaluate PRIVATE SmolPB_options SmolPB_warnings)

target_include_directories(evaluate ${WARNING_GUARD} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                                                  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_compile_features(evaluate PUBLIC cxx_std_20)

set_target_properties(
  evaluate
  PROPERTIES VERSION ${PROJECT_VERSION}
             CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES)

generate_export_header(evaluate EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/SmolPB/evaluate_export.hpp)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(evaluate PUBLIC EVALUATE_STATIC_DEFINE)
endif()
