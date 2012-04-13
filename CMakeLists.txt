project(LIBBRAUNR)
cmake_minimum_required(VERSION 2.6)

include(TestBigEndian)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Debug)
set(BUILD_SHARED_LIBS TRUE)
test_big_endian(HOST_BIG_ENDIAN)

add_definitions(-Wall -Wextra -Wmissing-prototypes)
add_definitions(-D_HOST_BIG_ENDIAN=${HOST_BIG_ENDIAN})

set(CONFIG_NR_CPUS 8)
set(CONFIG_CPU_L1_SHIFT 6)
set(CONFIG_MEM_MALLOC 0)
set(CONFIG_MEM_USE_PHYS 0)
set(CONFIG_MEM_VERIFY 0)

set(BRAUNR_SOURCES
  error.c
  list.c
  avltree.c
  rbtree.c
  rdxtree.c
  mem.c
  phys.c
  xprintf.c
)

set(LIBS -lpthread -lrt)

add_definitions(-DCONFIG_NR_CPUS=${CONFIG_NR_CPUS})
add_definitions(-DCONFIG_CPU_L1_SHIFT=${CONFIG_CPU_L1_SHIFT})

if(CONFIG_MEM_MALLOC)
  add_definitions(-DCONFIG_MEM_MALLOC)
  set(BRAUNR_SOURCES ${BRAUNR_SOURCES} mem_malloc.c)
endif(CONFIG_MEM_MALLOC)

if(CONFIG_MEM_USE_PHYS)
  add_definitions(-DCONFIG_MEM_USE_PHYS)
endif(CONFIG_MEM_USE_PHYS)

if(CONFIG_MEM_VERIFY)
  add_definitions(-DCONFIG_MEM_VERIFY)
endif(CONFIG_MEM_VERIFY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(rbraun ${BRAUNR_SOURCES})
target_link_libraries(rbraun ${LIBS})
set_target_properties(rbraun PROPERTIES VERSION 1)

add_executable(test_avltree test/test_avltree.c)
target_link_libraries(test_avltree rbraun ${LIBS})

add_executable(test_rbtree test/test_rbtree.c)
target_link_libraries(test_rbtree rbraun ${LIBS})

add_executable(test_rdxtree test/test_rdxtree.c)
target_link_libraries(test_rdxtree rbraun ${LIBS})

add_executable(test_mem_cache test/test_mem_cache.c)
target_link_libraries(test_mem_cache ${LIBS})

add_executable(test_mem test/test_mem.c)
target_link_libraries(test_mem rbraun ${LIBS})

add_executable(test_mem_offbyone test/test_mem_offbyone.c)
target_link_libraries(test_mem_offbyone rbraun ${LIBS})

add_executable(test_mem_cache_invalid_free test/test_mem_cache_invalid_free.c)
target_link_libraries(test_mem_cache_invalid_free rbraun ${LIBS})

add_executable(test_mem_cache_double_free test/test_mem_cache_double_free.c)
target_link_libraries(test_mem_cache_double_free rbraun ${LIBS})

add_executable(test_mem_cache_write_free test/test_mem_cache_write_free.c)
target_link_libraries(test_mem_cache_write_free rbraun ${LIBS})

add_executable(test_mem_cache_write_beyond test/test_mem_cache_write_beyond.c)
target_link_libraries(test_mem_cache_write_beyond rbraun ${LIBS})

add_executable(test_mem_cache_write_buftag test/test_mem_cache_write_buftag.c)
target_link_libraries(test_mem_cache_write_buftag rbraun ${LIBS})

add_executable(test_phys test/test_phys.c)
target_link_libraries(test_phys rbraun ${LIBS})

add_executable(test_xprintf test/test_xprintf.c)
target_link_libraries(test_xprintf rbraun ${LIBS})
set_target_properties(test_xprintf PROPERTIES COMPILE_FLAGS -Wno-format)
