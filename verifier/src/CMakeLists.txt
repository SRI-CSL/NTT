add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-D__STDC_LIMIT_MACROS)

set(LLVM_LINK_COMPONENTS 
  irreader 
  bitwriter 
  ipo 
  scalaropts 
  instrumentation
  transformutils
  core 
  codegen 
  objcarcopts)


add_llvm_executable(nttverifier DISABLE_LLVM_LINK_LLVM_DYLIB
  ntt_verifier.cpp
  ntt_intervals.cpp)

target_link_libraries (nttverifier PRIVATE
  ${LLVM_SEAHORN_LIBS}
  ${SEA_DSA_BS}
  ${CLAM_LIBS}
)
llvm_config (nttverifier ${LLVM_LINK_COMPONENTS})
install(TARGETS nttverifier RUNTIME DESTINATION bin)

if (NTT_VERIFIER_STATIC_EXE)
  set (CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
  set_target_properties (nttverifier PROPERTIES LINK_SEARCH_START_STATIC ON)
  set_target_properties (nttverifier PROPERTIES LINK_SEARCH_END_STATIC ON)
endif()
