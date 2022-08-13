include(CheckCXXCompilerFlag)

option(MSVC_LINK_DYNAMIC_RUNTIME "Link against dynamic runtime" ON)
option(MSVC_PARALLELBUILD "Use flag /MP" ON)

# add compiler flag, if not already present
macro(add_compiler_flag flag supportsFlag)
    check_cxx_compiler_flag(${flag} ${supportsFlag})

    if(${supportsFlag} AND NOT "${CMAKE_CXX_FLAGS}" MATCHES "${flag}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endif()
endmacro()

# use CXX 11 and std::unique_ptr in QuantLib (instead of std::auto_ptr, which is deprecated in C++11)
set(CMAKE_CXX_STANDARD 11)

# On single-configuration builds, select a default build type that gives the same compilation flags as a default autotools build.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

if(MSVC)
    set(BUILD_SHARED_LIBS OFF)

    # build static libs always
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${MSVC_LINK_DYNAMIC_RUNTIME}>:DLL>")

    # link against static boost libraries
    if(NOT DEFINED Boost_USE_STATIC_LIBS)
        if(BUILD_SHARED_LIBS)
            set(Boost_USE_STATIC_LIBS 0)
        else()
            set(Boost_USE_STATIC_LIBS 1)
        endif()
    endif()

    # Boost static runtime ON for MSVC
    if(NOT DEFINED Boost_USE_STATIC_RUNTIME)
        if(BUILD_SHARED_LIBS OR(MSVC AND MSVC_LINK_DYNAMIC_RUNTIME))
            set(Boost_USE_STATIC_RUNTIME 0)
        else()
            set(Boost_USE_STATIC_RUNTIME 1)
        endif()
    endif()

    IF(NOT Boost_USE_STATIC_LIBS)
        add_definitions(-DBOOST_ALL_DYN_LINK)
        add_definitions(-DBOOST_TEST_DYN_LINK)
    endif()

    add_compiler_flag("-D_SCL_SECURE_NO_DEPRECATE" supports_D_SCL_SECURE_NO_DEPRECATE)
    add_compiler_flag("-D_CRT_SECURE_NO_DEPRECATE" supports_D_CRT_SECURE_NO_DEPRECATE)
    add_compiler_flag("-DBOOST_ENABLE_ASSERT_HANDLER" enableAssertionHandler)
    add_compiler_flag("/bigobj" supports_bigobj)
    add_compiler_flag("/W3" supports_w3)

    add_compiler_flag("/we4189" unused_variable_mscv)
    add_compiler_flag("/wd4834" deactivate_discrading_return_value)

    # add_compiler_flag("/we4389" signed_compare_mscv)
    add_compiler_flag("/we5038" reorder_msvc)

    if(MSVC_PARALLELBUILD)
        add_compiler_flag("/MP" parallel_build)
    endif()

else()
    # build shared libs always
    set(BUILD_SHARED_LIBS ON)

    # link against dynamic boost libraries
    add_definitions(-DBOOST_ALL_DYN_LINK)
    add_definitions(-DBOOST_TEST_DYN_LINK)

    # avoid a crash in valgrind that sometimes occurs if this flag is not defined
    add_definitions(-DBOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS)

    # switch off blas debug for build types release
    if(NOT("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
        add_definitions(-DBOOST_UBLAS_NDEBUG)
    endif()

    # enable boost assert handler
    add_compiler_flag("-DBOOST_ENABLE_ASSERT_HANDLER" enableAssertionHandler)

    # add some warnings
    add_compiler_flag("-Wall" supportsWall)
    add_compiler_flag("-Wsometimes-uninitialized" supportsSometimesUninitialized)
    add_compiler_flag("-Wmaybe-uninitialized" supportsMaybeUninitialized)

    # turn the following warnings into errors
    add_compiler_flag("-Werror=non-virtual-dtor" supportsNonVirtualDtor)
    add_compiler_flag("-Werror=sign-compare" supportsSignCompare)
    add_compiler_flag("-Werror=float-conversion" supportsWfloatConversion)
    add_compiler_flag("-Werror=reorder" supportsReorder)
    add_compiler_flag("-Werror=unused-variable" supportsUnusedVariable)
    add_compiler_flag("-Werror=unused-but-set-variable" supportsUnusedButSetVariable)
    add_compiler_flag("-Werror=uninitialized" supportsUninitialized)
    add_compiler_flag("-Werror=unused-lambda-capture" supportsUnusedLambdaCapture)
    add_compiler_flag("-Werror=return-type" supportsReturnType)
    add_compiler_flag("-Werror=unused-function" supportsUnusedFunction)
    add_compiler_flag("-Werror=suggest-override" supportsSuggestOverride)
    add_compiler_flag("-Werror=inconsistent-missing-override" supportsInconsistentMissingOverride)

    # disable some warnings
    add_compiler_flag("-Wno-unknown-pragmas" supportsNoUnknownPragmas)

    # add build/QuantLib as first include directory to make sure we include QL's cmake-configured files
    include_directories("${PROJECT_BINARY_DIR}/QuantLib")

    # similar if QuantLib is build separately
    include_directories("${CMAKE_CURRENT_LIST_DIR}/../QuantLib/build")
endif()


# set library locations
get_filename_component(QUANTLIB_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../QuantLib" ABSOLUTE)
get_filename_component(QUANTEXT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../QuantExt" ABSOLUTE)
get_filename_component(OREDATA_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../OREData" ABSOLUTE)
get_filename_component(OREANALYTICS_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../OREAnalytics" ABSOLUTE)
get_filename_component(ORETEST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../ORETest" ABSOLUTE)
get_filename_component(RAPIDXML_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../ThirdPartyLibs/rapidxml-1.13" ABSOLUTE)

# convenience function that adds a link directory dir, but only if it exists
function(add_link_directory_if_exists dir)
  if(EXISTS "${dir}")
    link_directories("${dir}")
  endif()
endfunction()

macro(get_library_name LIB_NAME OUTPUT_NAME)

    # modified version of quantlib.cmake / get_quantlib_library_name

    # message(STATUS "${LIB_NAME} Library name tokens:")
    # MSVC: Give built library different names following code in 'ql/autolink.hpp'
    if(MSVC)
        if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
            set(LIB_PLATFORM "-x64")
            set(LIB_PLATFORM "-x64")
        endif()
        # - thread linkage
        set(LIB_THREAD_OPT "-mt")  # _MT is defined for /MD and /MT runtimes (https://docs.microsoft.com/es-es/cpp/build/reference/md-mt-ld-use-run-time-library)
        #message(STATUS " - Thread opt: ${LIB_THREAD_OPT}")

        if (NOT BUILD_SHARED_LIBS AND (MSVC AND NOT MSVC_LINK_DYNAMIC_RUNTIME))
            set(LIB_RT_OPT "-s")
            set(CMAKE_DEBUG_POSTFIX "gd")
        else()
            set(CMAKE_DEBUG_POSTFIX "-gd")
        endif()
    

        set(${OUTPUT_NAME} "${LIB_NAME}${LIB_PLATFORM}${LIB_THREAD_OPT}${LIB_RT_OPT}")
    else()
        set(${OUTPUT_NAME} ${LIB_NAME})
    endif()

    message(STATUS "${LIB_NAME} library name: ${${OUTPUT_NAME}}[${CMAKE_DEBUG_POSTFIX}]")
endmacro(get_library_name)

macro(set_ql_library_name)
  if(USE_GLOBAL_ORE_BUILD)
    set(QL_LIB_NAME ql_library)
  else()
    get_library_name("QuantLib" QL_LIB_NAME)
  endif()
endmacro()
