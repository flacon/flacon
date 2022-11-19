
function(add_bool_definition VAR)
    set(_value 0)
    if (${${VAR}})
        set(_value 1)
    endif()

    add_definitions(-D${VAR}=${_value})
endfunction()