# The vwsimplification_cla executable exposes the method.
cartocrow_add_cla(vwsimplification_cla)

target_link_libraries(
    ${TARGET}
    PRIVATE
    ${COMMON_CLA_TARGET}
    vw_simplification
    core
    CGAL::CGAL
    tinyxml2::tinyxml2
)

# PHP script to run the executable.
# Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
set(PHP "run_vwsimplification.php")
set(CURRENT_CLA ${TARGET})
configure_file(
    "run_vwsimplification.php.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PHP}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PHP}" DESTINATION "${INSTALL_SCRIPT_DIR}")
