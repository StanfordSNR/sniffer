macro (add_exec name)
	add_executable ("${name}" "${name}.cc")
	target_link_libraries ("${name}" util)
endmacro (add_exec)

add_exec(fun)
add_exec(blastudp)
add_exec(receiveudp)
