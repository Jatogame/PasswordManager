cmake_minimum_required(VERSION 3.19)
project(PasswordManager LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)
include_directories(${CMAKE_SOURCE_DIR}/include)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

qt_standard_project_setup()

qt_add_executable(PasswordManager
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    include/mainwindow.h
    ui/mainwindow.ui
    src/dbmanager.cpp
    include/dbmanager.h




    src/readfile.cpp
    src/createfile.cpp
    include/dbHeader.h
    src/savefile.cpp
    src/encryptfile.cpp
    src/derivePassword.cpp
    src/datasets.cpp
    src/serializeDB.cpp
    src/closeLock.cpp
    src/decryptfile.cpp





)

target_include_directories(${PROJECT_NAME} PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SODIUM_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE sqlite3)

target_link_libraries(PasswordManager
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

include(GNUInstallDirs)

install(TARGETS PasswordManager
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET PasswordManager
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
