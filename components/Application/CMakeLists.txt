idf_component_register(SRCS "Application.cpp"  
                    INCLUDE_DIRS "include"
                    REQUIRES Network IGpio ISensor Input Output Sensor debug_def esp_wifi esp_hw_support efuse)
