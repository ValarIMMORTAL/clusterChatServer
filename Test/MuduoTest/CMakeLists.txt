set(SRC_LIST ./MuduoServer.cpp)

# 设置可执行文件生成目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(MuduoTest ${SRC_LIST})

target_link_libraries(MuduoTest
    muduo_base
    muduo_net
    pthread
)