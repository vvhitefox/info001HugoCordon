#set(Qt5Core_DIR "/home/huj/home/5.15.2/gcc_64/lib/cmake/Qt5Core")
#set(Qt5_DIR "/home/huj/home/5.15.2/gcc_64/lib/cmake/Qt5")
#set(QT_QMAKE_EXECUTABLE "/home/huj/home/5.15.2/gcc_64/bin/qmake")

cmake_minimum_required(VERSION 3.5)
project( histogrammes )
set (CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )
#find_package( Qt5 COMPONENTS Core REQUIRED  )
#find_package( Qt5 COMPONENTS Gui REQUIRED  )
include_directories( ${OpenCV_INCLUDE_DIRS} )

SET( MAIN_EXEC
     minimum-prog
     # vous pouvez ajouter d'autres programmes ici
     )

FOREACH(FILE ${MAIN_EXEC})
            add_executable( ${FILE} ${FILE}.cpp )
            target_link_libraries( ${FILE} ${OpenCV_LIBS} )
ENDFOREACH(FILE)
