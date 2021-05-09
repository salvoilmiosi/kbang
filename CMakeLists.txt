cmake_minimum_required(VERSION 3.0.0)
project(kbang VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5Script REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(common_SRCS
src/common/parser/ioproxy.cpp
src/common/parser/parser.cpp
src/common/parser/parserstructs.cpp
src/common/parser/queryget.cpp
src/common/parser/queryresult.cpp
src/common/parser/xmlnode.cpp
src/common/config.cpp
src/common/util.cpp
src/common/gameenums.cpp
)

add_library(kbang_common ${common_SRCS})
target_link_libraries(kbang_common Qt5::Xml Qt5::Gui)
include_directories(src/common)

set(client_SRCS
src/client/gameloop.cpp
src/client/main.cpp
src/client/connecttoserverdialog.cpp
src/client/mainwindow.cpp
src/client/serverconnection.cpp
src/client/common.cpp
src/client/joingamedialog.cpp
src/client/logwidget.cpp
src/client/chatwidget.cpp
src/client/card.cpp
src/client/opponentwidget.cpp
src/client/game.cpp
src/client/cardwidget.cpp
src/client/creategamedialog.cpp
src/client/playerwidget.cpp
src/client/characterwidget.cpp
src/client/cardpilewidget.cpp
src/client/deckwidget.cpp
src/client/localplayerwidget.cpp
src/client/cardlistwidget.cpp
src/client/gameevent.cpp
src/client/cardmovementevent.cpp
src/client/gameeventqueue.cpp
src/client/gameeventhandler.cpp
src/client/gamecontextchangeevent.cpp
src/client/gamesyncevent.cpp
src/client/lifepointschangeevent.cpp
src/client/cardactionswidget.cpp
src/client/cardwidgetfactory.cpp
src/client/gameactionmanager.cpp
src/client/cardpocket.cpp
src/client/playerdiedevent.cpp
src/client/playerevent.cpp
src/client/graveyardwidget.cpp
src/client/cardzoomwidget.cpp
src/client/newserverdialog.cpp
src/client/selectplayericonwidget.cpp
src/client/gamemessageevent.cpp
src/client/cardwidgetsizemanager.cpp
src/client/aboutdialog.cpp
)

set(client_RES
kbang_client.rc
)

set(client_UI
src/client/connecttoserverdialog.ui
src/client/mainwindow.ui
src/client/joingamedialog.ui
src/client/logwidget.ui
src/client/chatwidget.ui
src/client/opponentwidget.ui
src/client/creategamedialog.ui
src/client/localplayerwidget.ui
src/client/newserverdialog.ui
src/client/aboutdialog.ui
)

qt5_add_resources(client_SRCS src/client/resources/client.qrc)
add_executable(kbang_client ${client_SRCS} ${client_UI} ${client_RES})
target_link_libraries(kbang_client kbang_common Qt5::Widgets Qt5::Multimedia Qt5::Network Qt5::Xml Qt5::Script)
target_include_directories(kbang_client PRIVATE src/client)

set(server_SRCS
src/server/main.cpp
src/server/player.cpp
src/server/gameserver.cpp
src/server/game.cpp
src/server/console.cpp
src/server/consolecommand.cpp
src/server/client.cpp
src/server/tcpserver.cpp
src/server/common.cpp
src/server/publicgameview.cpp
src/server/publicplayerview.cpp
src/server/privateplayerview.cpp
src/server/playingcard.cpp
src/server/reactioncard.cpp
src/server/tablecard.cpp
src/server/weaponcard.cpp
src/server/cardbang.cpp
src/server/cardmissed.cpp
src/server/gamecycle.cpp
src/server/gameinfo.cpp
src/server/gametable.cpp
src/server/playerctrl.cpp
src/server/voidai.cpp
src/server/gameeventlistener.cpp
src/server/cardbeer.cpp
src/server/cardhorse.cpp
src/server/cardfactory.cpp
src/server/carddrawcards.cpp
src/server/cardmultishoot.cpp
src/server/cardduel.cpp
src/server/cardgeneralstore.cpp
src/server/cardjail.cpp
src/server/reactionhandler.cpp
src/server/carddynamite.cpp
src/server/cardtaker.cpp
src/server/cardbarrel.cpp
src/server/gameeventmanager.cpp
src/server/gamelogger.cpp
src/server/characterbase.cpp
src/server/charactercalamityjanet.cpp
src/server/characterblackjack.cpp
src/server/characterlist.cpp
src/server/charactercassidygringo.cpp
src/server/charactersuzylafayette.cpp
src/server/characterpropertychange.cpp
src/server/characterjourdonnais.cpp
src/server/characterpedroramirez.cpp
src/server/characterjessejones.cpp
src/server/charactersidketchum.cpp
src/server/characterkitcarlson.cpp
src/server/playerreaper.cpp
src/server/charactervulturesam.cpp
src/server/characterluckyduke.cpp
)

add_executable(kbang_server ${server_SRCS})
target_link_libraries(kbang_server kbang_common Qt5::Network Qt5::Xml)
target_include_directories(kbang_server PRIVATE src/server)