add_library_unity(
  duckdb_llm
  OBJECT
  llm_call.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_llm>
    PARENT_SCOPE)
