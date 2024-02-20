set(srcs 
    "main.cpp" 
    )
                
set(includes
    "." 
)
                
idf_component_register( SRCS "${srcs}"
                        INCLUDE_DIRS "${includes}")                    
