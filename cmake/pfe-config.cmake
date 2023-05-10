find_package(OpenSSL REQUIRED)

find_path(PFE_INCLUDE_DIR NAMES pfe/pfe.h)
find_library(PFE_LIBRARY NAMES pfe)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pfe DEFAULT_MSG PFE_INCLUDE_DIR PFE_LIBRARY)


if(PFE_FOUND)
	set(PFE_LIBRARIES ${PFE_LIBRARY} ${OPENSSL_LIBRARIES})# ${Boost_LIBRARIES})
	set(PFE_INCLUDE_DIRS ${PFE_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})# ${Boost_INCLUDE_DIRS})
endif()
