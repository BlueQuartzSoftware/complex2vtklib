# -------------------------------------------------------------
# This function adds the necessary cmake code to find the Vtk
# shared libraries and setup custom copy commands and/or install
# rules for Linux and Windows to use
function(AddVtkCopyInstallRules)
  set(options )
  set(oneValueArgs )
  set(multiValueArgs LIBS)
  cmake_parse_arguments(vtk "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  set(INTER_DIR ".")


  if(MSVC_IDE)
    set(vtk_BUILD_TYPES Debug Release)
  else()
    set(vtk_BUILD_TYPES "${CMAKE_BUILD_TYPE}")
    if("${vtk_BUILD_TYPES}" STREQUAL "")
        set(vtk_BUILD_TYPES "Release")
    endif()
  endif()

  # message(STATUS "vtk_LIBS: ${vtk_LIBS}")

  set(vtk_INSTALL_DIR "lib")
  if(WIN32)
    set(vtk_INSTALL_DIR ".")
  endif()

  set(vtk_module_library_list "")

  foreach(vtk_module ${vtk_LIBS})
    foreach(BTYPE ${vtk_BUILD_TYPES} )
      string(TOUPPER ${BTYPE} UpperBType)
      if(MSVC_IDE)
        set(INTER_DIR "${BTYPE}")
      endif()

      set(vtk_module_target_name VTK::${vtk_module})
      #message(STATUS "${vtk_module_target_name}: Finding Dependent Libraries")
      list(APPEND vtk_module_library_list ${vtk_module_target_name})
      # Find the current library's dependent Vtk libraries
      get_target_property(vtkLibDeps ${vtk_module_target_name} IMPORTED_LINK_DEPENDENT_LIBRARIES_${UpperBType})
      if(NOT "${vtkLibDeps}" STREQUAL "vtkLibDeps-NOTFOUND" )
        list(APPEND vtk_module_library_list ${vtkLibDeps})
        #message(STATUS "  Dependent Libraries: ${vtkLibDeps}")
      else()
      # message(STATUS "---->${vtk_LIBNAME} IMPORTED_LINK_DEPENDENT_LIBRARIES_${UpperBType} NOT FOUND")
      endif()

      get_target_property(vtkLibDeps ${vtk_module_target_name} INTERFACE_LINK_LIBRARIES)
      if(NOT "${vtkLibDeps}" STREQUAL "vtkLibDeps-NOTFOUND" )
        list(APPEND vtk_module_library_list ${vtkLibDeps})
      else()
      # message(STATUS "---->${vtk_LIBNAME} INTERFACE_LINK_LIBRARIES NOT FOUND")
      endif()

      # Remove duplicates and set the stack_length variable (VERY IMPORTANT)
      list(REMOVE_DUPLICATES vtk_module_library_list)
      list(LENGTH vtk_module_library_list STACK_LENGTH)
      #message(STATUS "====> STACK LENGTH: ${STACK_LENGTH}")
    endforeach(BTYPE ${vtk_BUILD_TYPES} )
  endforeach(vtk_module ${vtk_LIBS})

  #message(STATUS "VTK: Generating Install Rules")
  foreach(vtk_module_target_name ${vtk_module_library_list})
    foreach(BTYPE ${vtk_BUILD_TYPES} )
      string(TOUPPER ${BTYPE} UpperBType)
      if(MSVC_IDE)
        set(INTER_DIR "${BTYPE}")
      endif()

      # message(STATUS "  Creating Install Rule for ${vtk_module_target_name}")
      # Get the Actual Library Path and create Install and copy rules
      get_target_property(DllLibPath ${vtk_module_target_name} IMPORTED_LOCATION_${UpperBType})
      # message(STATUS "  DllLibPath: ${DllLibPath}")
      if(NOT "${DllLibPath}" STREQUAL "DllLibPath-NOTFOUND")

        string(REPLACE "::" "_" module_short_name "${vtk_module_target_name}")
        if(NOT TARGET ZZ_${module_short_name}_DLL_${UpperBType}-Copy)
          #message(STATUS "Creating Copy and Install Rule for ${DllLibPath}")
          add_custom_target(ZZ_${module_short_name}_DLL_${UpperBType}-Copy ALL
                              COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DllLibPath}
                              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/
                              # COMMENT "  Copy: ${DllLibPath} To: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/"
                              )
          set_target_properties(ZZ_${module_short_name}_DLL_${UpperBType}-Copy PROPERTIES FOLDER ZZ_COPY_FILES/${BTYPE}/Vtk)
          install(FILES ${DllLibPath} 
                  DESTINATION "${vtk_INSTALL_DIR}" 
                  CONFIGURATIONS ${BTYPE} 
                  COMPONENT Applications)
          get_property(COPY_LIBRARY_TARGETS GLOBAL PROPERTY COPY_LIBRARY_TARGETS)
          set_property(GLOBAL PROPERTY COPY_LIBRARY_TARGETS ${COPY_LIBRARY_TARGETS} ZZ_${module_short_name}_DLL_${UpperBType}-Copy)
        endif()
      endif()
    endforeach(BTYPE ${vtk_BUILD_TYPES} )
  endforeach()

endfunction()