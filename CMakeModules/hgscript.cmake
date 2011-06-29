set(CMAKE_MODULE_PATH ${TOP_SOURCE_DIR}/CMakeModules)
find_package(Mercurial)
if (MERCURIAL_FOUND AND IS_DIRECTORY ${TOP_SOURCE_DIR}/.hg)
  MERCURIAL_HG_INFO(${TOP_SOURCE_DIR} THIS)
  MESSAGE("Current revision is ${THIS_HG_ID}")
  set (THIS_HG_ID "hgid: ${THIS_HG_ID}")
else(MERCURIAL_FOUND AND IS_DIRECTORY ${TOP_SOURCE_DIR}/.hg)
  set (THIS_HG_ID)
endif(MERCURIAL_FOUND AND IS_DIRECTORY ${TOP_SOURCE_DIR}/.hg)
file(READ ${INPUT} CONTENT)
string(REGEX REPLACE "#CSG_HG_ID#" "${THIS_HG_ID}" NEW_CONTENT "${CONTENT}")
file(WRITE "${OUTPUT}.tmp" "${NEW_CONTENT}")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OUTPUT}.tmp ${OUTPUT})
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${OUTPUT}.tmp)