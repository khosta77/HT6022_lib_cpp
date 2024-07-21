cmake_minimum_required(VERSION 3.0)
project(USBOscilloscope)

# Код для MacOS
if(APPLE)
    execute_process(
        COMMAND sw_vers -productVersion
        OUTPUT_VARIABLE OSX_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message("-- build for MacOS ${OSX_VERSION}")
    include_directories(/opt/homebrew/Cellar/libusb/1.0.26/include)
    link_directories(/opt/homebrew/Cellar/libusb/1.0.26/lib)
endif()

# Проверим есть ли у нас операционка Raspbian
execute_process(
        COMMAND cat /etc/os-release
        COMMAND grep "NAME=\"Raspbian GNU/Linux\""
        OUTPUT_VARIABLE IS_THIS_RASPBERRY_PI
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

if(IS_THIS_RASPBERRY_PI STREQUAL "NAME=\"Raspbian GNU/Linux\"")
    # Выведем уведомление о том какая у нас версия Raspberry
    execute_process(
        COMMAND cat /proc/cpuinfo
        COMMAND grep "Model"
        COMMAND cut -d: -f2-
        OUTPUT_VARIABLE RASPBERRY_PI_INFO
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message("-- build for${RASPBERRY_PI_INFO}")

    set(ENV{PKG_CONFIG_PATH} "/usr/lib/arm-linux-gnueabihf/pkgconfig")
    # Использование pkg-config для поиска пакета libusb
    find_package(PkgConfig REQUIRED)
    pkg_search_module(LIBUSB REQUIRED libusb-1.0)
endif()

# Добавление исполняемого файла с именем main.out
add_executable(main.out ./project/*.cpp)

# Включение заголовочных файлов и библиотек libusb
if(APPLE)
    target_link_libraries(main.out libusb-1.0.dylib)
endif()

if(IS_THIS_RASPBERRY_PI STREQUAL "NAME=\"Raspbian GNU/Linux\"")
    include_directories(${LIBUSB_INCLUDE_DIRS})
    target_link_libraries(main.out ${LIBUSB_LIBRARIES})
endif()
