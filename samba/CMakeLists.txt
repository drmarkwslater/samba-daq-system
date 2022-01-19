# Buildscript for samba
add_compile_definitions(ForSamba)

add_executable(samba archive.c
  banc.c
  cablage.c
  detecteurs.c
  export.c
  lect.c
  monit.c
  organigramme.c
  repartiteurs.c
  scripts.c
  settings.c
  version.c
  autom.c
  basic.c
  calculs.c
  diags.c
  gener.c
  media.c
  numeriseurs.c
  perso.c
  samba.c
  Samba.cpp
  SambaConfig.cpp
  plplot_test.cpp
  sequences.c
  traitmt.c
  PCI/IcaLib.c
  PCI/PciLib.c
  salsa/defauts.c
  salsa/edwdb.c
  )

target_include_directories(samba
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/PCI
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Icones
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/salsa
  )

if (WXWIDGETS)
  target_link_libraries(samba PRIVATE opium4 utils FFT fltr tcp notsvr db m db curl 
                        ${wxWidgets_LIBRARIES} Boost::program_options Boost::log
                        PLPLOT::plplot PLPLOT::wxwidgets PLPLOT::wxwidgets)
  #include_directories(${PLplot_INCLUDE_DIRS})
  #link_directories(${PLplot_LIBRARY_DIRS})
else()
  include_directories(${GLUT_INCLUDE_DIR})
  link_directories(${OPENGL_LIBRARY_DIRS})
  target_link_libraries(samba PRIVATE opium4 utils FFT fltr tcp notsvr db m db curl glfw3 
                        ${COREVIDEO_LIBRARY} ${IOKIT_LIBRARY} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})
endif()
