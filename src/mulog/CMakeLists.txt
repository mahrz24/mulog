file(GLOB mulog_directory_files "*")

foreach(f ${mulog_directory_files})
  if(NOT f MATCHES "\\.txt" AND NOT f MATCHES "CMakeFiles"
      AND NOT f MATCHES "Makefile"
      AND NOT f MATCHES "cmake_install.cmake")
    list(APPEND mulog_directory_files_to_install ${f})
  endif()
endforeach(f ${mulog_directory_files})

install(FILES
  ${mulog_directory_files_to_install}
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
  )