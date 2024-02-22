idf_component_register(SRCS "Server.c"
                    INCLUDE_DIRS "include"
                    REQUIRES log driver esp_http_server json)
