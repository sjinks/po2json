add_executable(po2json)

add_library(objlib OBJECT)
set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1)

target_compile_features(po2json PRIVATE cxx_std_20)
set_target_properties(
    po2json PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

target_compile_features(objlib PRIVATE cxx_std_20)
set_target_properties(
    objlib PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

target_sources(
    objlib PRIVATE
    conv.cpp
    gettext.cpp
    json.cpp
    translations.cpp
    processor.cpp
)

target_sources(
    po2json PRIVATE
    po2json.cpp
    $<TARGET_OBJECTS:objlib>
)

target_include_directories(po2json PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(po2json gettextpo ${EXTRA_LIBS})
