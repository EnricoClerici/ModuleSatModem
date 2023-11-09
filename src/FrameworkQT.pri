TARGET=$$PROJECT
TEMPLATE = app

#---------------------------------------------------
# SOME BASIC CONFIGS
#---------------------------------------------------
CONFIG += c++11 console
CONFIG -= app_bundle
QT += network xml serialport

#---------------------------------------------------
# CREATE ARCHITECTURE FLAGS
#---------------------------------------------------
CONFIG(unix):ARCHSUFF = $$(IOS_ARCH)
CONFIG(mingw):ARCHSUFF = mingw

isEmpty(ARCHSUFF) {
 CONFIG(unix):ARCHSUFF = Linux
}
CONFIG(unix):ARCH = LINUX
CONFIG(mingw):ARCH = MINGW

#---------------------------------------------------
# GET GIT VERSION
#---------------------------------------------------
GIT_VERSION="NO VERSION CONTROL"
GIT_REMOTE = origin/master

IS_GIT = $$system(git rev-parse --is-inside-work-tree 2> /dev/null)
isEmpty(IS_GIT) {
} else {
    LC = $$system(git diff --name-only)
    isEmpty(LC) {
        LOCAL_CHANGES = ""
    } else {
        LOCAL_CHANGES = "- WITH NOT COMMITTED FILES"
    }

    HASH=$$system(git log --pretty=\"%H\" -n1)
    FOUND = $$system(git rev-list  $$GIT_REMOTE | grep $$HASH)
    isEmpty(FOUND) {
        PUSHED = " - HAS NOT BEEN NOT PUSHED YET"
    } else {
        PUSHED =""
    }

    GIT_VERSION = [$$system(git remote show -n origin | grep Push | cut -d: -f2-)]  $$system(git log  --pretty=\"Commmited %ci %h  %s\" -n1) $$PUSHED $$LOCAL_CHANGES
}


#------------------------------------------------
# CREATE DESTINATION DIRECTORIES
#---------------------------------------------------
DESTDIR     = $$BIN_DIR/$$ARCHSUFF/$$QT_VERSION/bin
OBJECTS_DIR = $$BUILD_DIR/$$ARCHSUFF/$$QT_VERSION/obj
MOC_DIR     = $$BUILD_DIR/$$ARCHSUFF/$$QT_VERSION/moc



#------------------------------------------------
# BUILDING FLAGS
#---------------------------------------------------
QMAKE_CXXFLAGS_WARN_ON += -Wall -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
INCLUDEPATH += $$APPLIB_DIR
LIBS +=  -L$$APPLIB_DIR/$$ARCHSUFF/$$QT_VERSION/lib -lAppLib

DEFINES += $$ARCH \
           BUILD=\"'\\"$$GIT_VERSION\\"'\" \
           BUILD_TARGET=\"'\\"$$TARGET\\"'\"




