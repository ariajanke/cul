#-----------------------------------------------------
#
# Modified from automatically generated configuration
#
#-----------------------------------------------------

QT      -= core gui
# try to make this a test application
# TEMPLATE = lib
#CONFIG  += staticlib
CONFIG  -= c++11

linux {
    contains(QT_ARCH, i386) {
        target.path = ../../bin/linux/g++-x86
    } else:contains(QT_ARCH, x86_64) {
        target.path = ../../bin/linux/g++-x86_64
    }

    INSTALLS       += target
    QMAKE_CXXFLAGS += -DMACRO_PLATFORM_LINUX
}

debug {
    CONFIG -= release
}

debug {
    TARGET          = common-test-app-d
    QMAKE_CXXFLAGS += -DMACRO_DEBUG
}
#message($$CONFIG)
contains(CONFIG, release) {
    TARGET = common-test-app
}

QMAKE_CXXFLAGS += -std=c++17
QMAKE_LFLAGS   += -std=c++17
LIBS           += -lsfml-graphics \
                  -L/usr/lib/x86_64-linux-gnu

SOURCES += \
    ../src/ConstString.cpp             \
    ../src/CurrentWorkingDirectory.cpp \
    ../src/DrawRectangle.cpp           \
    ../src/Util.cpp                    \
    ../src/TestSuite.cpp               \
    ../src/DrawTriangle.cpp

HEADERS += \
    ../inc/common/ConstString.hpp             \
    ../inc/common/DrawRectangle.hpp           \
    ../inc/common/Util.hpp                    \
    ../inc/common/CurrentWorkingDirectory.hpp \
    ../inc/common/BitMaskable.hpp             \
    ../inc/common/StorageUnion.hpp            \
    ../inc/common/MultiType.hpp               \
    ../inc/common/TypeList.hpp                \
    ../inc/common/StringUtil.hpp              \
    ../inc/common/TestSuite.hpp               \
    ../inc/common/Grid.hpp                    \
    ../inc/common/ParseOptions.hpp            \
    ../inc/common/SubGrid.hpp                 \
    ../inc/common/DrawTriangle.hpp \
    ../inc/common/Vector2Util.hpp \
    ../inc/common/Vector2Traits.hpp \
    ../inc/common/Vector2.hpp \
    ../inc/common/SfmlVectorTraits.hpp

# unit tests rounded into a single program (note, this is for dev/debug/IDE
# purposes, NOT for general/automated testing)
SOURCES += \
    \#../unit-tests/all-tests-driver.cpp \
    \#../unit-tests/TestGrid.cpp \
    \#../unit-tests/TestUtil.cpp \
    \#../unit-tests/TestFixedLengthArray.cpp \
    \#../unit-tests/TestMultiType.cpp \
    \#../unit-tests/test-string-utils.cpp
    \#../unit-tests/test-math-utils.cpp
    ../unit-tests/vector-tests.cpp


INCLUDEPATH += \
    ../inc
