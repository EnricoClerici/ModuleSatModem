CONFIG += silent
PROJECT="ModuleSatModem"
BUILD_DIR = $$PWD

include("../Project.pri")
include("FrameworkQT.pri")

QMAKE_CXXFLAGS_WARN_ON += -Wall -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable

SOURCES +=  main.cpp \
            SatModemMod.cpp \
            SatModemDec.cpp \
            SatModemDev.cpp \
            SatModemMsg.cpp

HEADERS +=  \
            SatModemMod.h \
            SatModemDec.h \
            SatModemDev.h \
            SatModemMsg.h \
            SatModemRes.h

DISTFILES += \
            ../config/SatModem/COMMANDS.xml \
            ../config/SatModem/CONFIG.xml \
            ../config/SatModem/PARAMS.xml \
            ../config/SatModem/PARAMS.xml \
    config/SYSTEM.xml \
            \





LIBS +=  -lusb-1.0
INCLUDEPATH += $$APPLIB_DIR

#unix:!macx: LIBS += -L$$PWD/../../libuldaq-1.2.0/lib/ -luldaq
#unix:!macx: LIBS +=/usr/local/lib/ -luldaq
#LIBS +=  -L$$APPLIB_DIR/$$ARCHSUFF/$$QT_VERSION/lib -lAppLib



# commentare la seguente per buil x86
BBB = "1"

isEmpty (BBB) {
#    LIBS +=/usr/local/lib/ -luldaq
    LIBS +=  -L$$APPLIB_DIR/$$ARCHSUFF/$$QT_VERSION/lib -lAppLib

}
else {
#unix:!macx:



}


