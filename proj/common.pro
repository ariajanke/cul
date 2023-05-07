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
release {
    QMAKE_CXXFLAGS += -O3
    TARGET          = common-test-app
}

debug {
    TARGET          = common-test-app-d
    QMAKE_CXXFLAGS += -DMACRO_DEBUG
}
#message($$CONFIG)
contains(CONFIG, release) {
    TARGET = common-test-app
}

QMAKE_CXXFLAGS += -std=c++17 -DMACRO_ARIAJANKE_CUL_ENABLE_TYPELIST_TESTS
QMAKE_LFLAGS   += -std=c++17
LIBS           += -lsfml-graphics -lsfml-window -lsfml-system \
                  -L/usr/lib/x86_64-linux-gnu

SOURCES += \
    ../src/BitmapFont.cpp              \
    ../src/CurrentWorkingDirectory.cpp \
    \ # SFML Utilities
    ../src/sf-DrawText.cpp             \
    ../src/sf-DrawRectangle.cpp        \
    ../src/sf-DrawTriangle.cpp         \
    ../src/sf-Util.cpp                 \
    ../src/sf-DrawLine.cpp             \
    ../src/sf-8x8Font.cpp              \
    ../src/sf-8x16Font.cpp

HEADERS += \
    ../../../../../media/aria/UBUNTU/home/aria/dev/mapproject/src/GlmVectorTraits.hpp \
    ../inc/ariajanke/cul/BitmapFont.hpp              \
    ../inc/ariajanke/cul/Util.hpp                    \
    ../inc/ariajanke/cul/CurrentWorkingDirectory.hpp \
    ../inc/ariajanke/cul/BitMaskable.hpp             \
    ../inc/ariajanke/cul/StorageUnion.hpp            \
    ../inc/ariajanke/cul/MultiType.hpp               \
    ../inc/ariajanke/cul/TypeList.hpp                \
    ../inc/ariajanke/cul/TypeSet.hpp                 \
    ../inc/ariajanke/cul/StringUtil.hpp              \
    ../inc/ariajanke/cul/TestSuite.hpp               \
    ../inc/ariajanke/cul/Grid.hpp                    \
    ../inc/ariajanke/cul/ParseOptions.hpp            \
    ../inc/ariajanke/cul/SubGrid.hpp                 \
    ../inc/ariajanke/cul/Vector2.hpp                 \
    ../inc/ariajanke/cul/BezierCurves.hpp            \
    ../inc/ariajanke/cul/BezierCurvesDetails.hpp     \
    ../inc/ariajanke/cul/ColorString.hpp             \
    ../inc/ariajanke/cul/VectorTraits.hpp            \
    ../inc/ariajanke/cul/Vector3.hpp                 \
    ../inc/ariajanke/cul/RectangleUtils.hpp          \
    ../inc/ariajanke/cul/VectorUtils.hpp             \
    ../inc/ariajanke/cul/TreeTestSuite.hpp           \
    ../inc/ariajanke/cul/Either.hpp                  \
    ../inc/ariajanke/cul/FunctionTraits.hpp          \
    ../inc/ariajanke/cul/OptionalEither.hpp          \
    ../inc/ariajanke/cul/detail/either-helpers.hpp   \
    ../inc/ariajanke/cul/EitherFold.hpp              \
    \ # SFML Utilities
    ../inc/ariajanke/cul/sf/DrawText.hpp             \
    ../inc/ariajanke/cul/sf/DrawRectangle.hpp        \
    ../inc/ariajanke/cul/sf/DrawTriangle.hpp         \
    ../inc/ariajanke/cul/sf/DrawLine.hpp             \
    ../inc/ariajanke/cul/sf/Util.hpp                 \
    ../inc/ariajanke/cul/sf/VectorTraits.hpp         \
    \ # Private SFML Utility Headers
    ../src/sf-8x8Font.hpp                     \
    ../src/sf-8x16Font.hpp

# unit tests rounded into a single program (note, this is for dev/debug/IDE
# purposes, NOT for general/automated testing)
SOURCES += \
    \#../unit-tests/all-tests-driver.cpp \ # empty?!
    \#../unit-tests/TestGrid.cpp \
    \#../unit-tests/TestUtil.cpp \
    \#../unit-tests/TestMultiType.cpp \
    \#../unit-tests/test-string-utils.cpp \
    \#../unit-tests/test-math-utils.cpp \
    \#../unit-tests/vector-tests.cpp \
    \#../unit-tests/sfutils-tests.cpp \ # empty?!
    \#../demos/sf-util-demos.cpp \ # problems here
    \#../unit-tests/test-ColorString.cpp \
    \#../unit-tests/test-new-vectors.cpp \
    \#../unit-tests/sample-tree-test-suite.cpp \
    \#../unit-tests/test-tree-test-suite-p2.cpp \
    ../unit-tests/test-either.cpp

INCLUDEPATH += \
    ../inc
