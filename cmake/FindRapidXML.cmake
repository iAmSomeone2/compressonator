#[=======================================================================[.rst:
FindRapid.xml
-------

Finds the RapidXML header library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``RapidXML::RapidXML``
  The RapidXML library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``RapidXML_FOUND``
  True if the system has the RapidXML library.
``RapidXML_INCLUDE_DIRS``
  Include directories needed to use RapidXML.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``RapidXML_INCLUDE_DIR``
  The directory containing ``RapidXML.h``.

#]=======================================================================]

find_path(RapidXML_INCLUDE_DIR
    NAMES
        rapidxml.hpp
        rapidxml_iterators.hpp
        rapidxml_print.hpp
        rapidxml_utils.hpp
    PATHS /usr/include/boost/property_tree/detail/
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RapidXML
        FOUND_VAR RapidXML_FOUND
        REQUIRED_VARS RapidXML_INCLUDE_DIR
)

if(RapidXML_FOUND)
    set(RapidXML_INCLUDE_DIRS ${RapidXML_INCLUDE_DIR})
endif()

if(RapidXML_FOUND AND NOT TARGET RapidXML::RapidXML)
    add_library(RapidXML::RapidXML UNKNOWN IMPORTED)
    set_target_properties(RapidXML::RapidXML PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${RapidXML_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(RapidXML_INCLUDE_DIR)
