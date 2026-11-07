build_exec(
  EXECNAME imt2030-density-gestion
  SOURCE_FILES imt2030-density-gestion.cc
  LIBRARIES_TO_LINK ${ns3-libs} ${ns3-contrib-libs}
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/projet-visuel-imt2030
)

build_exec(
  EXECNAME projet-densite-hotzone
  SOURCE_FILES projet-densite-hotzone.cc
  LIBRARIES_TO_LINK ${ns3-libs} ${ns3-contrib-libs}
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/projet-visuel-imt2030
)
