# - Build sub-script for the Samba libraries

# - Add all the various libraries to the build
# - db  FFT  opium4  tcp  utils
add_library(db STATIC
  db/db.c
  )

target_include_directories(db
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/db
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/utils
  )

add_library(fltr STATIC
  cephes/chbevl.c
  cephes/cmplx.c
  cephes/const.c
  cephes/ellf.c
  cephes/ellie.c
  cephes/ellik.c
  cephes/ellpe.c
  cephes/ellpj.c
  cephes/ellpk.c
  cephes/mtherr.c
  cephes/polevl.c
  )

target_include_directories(fltr
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/cephes
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/utils
  )

add_library(opium4 STATIC
  opium4/aide.c
  opium4/editeur.c
  opium4/figures.c
  opium4/graphs.c
  opium4/histos.c
  opium4/infos.c
  opium4/instrum.c
  opium4/logbook.c
  opium4/menus.c
  opium4/nanopaw.c
  opium4/opium.c
  opium4/panels.c
  opium4/planches.c
  opium4/prompts.c  
  opium4/terms.c
  opium4/wnd.c
  opium4/impression.c
  opium4/opium_wx_interface.cpp
  opium4/samba_wnd.cpp
  opium4/samba_app.cpp
  opium4/samba_evt_handler.cpp
  )

target_include_directories(opium4
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/opium4
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/utils
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/tcp
  )

add_library(tcp STATIC
  tcp/html.c
  tcp/serveur.c
  tcp/sktlib.c
  )

target_include_directories(tcp
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/utils
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/tcp
  )

add_library(notsvr STATIC
  tcp/svrnul.c
  )

target_include_directories(notsvr
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/utils
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/tcp
  )

add_library(utils STATIC
  utils/calendrier.c
  utils/ceb.c
  utils/claps.c
  utils/dateheure.c
  utils/decode.c
  utils/dico.c
  utils/filtres.c
  utils/fitlib.c
  utils/rand.c
  utils/shared_memory.c
  utils/verroux.c
  utils/timeruser.c
  )

target_include_directories(utils
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/utils
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/opium4
  )

add_library(FFT STATIC
  FFT/fftw/executor.c
  FFT/fftw/fftwf77.c
  FFT/fftw/fftwnd.c
  FFT/fftw/fn_10.c
  FFT/fftw/fn_11.c
  FFT/fftw/fn_12.c
  FFT/fftw/fn_13.c
  FFT/fftw/fn_14.c
  FFT/fftw/fn_15.c
  FFT/fftw/fn_16.c
  FFT/fftw/fn_1.c
  FFT/fftw/fn_2.c
  FFT/fftw/fn_32.c
  FFT/fftw/fn_3.c
  FFT/fftw/fn_4.c
  FFT/fftw/fn_5.c
  FFT/fftw/fn_64.c
  FFT/fftw/fn_6.c
  FFT/fftw/fn_7.c
  FFT/fftw/fn_8.c
  FFT/fftw/fn_9.c
  FFT/fftw/fni_10.c
  FFT/fftw/fni_11.c
  FFT/fftw/fni_12.c
  FFT/fftw/fni_13.c
  FFT/fftw/fni_14.c
  FFT/fftw/fni_15.c
  FFT/fftw/fni_16.c
  FFT/fftw/fni_1.c
  FFT/fftw/fni_2.c
  FFT/fftw/fni_32.c
  FFT/fftw/fni_3.c
  FFT/fftw/fni_4.c
  FFT/fftw/fni_5.c
  FFT/fftw/fni_64.c
  FFT/fftw/fni_6.c
  FFT/fftw/fni_7.c
  FFT/fftw/fni_8.c
  FFT/fftw/fni_9.c
  FFT/fftw/ftw_10.c
  FFT/fftw/ftw_16.c
  FFT/fftw/ftw_2.c
  FFT/fftw/ftw_32.c
  FFT/fftw/ftw_3.c
  FFT/fftw/ftw_4.c
  FFT/fftw/ftw_5.c
  FFT/fftw/ftw_64.c
  FFT/fftw/ftw_6.c
  FFT/fftw/ftw_7.c
  FFT/fftw/ftw_8.c
  FFT/fftw/ftw_9.c
  FFT/fftw/ftwi_10.c
  FFT/fftw/ftwi_16.c
  FFT/fftw/ftwi_2.c
  FFT/fftw/ftwi_32.c
  FFT/fftw/ftwi_3.c
  FFT/fftw/ftwi_4.c
  FFT/fftw/ftwi_5.c
  FFT/fftw/ftwi_64.c
  FFT/fftw/ftwi_6.c
  FFT/fftw/ftwi_7.c
  FFT/fftw/ftwi_8.c
  FFT/fftw/ftwi_9.c
  FFT/fftw/generic.c
  FFT/fftw/malloc.c
  FFT/fftw/planner.c
  FFT/fftw/putils.c
  FFT/fftw/rader.c
  FFT/fftw/timer.c
  FFT/fftw/twiddle.c
  FFT/fftw/wisdom.c
  FFT/fftw/wisdomio.c
  FFT/rfftw/fcr_10.c
  FFT/rfftw/fcr_11.c
  FFT/rfftw/fcr_128.c
  FFT/rfftw/fcr_12.c
  FFT/rfftw/fcr_13.c
  FFT/rfftw/fcr_14.c
  FFT/rfftw/fcr_15.c
  FFT/rfftw/fcr_16.c
  FFT/rfftw/fcr_1.c
  FFT/rfftw/fcr_2.c
  FFT/rfftw/fcr_32.c
  FFT/rfftw/fcr_3.c
  FFT/rfftw/fcr_4.c
  FFT/rfftw/fcr_5.c
  FFT/rfftw/fcr_64.c
  FFT/rfftw/fcr_6.c
  FFT/rfftw/fcr_7.c
  FFT/rfftw/fcr_8.c
  FFT/rfftw/fcr_9.c
  FFT/rfftw/fhb_10.c
  FFT/rfftw/fhb_16.c
  FFT/rfftw/fhb_2.c
  FFT/rfftw/fhb_32.c
  FFT/rfftw/fhb_3.c
  FFT/rfftw/fhb_4.c
  FFT/rfftw/fhb_5.c
  FFT/rfftw/fhb_6.c
  FFT/rfftw/fhb_7.c
  FFT/rfftw/fhb_8.c
  FFT/rfftw/fhb_9.c
  FFT/rfftw/fhf_10.c
  FFT/rfftw/fhf_16.c
  FFT/rfftw/fhf_2.c
  FFT/rfftw/fhf_32.c
  FFT/rfftw/fhf_3.c
  FFT/rfftw/fhf_4.c
  FFT/rfftw/fhf_5.c
  FFT/rfftw/fhf_6.c
  FFT/rfftw/fhf_7.c
  FFT/rfftw/fhf_8.c
  FFT/rfftw/fhf_9.c
  FFT/rfftw/frc_10.c
  FFT/rfftw/frc_11.c
  FFT/rfftw/frc_128.c
  FFT/rfftw/frc_12.c
  FFT/rfftw/frc_13.c
  FFT/rfftw/frc_14.c
  FFT/rfftw/frc_15.c
  FFT/rfftw/frc_16.c
  FFT/rfftw/frc_1.c
  FFT/rfftw/frc_2.c
  FFT/rfftw/frc_32.c
  FFT/rfftw/frc_3.c
  FFT/rfftw/frc_4.c
  FFT/rfftw/frc_5.c
  FFT/rfftw/frc_64.c
  FFT/rfftw/frc_6.c
  FFT/rfftw/frc_7.c
  FFT/rfftw/frc_8.c
  FFT/rfftw/frc_9.c
  FFT/rfftw/rconfig.c
  FFT/rfftw/rexec2.c
  FFT/rfftw/rexec.c
  FFT/rfftw/rfftwf77.c
  FFT/rfftw/rfftwnd.c
  FFT/rfftw/rgeneric.c
  FFT/rfftw/rplanner.c
  )

target_include_directories(FFT
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/FFT/
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/FFT/fftw
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/FFT/rfftw
  )
