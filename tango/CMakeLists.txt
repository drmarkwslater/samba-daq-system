# Buildscript for samba
add_compile_definitions(ForTango)

add_executable(tango tango.c
  version.c
  calib.c
  evtread.c
  vsninfo.h
  EDW/affichage.c
  EDW/production.c
  EDW/relecture.c
  ../samba/salsa/defauts.c
  ../samba/salsa/edwdb.c
  )

target_include_directories(tango
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/EDW
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Icon
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../samba/salsa
  )

if (WXWIDGETS)
  target_link_libraries(tango PRIVATE opium4 utils FFT fltr tcp notsvr db m db curl 
                        ${wxWidgets_LIBRARIES})
else()
  include_directories(${GLUT_INCLUDE_DIR})
  link_directories(${OPENGL_LIBRARY_DIRS})
  target_link_libraries(samba PRIVATE opium4 utils FFT fltr tcp notsvr db m db curl glfw3 
                        ${COREVIDEO_LIBRARY} ${IOKIT_LIBRARY} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})
endif()
