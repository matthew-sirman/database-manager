cmake_minimum_required(VERSION 3.16)
project(database_manager)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS -pthread)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_UI ui/MainMenu.ui ui/MainMenu.cpp ui/MainMenu.h)
set(WIDGETS ui/widgets/DynamicComboBox.cpp ui/widgets/DynamicComboBox.h ui/widgets/ActivatorLabel.cpp ui/widgets/ActivatorLabel.h ui/widgets/AddDrawingPageWidget.ui ui/widgets/AddDrawingPageWidget.cpp ui/widgets/AddDrawingPageWidget.h ui/widgets/DrawingViewWidget.ui ui/widgets/DrawingViewWidget.cpp ui/widgets/DrawingViewWidget.h ui/widgets/DrawingView.cpp ui/widgets/DrawingView.h ui/widgets/DimensionLine.cpp ui/widgets/DimensionLine.h)

add_subdirectory(${PROJECT_SOURCE_DIR}/../encrypt encrypt-build)

include_directories(${PROJECT_SOURCE_DIR}/../encrypt/include)

add_executable(${PROJECT_NAME} ${WIDGETS} ${PROJECT_UI} main.cpp src/networking/Server.cpp include/networking/Server.h src/networking/Client.cpp include/networking/Client.h guard.h src/networking/NetworkMessage.cpp include/networking/NetworkMessage.h src/networking/TCPSocket.cpp include/networking/TCPSocket.h src/database/DatabaseManager.cpp include/database/DatabaseManager.h src/database/Drawing.cpp include/database/Drawing.h src/database/DatabaseRequestHandler.cpp include/database/DatabaseRequestHandler.h src/database/DatabaseQuery.cpp include/database/DatabaseQuery.h src/database/drawingComponents.cpp include/database/drawingComponents.h include/database/RequestType.h src/database/DatabaseResponseHandler.cpp include/database/DatabaseResponseHandler.h include/database/ComboboxDataSource.h src/database/ComboboxDataSource.cpp include/database/DataSource.h src/database/DrawingSearchResultsModel.cpp include/database/DrawingSearchResultsModel.h)

target_link_libraries(${PROJECT_NAME} encrypt mysqlcppconn Qt5::Core Qt5::Gui Qt5::Widgets)