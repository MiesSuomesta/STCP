function(run_symbol_test include_dir lib_dir)
    file(GLOB HEADER_FILES "${include_dir}/*.h")

    foreach(header ${HEADER_FILES})
        file(READ "${header}" HEADER_CONTENTS)
        string(REGEX MATCHALL "stcp_[a-zA-Z0-9_]*" FOUND_FUNCS "${HEADER_CONTENTS}")

        foreach(func ${FOUND_FUNCS})
            set(SYMBOL_FOUND FALSE)

            foreach(libfile client server)
                execute_process(
                    COMMAND nm -g "${lib_dir}/libstcp${libfile}.a"
                    COMMAND grep "${func}"
                    RESULT_VARIABLE nm_result
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
                if(nm_result EQUAL 0)
                    set(SYMBOL_FOUND TRUE)
                endif()
            endforeach()

            if(NOT SYMBOL_FOUND)
                message(FATAL_ERROR "❌ Function ${func} NOK, it does not found in any library!")
            else()
                message(STATUS "✅ Function ${func} OK")
            endif()
        endforeach()
    endforeach()
endfunction()
