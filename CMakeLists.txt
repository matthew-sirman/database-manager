cmake_minimum_required(VERSION 3.2)
project(database_manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

if (WIN32)
set(CMAKE_CXX_FLAGS)
else()
set(CMAKE_CXX_FLAGS -pthread)
endif()
set(Qt5Pdf_DIR "C:\\Libraries\\Qt\\5.15.0\\modules\\qtpdf\\lib\\cmake\\Qt5Pdf")
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Svg Pdf)
find_package(Qt5Pdf HINTS "C:\\Libraries\\Qt\\5.15.0\\modules\\qtpdf\\lib\\cmake\\Qt5Pdf")
find_package(Qt5PdfWidgets HINTS "C:\\Libraries\\Qt\\5.15.0\\modules\\qtpdf\\lib\\cmake\\Qt5PdfWidgets")

set(ZLIB_INCLUDE_DIR "C:\\Libraries\\zlib\\include")
set(ZLIB_LIBRARY "C:\\Libraries\\zlib\\zlib.lib")
set(CURL_INCLUDE_DIRS "C:\\Libraries\\curl\\include")
set(CURL_LIBRARY "C:\\Libraries\\curl\\libcurl.lib")

set(MY_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug or Release)")

set(MYSQL_INCLUDE_DIRS "C:\\Libraries\\mysql-connector-cpp\\include")
set(MYSQL_LIBRARY "C:/Libraries/mysql-connector-cpp/lib64/vs14/debug/mysqlcppconn8.lib")
set(JSON_INCLUDE_DIRS "C:/Libraries/json/include")
set(QT_PLUGIN_PATH "C:/Libraries/Qt/5.15.0/msvc2019_64/plugins")
cmake_path(SET PRICING_INCLUDE_DIR NORMALIZE "${PROJECT_SOURCE_DIR}/../pricing-package/include")
cmake_path(SET PRICING_LIBRARY_D NORMALIZE "${PROJECT_SOURCE_DIR}/../pricing-package/out/build/x64-Debug/lib/pricing_package_d.lib")
cmake_path(SET PRICING_LIBRARY NORMALIZE "${PROJECT_SOURCE_DIR}/../pricing-package/out/build/x64-Release/pricing_package.lib")


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
set(WIDGETS ui/widgets/DynamicComboBox.cpp ui/widgets/DynamicComboBox.h ui/widgets/ActivatorLabel.cpp ui/widgets/ActivatorLabel.h ui/widgets/AddDrawingPageWidget.ui ui/widgets/AddDrawingPageWidget.cpp ui/widgets/AddDrawingPageWidget.h ui/widgets/DrawingViewWidget.ui ui/widgets/DrawingViewWidget.cpp ui/widgets/DrawingViewWidget.h ui/widgets/DrawingView.cpp ui/widgets/DrawingView.h ui/widgets/DimensionLine.cpp ui/widgets/DimensionLine.h ui/widgets/AddLapWidget.cpp ui/widgets/AddLapWidget.h "ui/widgets/addons/ImpactPadGraphicsItem.h" "ui/widgets/addons/ImpactPadGraphicsItem.cpp"  "ui/widgets/addons/BlankSpaceGraphicsItem.h" "ui/widgets/addons/BlankSpaceGraphicsItem.cpp" "ui/widgets/addons/ExtraApertureGraphicsItem.h" "ui/widgets/addons/ExtraApertureGraphicsItem.cpp" "ui/widgets/ExpandingWidget.h" "ui/widgets/ExpandingWidget.cpp" "ui/widgets/Inspector.h" "ui/widgets/Inspector.cpp" "ui/widgets/addons/CentreHoleSetGraphicsItem.h" "ui/widgets/addons/CentreHoleSetGraphicsItem.cpp" "ui/widgets/addons/DeflectorSetGraphicsItem.h" "ui/widgets/addons/DeflectorSetGraphicsItem.cpp" "ui/widgets/addons/DivertorSetGraphicsItem.h" "ui/widgets/addons/DivertorSetGraphicsItem.cpp" "ui/widgets/addons/DamBarGraphicsItem.h" "ui/widgets/addons/DamBarGraphicsItem.cpp")
set(COMPONENT_WINDOWS ui/AddApertureWindow.ui ui/AddApertureWindow.cpp ui/AddApertureWindow.h ui/AddSideIronWindow.ui ui/AddSideIronWindow.cpp ui/AddSideIronWindow.h ui/AddMaterialWindow.ui ui/AddMaterialWindow.cpp ui/AddMaterialWindow.h ui/AddMachineWindow.ui ui/AddMachineWindow.cpp ui/AddMachineWindow.h ui/MaterialPricingWindow.ui ui/MaterialPricingWindow.h ui/MaterialPricingWindow.cpp ui/SideIronPricingWindow.ui ui/SideIronPricingWindow.h ui/SideIronPricingWindow.cpp ui/AddMaterialPriceWindow.ui ui/AddMaterialPriceWindow.h ui/AddMaterialPriceWindow.cpp ui/AddSideIronPriceWindow.ui ui/AddSideIronPriceWindow.h ui/AddSideIronPriceWindow.cpp ui/ExtraPricingWindow.ui ui/ExtraPricingWindow.h ui/ExtraPricingWindow.cpp ui/AddExtraPriceWindow.ui ui/AddExtraPriceWindow.h ui/AddExtraPriceWindow.cpp ui/LabourTimesWindow.h ui/LabourTimesWindow.cpp ui/LabourTimesWindow.ui ui/AddLabourTimesWindow.h ui/AddLabourTimesWindow.cpp ui/AddLabourTimesWindow.ui)
set(QT_RESOURCES res/qtresources.qrc)


add_subdirectory(${PROJECT_SOURCE_DIR}/../encrypt encrypt-build)

add_executable(${PROJECT_NAME} ${EXTRA_ADD_EXECUTABLE_ARGS} ${COMPONENT_WINDOWS} ${WIDGETS} ${PROJECT_UI} ${QT_RESOURCES} main.cpp src/networking/Server.cpp include/networking/Server.h src/networking/Client.cpp include/networking/Client.h guard.h src/networking/NetworkMessage.cpp include/networking/NetworkMessage.h src/networking/TCPSocket.cpp include/networking/TCPSocket.h src/database/DatabaseManager.cpp include/database/DatabaseManager.h src/database/Drawing.cpp include/database/Drawing.h src/database/DatabaseRequestHandler.cpp include/database/DatabaseRequestHandler.h src/database/DatabaseQuery.cpp include/database/DatabaseQuery.h src/database/drawingComponents.cpp include/database/drawingComponents.h include/database/RequestType.h src/database/DatabaseResponseHandler.cpp include/database/DatabaseResponseHandler.h include/database/ComboboxDataSource.h src/database/ComboboxDataSource.cpp include/database/DataSource.h src/database/DrawingSearchResultsModel.cpp include/database/DrawingSearchResultsModel.h packer.h include/database/DrawingPDFWriter.h src/database/DrawingPDFWriter.cpp   "include/database/componentFilters.h" "src/database/componentFilters.cpp" "include/util/format.h" include/util/DataSerialiser.h "ui/widgets/addons/EventTracker.h" "ui/widgets/addons/EventTracker.cpp"  )

include_directories(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/../encrypt/include ${MYSQL_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${PRICING_INCLUDE_DIR})



if (MSVC)

add_compile_options(
    #$<$<CONFIG:>:/MT>
    #$<$<CONFIG:Debug>:/MTd>
    #$<$<CONFIG:Release>:/MT>
    "/std:c++17"
    "/wd4068"
)
target_link_libraries(${PROJECT_NAME} encrypt Dnsapi ${MYSQL_LIBRARY} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Pdf Qt5::PdfWidgets Qt5::Svg ${ZLIB_LIBRARIES})
target_link_libraries(${PROJECT_NAME} debug ${PRICING_LIBRARY_D})
target_link_libraries(${PROJECT_NAME} optimized ${PRICING_LIBRARY})

else()

target_link_libraries(${PROJECT_NAME} encrypt mysqlcppconn Qt5::Core Qt5::Gui Qt5::Widgets ${ZLIB_LIBRARIES} ${CURL_LIBRARY} ${PRICING_LIBRARY})

endif ()

option(ASAN_ENABLED "Build this target with AddressSanitizer" ON)

if(ASAN_ENABLED)
    if(MSVC)
        target_link_options(${PROJECT_NAME} PUBLIC /NODEFAULTLIB:libcmt.lib)
    else()

    endif()
endif()