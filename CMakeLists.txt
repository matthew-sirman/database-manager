cmake_minimum_required(VERSION 3.26)
project(database_manager)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
set(CMAKE_CXX_FLAGS)
else()
set(CMAKE_CXX_FLAGS -pthread)
endif()

set(CMAKE_PREFIX_PATH "C:\\Libraries\\Qt\\6.7.2\\msvc2019_64")
set(QT_PLUGIN_PATH "C:\\Libraries\\Qt\\6.7.2\\msvc2019_64\\plugins")

#find_package(OpenSSL REQUIRED)

add_compile_definitions(CONCPP_BUILD_STATIC)
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Pdf Core Gui PdfWidgets SvgWidgets Xml)
set(mysql-concpp_DIR "C:/Libraries/mysql-connector-c++-9.0.0-winx64")
#set(mysql-concpp_DIR "C:/Libraries/mysql-connector-cpp")
find_package(mysql-concpp REQUIRED)
set(pricing-package_DIR "C:/Users/Alistair.Sirman/Manager/pricing-package")
find_package(pricing-package REQUIRED)

set(ZLIB_INCLUDE_DIR "C:\\Libraries\\zlib\\include")
set(ZLIB_LIBRARY "C:\\Libraries\\zlib\\zlib.lib")
set(CURL_INCLUDE_DIRS "C:\\Libraries\\curl\\include")
set(CURL_LIBRARY "C:\\Libraries\\curl\\libcurl.lib")
# set(MYSQL_INCLUDE_DIRS "C:\\Libraries\\mysql-connector-cpp\\include")
# set(MYSQL_LIBRARY "C:/Libraries/mysql-connector-cpp/lib64/debug/vs14/mysqlcppconn-static.lib")
set(JSON_INCLUDE_DIRS "C:/Libraries/json/include")
set(ENCRYPT_LIBRARY "C:/Libraries/Encrypt/encrypt.lib")
set(ENCRYPT_LIBRARY_D "C:/Libraries/Encrypt/encrypt_d.lib")
set(ENCRYPT_INCLUDE "C:/Libraries/Encrypt/include")
#set(PRICING_INCLUDE_DIR "C:\\Users\\Alistair.Sirman\\Manager\\pricing-package\\include")
#set(PRICING_LIBRARY_D "C:/Libraries/Pricing/pricing_package_d.lib")
#set(PRICING_LIBRARY "C:/Libraries/Pricing/pricing_package.lib")

#if (WIN32)

include_directories(
    ${CURL_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
)

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
set(BASE src/networking/Server.cpp include/networking/Server.h src/networking/Client.cpp include/networking/Client.h guard.h src/networking/NetworkMessage.cpp include/networking/NetworkMessage.h src/networking/TCPSocket.cpp include/networking/TCPSocket.h src/database/DatabaseManager.cpp include/database/DatabaseManager.h src/database/Drawing.cpp include/database/Drawing.h src/database/DatabaseRequestHandler.cpp include/database/DatabaseRequestHandler.h src/database/DatabaseQuery.cpp include/database/DatabaseQuery.h include/database/drawingComponents.h src/database/drawingComponents.cpp include/database/RequestType.h src/database/DatabaseResponseHandler.cpp include/database/DatabaseResponseHandler.h include/database/ComboboxDataSource.h src/database/ComboboxDataSource.cpp include/database/DataSource.h packer.h include/database/componentFilters.h src/database/componentFilters.cpp include/util/format.h include/util/DataSerialiser.h include/database/Logger.h src/database/Logger.cpp include/database/ExtraPriceManager.h)
set(QT_RESOURCES res/qtresources.qrc res/resources.rc)

add_executable(${PROJECT_NAME} ${EXTRA_ADD_EXECUTABLE_ARGS} ${COMPONENT_WINDOWS} ${WIDGETS} ${PROJECT_UI} ${QT_RESOURCES} main.cpp)

add_library(${PROJECT_NAME}_core STATIC ${BASE})# include/database/DrawingComponentManager.h)
target_compile_definitions(${PROJECT_NAME}_core PRIVATE BUILDING_LIB)
target_link_libraries(${PROJECT_NAME}_core mysql::concpp-xdevapi-static ${CURL_LIBRARY})
target_link_libraries(${PROJECT_NAME}_core optimized ${ENCRYPT_LIBRARY})
target_link_libraries(${PROJECT_NAME}_core debug ${ENCRYPT_LIBRARY_D})
target_compile_definitions(${PROJECT_NAME}_core PRIVATE CORE_DLL_EXPORT)

include_directories(${PROJECT_NAME} ${JSON_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${pricing_package_INCLUDE_DIR} ${MYSQL_CONCPP_INCLUDE_DIR})


#if (MSVC)

#add_compile_options(
    #$<$<CONFIG:>:/MT>
    #$<$<CONFIG:Debug>:/MTd>
    #$<$<CONFIG:Release>:/MT>
#    "/std:c++17"
#    "/wd4068"
#)

#target_link_libraries(${PROJECT_NAME} Dnsapi ${MYSQL_LIBRARY} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Pdf Qt5::PdfWidgets Qt5::Svg ${ZLIB_LIBRARIES} ${CURL_LIBRARY})
target_link_libraries(${PROJECT_NAME} Dnsapi ${ZLIB_LIBRARIES} ${CURL_LIBRARY} Qt6::Widgets Qt6::Svg Qt6::Core Qt6::Pdf Qt6::Gui Qt6::PdfWidgets Qt6::SvgWidgets Qt6::Xml mysql::concpp-xdevapi-static ${pricing_package_LIB} ${PROJECT_NAME}_core)
#target_link_libraries(${PROJECT_NAME}_core ${CURL_LIBRARY} Qt5::Core Qt5::Widgets Qt5::Pdf Qt5::Svg)

#else()

#target_link_libraries(${PROJECT_NAME} encrypt mysqlcppconn Qt5::Core Qt5::Gui Qt5::Widgets ${ZLIB_LIBRARIES} ${CURL_LIBRARY} ${PRICING_LIBRARY})

#endif ()

option(ASAN_ENABLED "Build this target with AddressSanitizer" ON)

if(ASAN_ENABLED)
    if(MSVC)
        target_link_options(${PROJECT_NAME} PUBLIC /NODEFAULTLIB:libcmt.lib)
    else()

    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Set optimization flags to none ("-O0")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0)
endif()

target_include_directories(${PROJECT_NAME}_core PUBLIC ../encrypt/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../encrypt/include)
include_directories(${ENRYPT_INCLUDE})
target_link_libraries(${PROJECT_NAME} debug ${ENCRYPT_LIBRARY_D})
target_link_libraries(${PROJECT_NAME} optimized ${ENCRYPT_LIBRARY})

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

add_custom_target(BuildLib ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Building ${PROJECT_NAME} lib"
    add_compile_definitions(LIB_EXPORT)
    COMMAND ${CMAKE_COMMAND} -E echo "Library is located at: $<TARGET_FILE:${PROJECT_NAME}_core>"
    DEPENDS ${PROJECT_NAME}_core
)
