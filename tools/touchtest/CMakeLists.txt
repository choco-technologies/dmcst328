# touchtest - a manual test of an input node (dmcst328 touch panel); added from the top-level CMakeLists.txt (shares its dmod setup)
set(DMOD_DMR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/touchtest.dmr)

dmod_add_executable(touchtest ${DMOD_MODULE_VERSION}
    main.c
)

# dmdrvi: headers only, for the input commands of the node (DMDRVI_IOCTL_INPUT_*) and the graphics ones of the display
dmod_link_modules(touchtest
    dmdrvi
)
