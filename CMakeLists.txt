cmake_minimum_required(VERSION 3.16)
project(database_manager)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
set(CMAKE_CXX_FLAGS)
else()
set(CMAKE_CXX_FLAGS -pthread)
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Pdf PdfWidgets Svg)

if (WIN32)

include_directories(
    ${CURL_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
)

endif()

include_directories(${PROJECT_SOURCE_DIR}/ui)

add_definitions(-DQT_NO_VERSION_TAGGING -DWIN32_LEAN_AND_MEAN -DCURL_STATICLIB)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_UI ui/MainMenu.ui ui/MainMenu.cpp ui/MainMenu.h)
set(WIDGETS ui/widgets/DynamicComboBox.cpp ui/widgets/DynamicComboBox.h ui/widgets/ActivatorLabel.cpp ui/widgets/ActivatorLabel.h ui/widgets/AddDrawingPageWidget.ui ui/widgets/AddDrawingPageWidget.cpp ui/widgets/AddDrawingPageWidget.h ui/widgets/DrawingViewWidget.ui ui/widgets/DrawingViewWidget.cpp ui/widgets/DrawingViewWidget.h ui/widgets/DrawingView.cpp ui/widgets/DrawingView.h ui/widgets/DimensionLine.cpp ui/widgets/DimensionLine.h ui/widgets/AddLapWidget.cpp ui/widgets/AddLapWidget.h "ui/widgets/ImpactPadGraphicsItem.h" "ui/widgets/ImpactPadGraphicsItem.cpp" "ui/widgets/ExpandingWidget.h" "ui/widgets/ExpandingWidget.cpp" "ui/widgets/Inspector.h" "ui/widgets/Inspector.cpp")
set(COMPONENT_WINDOWS ui/AddApertureWindow.ui ui/AddApertureWindow.cpp ui/AddApertureWindow.h ui/AddSideIronWindow.ui ui/AddSideIronWindow.cpp ui/AddSideIronWindow.h ui/AddMaterialWindow.ui ui/AddMaterialWindow.cpp ui/AddMaterialWindow.h ui/AddMachineWindow.ui ui/AddMachineWindow.cpp ui/AddMachineWindow.h)
set(QT_RESOURCES res/qtresources.qrc)

add_subdirectory(${PROJECT_SOURCE_DIR}/../encrypt encrypt-build)

add_executable(${PROJECT_NAME} ${EXTRA_ADD_EXECUTABLE_ARGS} ${COMPONENT_WINDOWS} ${WIDGETS} ${PROJECT_UI} ${QT_RESOURCES} main.cpp src/networking/Server.cpp include/networking/Server.h src/networking/Client.cpp include/networking/Client.h guard.h src/networking/NetworkMessage.cpp include/networking/NetworkMessage.h src/networking/TCPSocket.cpp include/networking/TCPSocket.h src/database/DatabaseManager.cpp include/database/DatabaseManager.h src/database/Drawing.cpp include/database/Drawing.h src/database/DatabaseRequestHandler.cpp include/database/DatabaseRequestHandler.h src/database/DatabaseQuery.cpp include/database/DatabaseQuery.h src/database/drawingComponents.cpp include/database/drawingComponents.h include/database/RequestType.h src/database/DatabaseResponseHandler.cpp include/database/DatabaseResponseHandler.h include/database/ComboboxDataSource.h src/database/ComboboxDataSource.cpp include/database/DataSource.h src/database/DrawingSearchResultsModel.cpp include/database/DrawingSearchResultsModel.h packer.h include/database/DrawingPDFWriter.h src/database/DrawingPDFWriter.cpp   "include/database/componentFilters.h" "src/database/componentFilters.cpp" "include/util/format.h")

include_directories(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/../encrypt/include)

if (MSVC)

add_compile_options(
    #$<$<CONFIG:>:/MT>
    #$<$<CONFIG:Debug>:/MTd>
    #$<$<CONFIG:Release>:/MT>
    "/std:c++17"
    "/wd4068"
)
target_link_libraries(${PROJECT_NAME} encrypt Dnsapi ${MYSQL_LIBRARY} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Pdf Qt5::PdfWidgets Qt5::Svg)

else()

target_link_libraries(${PROJECT_NAME} encrypt mysqlcppconn Qt5::Core Qt5::Gui Qt5::Widgets)

endif ()

