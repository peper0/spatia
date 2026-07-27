find_package(Doxygen REQUIRED)
find_program(SPHINX_EXECUTABLE NAMES sphinx-build)

if (NOT SPHINX_EXECUTABLE)
    message(FATAL_ERROR
            "sphinx-build was not found; install docs/requirements.txt")
endif ()

set(SPATIA_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../include")
set(SPATIA_SPHINX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html")
# Doxygen writes straight into a subdirectory of the Sphinx output, so the
# whole site is one publishable directory. The narrative pages and the API
# reference are produced by the two tools separately instead of being bridged
# by Breathe: Sphinx's C++ domain cannot resolve the declarations this library
# generates, and fails while cross-referencing them.
set(SPATIA_DOXYGEN_OUTPUT "${SPATIA_SPHINX_OUTPUT}/api")
set(SPATIA_DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

configure_file(Doxyfile.in "${SPATIA_DOXYFILE}" @ONLY)

file(GLOB_RECURSE SPATIA_PUBLIC_HEADERS CONFIGURE_DEPENDS
     "${SPATIA_INCLUDE_DIR}/spatia/*.hpp")

add_custom_target(
        spatia_docs_narrative
        COMMAND "${SPHINX_EXECUTABLE}" -b html -W --keep-going
                "${CMAKE_CURRENT_LIST_DIR}" "${SPATIA_SPHINX_OUTPUT}"
        COMMENT "Building spatia documentation with Sphinx"
        VERBATIM
)

# Runs after Sphinx, which does not know about the api/ directory and would
# report it as unexpected output.
add_custom_target(
        spatia_docs
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SPATIA_DOXYGEN_OUTPUT}"
        COMMAND "${DOXYGEN_EXECUTABLE}" "${SPATIA_DOXYFILE}"
        DEPENDS spatia_docs_narrative ${SPATIA_PUBLIC_HEADERS} "${SPATIA_DOXYFILE}"
        COMMENT "Generating spatia API reference with Doxygen"
        VERBATIM
)
