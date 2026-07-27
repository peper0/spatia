find_package(Doxygen REQUIRED)
find_program(SPHINX_EXECUTABLE NAMES sphinx-build)

if (NOT SPHINX_EXECUTABLE)
    message(FATAL_ERROR
            "sphinx-build was not found; install docs/requirements.txt")
endif ()

set(SPATIA_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../include")
set(SPATIA_DOXYGEN_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
set(SPATIA_DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
set(SPATIA_SPHINX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html")

configure_file(Doxyfile.in "${SPATIA_DOXYFILE}" @ONLY)

file(GLOB_RECURSE SPATIA_PUBLIC_HEADERS CONFIGURE_DEPENDS
     "${SPATIA_INCLUDE_DIR}/spatia/*.hpp")

add_custom_command(
        OUTPUT "${SPATIA_DOXYGEN_OUTPUT}/xml/index.xml"
        COMMAND "${DOXYGEN_EXECUTABLE}" "${SPATIA_DOXYFILE}"
        DEPENDS ${SPATIA_PUBLIC_HEADERS} "${SPATIA_DOXYFILE}"
        COMMENT "Generating spatia API XML with Doxygen"
        VERBATIM
)

add_custom_target(
        spatia_docs
        COMMAND "${CMAKE_COMMAND}" -E env
                "SPATIA_DOXYGEN_XML=${SPATIA_DOXYGEN_OUTPUT}/xml"
                "${SPHINX_EXECUTABLE}" -b html -W --keep-going
                "${CMAKE_CURRENT_LIST_DIR}" "${SPATIA_SPHINX_OUTPUT}"
        DEPENDS "${SPATIA_DOXYGEN_OUTPUT}/xml/index.xml"
        COMMENT "Building spatia documentation with Sphinx"
        VERBATIM
)
