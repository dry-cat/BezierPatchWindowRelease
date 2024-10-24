######################################################################
# Automatically generated by qmake (3.1) Wed Oct 16 16:36:14 2024
######################################################################

QT+=core gui widgets opengl openglwidgets
QMAKE_CXXFLAGS+= -D_GLIBCXX_PARALLEL -fopenmp -Wall -Wno-deprecated -Wno-deprecated-declarations
LIBS += -lGL -lGLU -fopenmp
TEMPLATE = app
TARGET = BezierPatchWindowRelease
INCLUDEPATH += .

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060000 # disables all APIs deprecated in Qt 6.0.0 and earlier

# Input
HEADERS += ArcBall.h \
           ArcBallWidget.h \
           BezierPatchRenderWidget.h \
           ControlPoints.h \
           Homogeneous4.h \
           Matrix4.h \
           Point3.h \
           Quaternion.h \
           RenderController.h \
           RenderParameters.h \
           RenderWidget.h \
           RenderWindow.h \
           RGBAImage.h \
           RGBAValue.h \
           Vector3.h
SOURCES += ArcBall.cpp \
           ArcBallWidget.cpp \
           BezierPatchRenderWidget.cpp \
           ControlPoints.cpp \
           Homogeneous4.cpp \
           main.cpp \
           Matrix4.cpp \
           Point3.cpp \
           Quaternion.cpp \
           RenderController.cpp \
           RenderWidget.cpp \
           RenderWindow.cpp \
           RGBAImage.cpp \
           RGBAValue.cpp \
           Vector3.cpp
