cmake_minimum_required(VERSION 3.26)
project(database_manager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
set(CMAKE_CXX_FLAGS)
else()
set(CMAKE_CXX_FLAGS -pthread)
endif()

#NOTE I installed all libraries with vcpkg, however mysql connector didnt work and qt took multiple days to install and failed, so these are downloaded and linked manually. Mingw also had many issues so preferably use msvc.

#set(CMAKE_PREFIX_PATH "D:/Libraries/Qt/6.7.3/mingw_64")
#list(APPEND CMAKE_PREFIX_PATH "D:/Libraries/Qt/6.7.3/mingw_64")
#set(QT_PLUGIN_PATH "D:/Libraries/Qt/6.7.3/mingw_64/plugins")
list(APPEND CMAKE_PREFIX_PATH "D:/Libraries/Qt/6.7.3/msvc2022_64")
set(QT_PLUGIN_PATH "D:/Libraries/Qt/6.7.3/msvc2022_64/plugins")

#set(Qt6_DIR "D:/Libraries/Qt/6.7.3/mingw_64/lib/cmake/Qt6")
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Pdf Core Gui PdfWidgets SvgWidgets Xml)

#set(mysql-concpp_DIR "C:/Program Files/MySQL/MySQL Connector C++ 9.2")
set(mysql-concpp_DIR "D:/Libraries/mysql")
find_package(mysql-concpp REQUIRED)
#set(mysql_lib "D:/Libraries/mysql/lib64/vs14/mysqlcppconnx.lib")
#set(mysql_lib_d "D:/Libraries/mysql/lib64/debug/vs14/mysqlcppconnx.lib")
#set(mysql_include "D:/Libraries/mysql/include")

#-DCMAKE_TOOLCHAIN_FILE=D:/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic

find_package(absl CONFIG REQUIRED)
find_package(utf8_range CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
#set(pricing_package_DIR "C:/Users/alist/CLionProjects/pricing-package")
#find_package(pricing_package)
#set(${PROJECT_NAME}_core_DIR ${CMAKE_SOURCE_DIR})
#find_package(${PROJECT_NAME}_core)
#set(encrypt_DIR "C:/Users/alist/CLionProjects/encrypt")
#find_package(encrypt REQUIRED)

add_library(encrypt STATIC IMPORTED)

set_target_properties(encrypt PROPERTIES
        IMPORTED_LOCATION_DEBUG   "C:/Users/alist/CLionProjects/encrypt/cmake-build-debug-visual-studio/encrypt.lib"
        IMPORTED_LOCATION_RELEASE "C:/Users/alist/CLionProjects/encrypt/cmake-build-release-visual-studio/encrypt.lib"
)

add_library(pricing_package STATIC IMPORTED)

set_target_properties(pricing_package PROPERTIES
        IMPORTED_LOCATION_DEBUG   "C:/Users/alist/CLionProjects/pricing-package/cmake-build-debug-visual-studio/lib/pricing_package_static_d.lib"
        IMPORTED_LOCATION_RELEASE "C:/Users/alist/CLionProjects/pricing-package/cmake-build-release-visual-studio/lib/pricing_package_static.lib"
)

#add_library(database_manager_core::database_manager_core STATIC IMPORTED)
#
#set_target_properties(database_manager_core::database_manager_core PROPERTIES
#        IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_BINARY_DIR}/database_manager_core_d.lib"
#        IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/database_manager_core.lib"
#)

#if (WIN32)

#include_directories(
#    ${CURL_INCLUDE_DIRS}
#    ${JSON_INCLUDE_DIRS}
#    ${MYSQL_INCLUDE_DIRS}
#    ${BOOST_INCLUDE_DIRS}
#)

#endif()

include_directories(${PROJECT_SOURCE_DIR}/ui)

add_definitions(-DQT_NO_VERSION_TAGGING -DWIN32_LEAN_AND_MEAN -DCURL_STATICLIB -DNOMINMAX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_UI ui/MainMenu.ui ui/MainMenu.cpp ui/MainMenu.h)
set(WIDGETS ui/widgets/DynamicComboBox.cpp ui/widgets/DynamicComboBox.h ui/widgets/ActivatorLabel.cpp ui/widgets/ActivatorLabel.h ui/widgets/AddDrawingPageWidget.ui ui/widgets/AddDrawingPageWidget.cpp ui/widgets/AddDrawingPageWidget.h ui/widgets/DrawingViewWidget.ui ui/widgets/DrawingViewWidget.cpp ui/widgets/DrawingViewWidget.h ui/widgets/DrawingView.cpp ui/widgets/DrawingView.h ui/widgets/DimensionLine.cpp ui/widgets/DimensionLine.h ui/widgets/AddLapWidget.cpp ui/widgets/AddLapWidget.h ui/widgets/ExpandingWidget.h ui/widgets/ExpandingWidget.cpp ui/widgets/Inspector.h ui/widgets/Inspector.cpp       ui/widgets/addons/AreaGraphicsItem.h ui/widgets/addons/AreaGraphicsItem.cpp ui/widgets/addons/GroupGraphicsItem.h ui/widgets/addons/GroupGraphicsItem.cpp ui/widgets/DrawingSearchResultsModel.cpp ui/widgets/DrawingSearchResultsModel.h include/database/DrawingPDFWriter.h src/database/DrawingPDFWriter.cpp ui/widgets/PdfView.h ui/widgets/PdfView.cpp)
set(COMPONENT_WINDOWS ui/AddApertureWindow.ui ui/AddApertureWindow.cpp ui/AddApertureWindow.h ui/AddSideIronWindow.ui ui/AddSideIronWindow.cpp ui/AddSideIronWindow.h ui/AddMaterialWindow.ui ui/AddMaterialWindow.cpp ui/AddMaterialWindow.h ui/AddMachineWindow.ui ui/AddMachineWindow.cpp ui/AddMachineWindow.h ui/MaterialPricingWindow.ui ui/MaterialPricingWindow.h ui/MaterialPricingWindow.cpp ui/SideIronPricingWindow.ui ui/SideIronPricingWindow.h ui/SideIronPricingWindow.cpp ui/AddMaterialPriceWindow.ui ui/AddMaterialPriceWindow.h ui/AddMaterialPriceWindow.cpp ui/AddSideIronPriceWindow.ui ui/AddSideIronPriceWindow.h ui/AddSideIronPriceWindow.cpp ui/ExtraPricingWindow.ui ui/ExtraPricingWindow.h ui/ExtraPricingWindow.cpp ui/AddExtraPriceWindow.ui ui/AddExtraPriceWindow.h ui/AddExtraPriceWindow.cpp ui/LabourTimesWindow.h ui/LabourTimesWindow.cpp ui/LabourTimesWindow.ui ui/AddLabourTimesWindow.h ui/AddLabourTimesWindow.cpp ui/AddLabourTimesWindow.ui ui/SpecificSideIronPricingWindow.h ui/SpecificSideIronPricingWindow.cpp ui/SpecificSideIronPricingWindow.ui ui/AddSpecificSideIronPriceWindow.h ui/AddSpecificSideIronPriceWindow.cpp ui/AddSpecificSideIronPriceWindow.ui ui/PowderCoatingPricingWindow.h ui/PowderCoatingPricingWindow.cpp ui/PowderCoatingPricingWindow.ui ui/AddSpecificSideIronPriceWindow.h ui/AddSpecificSideIronPriceWindow.cpp)
set(BASE src/networking/Server.cpp src/networking/Client.cpp guard.h src/networking/NetworkMessage.cpp src/networking/TCPSocket.cpp src/database/DatabaseManager.cpp src/database/Drawing.cpp src/database/DatabaseRequestHandler.cpp src/database/DatabaseQuery.cpp src/database/drawingComponents.cpp src/database/DatabaseResponseHandler.cpp include/database/ComboboxDataSource.h src/database/ComboboxDataSource.cpp src/database/componentFilters.cpp src/database/Logger.cpp)
set(BASE_H include/networking/Server.h include/networking/Client.h include/networking/NetworkMessage.h include/networking/TCPSocket.h include/database/DatabaseManager.h include/database/Drawing.h include/database/DatabaseRequestHandler.h include/database/DatabaseQuery.h include/database/drawingComponents.h include/database/RequestType.h include/database/DatabaseResponseHandler.h include/database/DataSource.h packer.h include/database/componentFilters.h include/util/format.h include/util/DataSerialiser.h include/database/Logger.h include/database/ExtraPriceManager.h)
set(QT_RESOURCES res/qtresources.qrc res/resources.rc)


add_library(${PROJECT_NAME}_core STATIC ${BASE} ${BASE_H} include/database/ExtraPriceManager.h)

include_directories(${mysql_include})

#target_compile_definitions(${PROJECT_NAME}_core PRIVATE BUILDING_LIB)
target_include_directories(${PROJECT_NAME}_core PUBLIC include/database include/networking)
target_link_libraries(${PROJECT_NAME}_core PRIVATE CURL::libcurl absl::absl_check utf8_range::utf8_range mysql::concpp encrypt)

install(TARGETS ${PROJECT_NAME}_core
        EXPORT ${PROJECT_NAME}_core-config
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
        DIRECTORY include/database
        DESTINATION include/database
)

install(
        DIRECTORY include/networking
        DESTINATION include/networking
)

export(TARGETS ${PROJECT_NAME}_core
        NAMESPACE ${PROJECT_NAME}_core::
        FILE "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}_core-config.cmake"
)


include_directories(${PROJECT_NAME} ${JSON_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${MYSQL_CONCPP_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${EXTRA_ADD_EXECUTABLE_ARGS} ${COMPONENT_WINDOWS} ${WIDGETS} ${PROJECT_UI} ${QT_RESOURCES} main.cpp include/database/ExtraPriceManager.h)

#if (MSVC)

#add_compile_options(
    #$<$<CONFIG:>:/MT>
    #$<$<CONFIG:Debug>:/MTd>
    #$<$<CONFIG:Release>:/MT>
#    "/std:c++17"
#    "/wd4068"
#)

#target_link_libraries(${PROJECT_NAME} Dnsapi ${MYSQL_LIBRARY} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Pdf Qt5::PdfWidgets Qt5::Svg ${ZLIB_LIBRARIES} ${CURL_LIBRARY})
target_link_libraries(${PROJECT_NAME} Dnsapi ZLIB::ZLIB CURL::libcurl mysql::concpp Ws2_32 Qt6::Widgets Qt6::Svg Qt6::Core Qt6::Pdf Qt6::Gui Qt6::PdfWidgets Qt6::SvgWidgets Qt6::Xml pricing_package encrypt database_manager_core)# libstdc++)
#target_link_libraries(${PROJECT_NAME}_core ${CURL_LIBRARY} Qt5::Core Qt5::Widgets Qt5::Pdf Qt5::Svg)


#else()


#endif ()

#option(ASAN_ENABLED "Build this target with AddressSanitizer" ON)

#if(ASAN_ENABLED)
#    if(MSVC)
#target_link_options(${PROJECT_NAME} PUBLIC /NODEFAULTLIB:libcmt.lib)
#    else()
#
#    endif()
#endif()


if(ASAN_ENABLED)
    if (WIN32)
#        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /fsanitize=address")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /fsanitize=address /fsanitize-ignorelist=${CMAKE_SOURCE_DIR}/ingorelist.txt")
#        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /fsanitize=address /STACK:10000000 /HEAP:1000000000")
    else()
        #    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -g -O0")
        #    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -g -O0")
        include_directories("/usr/include/opencv4")
    endif()
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Set optimization flags to none ("-O0")
#    target_compile_options(${PROJECT_NAME} PRIVATE -O0)
endif()

target_include_directories(${PROJECT_NAME}_core PUBLIC C:/Users/alist/CLionProjects/encrypt/include)
target_include_directories(${PROJECT_NAME} PUBLIC C:/Users/alist/CLionProjects/encrypt/include)
include_directories(${ENRYPT_INCLUDE})

target_compile_definitions(${PROJECT_NAME}_core PRIVATE $<$<CONFIG:Debug>:DEBUG>)
set_target_properties(${PROJECT_NAME}_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
    ARCHIVE_OUTPUT_NAME_DEBUG ${PROJECT_NAME}_core_d
    RUNTIME_OUTPUT_NAME_DEBUG ${PROJECT_NAME}_core_d
)

# Configure Release build
set_target_properties(${PROJECT_NAME}_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
    ARCHIVE_OUTPUT_NAME_RELEASE ${PROJECT_NAME}_core
    RUNTIME_OUTPUT_NAME_RELEASE ${PROJECT_NAME}_core
)


include(CPack)
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNDEBUG)  # Disable debug-related behavior in release builds
endif()

set(INSTALL_DIR "C:/Users/alist/database_test")
set(CMAKE_INSTALL_PREFIX "C:/Users/alist/database_test")
set(DEBUG_BIN_DIR "${INSTALL_DIR}/bin/Debug")
set(RELEASE_BIN_DIR "${INSTALL_DIR}/bin/Release")
string(REPLACE "/" "\\" RELEASE_BIN_DIR_W ${RELEASE_BIN_DIR})

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION ${DEBUG_BIN_DIR}
)

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION ${RELEASE_BIN_DIR}
)

install(FILES
        "${MYSQL_CONCPP_RUNTIME_LIBRARY_DIR_DEBUG}/mysqlcppconnx-2-vs14.dll"
        "cmake-build-debug-visual-studio/libcurl-d.dll"
        "cmake-build-debug-visual-studio/Qt6Cored.dll"
        "cmake-build-debug-visual-studio/Qt6Guid.dll"
        "cmake-build-debug-visual-studio/Qt6Networkd.dll"
        "cmake-build-debug-visual-studio/Qt6Pdfd.dll"
        "cmake-build-debug-visual-studio/Qt6PdfWidgetsd.dll"
        "cmake-build-debug-visual-studio/Qt6Svgd.dll"
        "cmake-build-debug-visual-studio/Qt6Widgetsd.dll"
        "cmake-build-debug-visual-studio/zlibd1.dll"
        DESTINATION "${DEBUG_BIN_DIR}"
)

install(FILES
        "cmake-build-debug-visual-studio/platforms/qwindowsd.dll"
        DESTINATION "${DEBUG_BIN_DIR}/platforms"
)

install(FILES
        "${MYSQL_CONCPP_RUNTIME_LIBRARY_DIR}/mysqlcppconnx-2-vs14.dll"
        "cmake-build-release-visual-studio/libcurl.dll"
        "cmake-build-release-visual-studio/Qt6Core.dll"
        "cmake-build-release-visual-studio/Qt6Gui.dll"
        "cmake-build-release-visual-studio/Qt6Network.dll"
        "cmake-build-release-visual-studio/Qt6Pdf.dll"
        "cmake-build-release-visual-studio/Qt6PdfWidgets.dll"
        "cmake-build-release-visual-studio/Qt6Svg.dll"
        "cmake-build-release-visual-studio/Qt6Widgets.dll"
        "cmake-build-release-visual-studio/zlib1.dll"
        DESTINATION "${RELEASE_BIN_DIR}"
)

install(FILES
        "cmake-build-release-visual-studio/platforms/qwindows.dll"
        DESTINATION "${RELEASE_BIN_DIR}/platforms"
)

configure_file(${CMAKE_SOURCE_DIR}/in/clientMeta-local.json.in ${CMAKE_BINARY_DIR}/in/local/clientMeta.json @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/in/local/clientMeta.json DESTINATION "${INSTALL_DIR}/Client/Local/client")

configure_file(${CMAKE_SOURCE_DIR}/in/clientMeta-server.json.in ${CMAKE_BINARY_DIR}/in/server/clientMeta.json @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/in/server/clientMeta.json DESTINATION "${INSTALL_DIR}/Client/Server/client")

configure_file(${CMAKE_SOURCE_DIR}/in/serverMeta.json.in ${CMAKE_BINARY_DIR}/in/serverMeta.json @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/in/serverMeta.json DESTINATION "${INSTALL_DIR}/Server/server")

configure_file(${CMAKE_SOURCE_DIR}/in/installer.nsis.in ${CMAKE_BINARY_DIR}/in/installer.nsis @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/in/installer.nsis DESTINATION "${INSTALL_DIR}")

install(DIRECTORY DESTINATION "${INSTALL_DIR}/Server/Dev")
install(DIRECTORY DESTINATION "${INSTALL_DIR}/Server/Live")

install(CODE "
    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Client/Local/SCS Screen Mat Database - Debug.lnk');
        \$s.TargetPath='${DEBUG_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${DEBUG_BIN_DIR}';
        \$s.Arguments='--client --meta \"${INSTALL_DIR}/Client/Local/client\" --console -style Fusion -platformpluginpath platforms --console';
        \$s.Save()\"
    )

    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Client/Local/SCS Screen Mat Database - Release.lnk');
        \$s.TargetPath='${RELEASE_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${RELEASE_BIN_DIR}';
        \$s.Arguments='--client --meta \"${INSTALL_DIR}/Client/Local/client\" --console -style Fusion -platformpluginpath platforms --console';
        \$s.Save()\"
    )
    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Client/Server/SCS Screen Mat Database - Debug.lnk');
        \$s.TargetPath='${DEBUG_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${DEBUG_BIN_DIR}';
        \$s.Arguments='--client --meta \"${INSTALL_DIR}/Client/Server/client\" --console -style Fusion -platformpluginpath platforms --console';
        \$s.Save()\"
    )

    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Client/Server/SCS Screen Mat Database - Release.lnk');
        \$s.TargetPath='${RELEASE_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${RELEASE_BIN_DIR}';
        \$s.Arguments='--client --meta \"${INSTALL_DIR}/Client/Server/client\" --console -style Fusion -platformpluginpath platforms --console';
        \$s.Save()\"
    )

    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Server/Dev/SCS Screen Mat Database - Debug.lnk');
        \$s.TargetPath='${DEBUG_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${DEBUG_BIN_DIR}';
        \$s.Arguments='--server --dev --meta \"${INSTALL_DIR}/Server/server/\"';
        \$s.Save()\"
    )

    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Server/Dev/SCS Screen Mat Database - Release.lnk');
        \$s.TargetPath='${RELEASE_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${RELEASE_BIN_DIR}';
        \$s.Arguments='--server --dev --meta \"${INSTALL_DIR}/Server/server/\"';
        \$s.Save()\"
    )

    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Server/Dev/Setup Server.lnk');
        \$s.TargetPath='${RELEASE_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${RELEASE_BIN_DIR}';
        \$s.Arguments='--setup --meta \"${INSTALL_DIR}/Server/server\"';
        \$s.Save()\"
    )

        execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Server/Live/SCS Screen Mat Database - Debug.lnk');
        \$s.TargetPath='${DEBUG_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${DEBUG_BIN_DIR}';
        \$s.Arguments='--server --meta \"${INSTALL_DIR}/Server/server/\"';
        \$s.Save()\"
    )

    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Server/Live/SCS Screen Mat Database - Release.lnk');
        \$s.TargetPath='${RELEASE_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${RELEASE_BIN_DIR}';
        \$s.Arguments='--server --meta \"${INSTALL_DIR}/Server/server/\"';
        \$s.Save()\"
    )

    execute_process(COMMAND powershell -Command
        \"\$s=(New-Object -COM WScript.Shell).CreateShortcut('${INSTALL_DIR}/Server/Live/Setup Server.lnk');
        \$s.TargetPath='${RELEASE_BIN_DIR}/${PROJECT_NAME}.exe';
        \$s.WorkingDirectory='${RELEASE_BIN_DIR}';
        \$s.Arguments='--setup --meta \"${INSTALL_DIR}/Server/server\"';
        \$s.Save()\"
    )
")