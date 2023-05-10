#Testing macro
macro (add_test_executable_with_lib _name libs)
	add_executable(test_${_name} "${_name}.cpp")
	target_link_libraries(test_${_name} ${OPENSSL_LIBRARIES} ${libs})
endmacro()

macro (add_test_case _name)
	add_test_executable_with_lib(${_name} "pfe")
  	add_test(NAME ${_name} COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_${_name}" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")
endmacro()

macro (add_test_case_with_run _name)
	add_test_executable_with_lib(${_name} "pfe")
	add_test(NAME ${_name} COMMAND "./run" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_${_name}" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")

endmacro()


# Test cases
add_test_case(pfe_openssl_malicious)
add_test_case(pfe_openssl_semihonest)
# add_test_case(pfe_relic_malicious)
add_test_case(pfe_relic_semihonest)
add_test_case(ecc)
add_test_case(judge)
add_test_case(rsa)
add_test_case(tim)
