#定义变量包含所有的源文件
aux_source_directory(. SRC_LIST)

# 指定生成可执行文件
add_executable(chatClient ${SRC_LIST})

# 指定需要链接的库
target_link_libraries(chatClient pthread)
