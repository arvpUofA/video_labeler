QT += core
QT -= gui

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

TARGET = video_labeler
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cpp \
           tracker/kcftracker.cpp \
           tracker/fhog.cpp

# opencv3 from ros-kinetic-opencv3
INCLUDEPATH += /opt/ros/kinetic/include/opencv-3.2.0-dev
LIBS += -L/opt/ros/kinetic/lib -lopencv_core3 \
                               -lopencv_highgui3 \
                               -lopencv_imgcodecs3 \
                               -lopencv_imgproc3 \

HEADERS += \
           tracker/kcftracker.hpp \
           tracker/ffttools.hpp \
           tracker/fhog.hpp \
           tracker/recttools.hpp \
           tracker/tracker.h \
           tracker/labdata.hpp \
           tracker/ExtendedKalmanFilter.hpp \
           tracker/KalmanFilterBase.hpp \
           tracker/LinearizedMeasurementModel.hpp \
           tracker/LinearizedSystemModel.hpp \
           tracker/Matrix.hpp \
           tracker/MeasurementModel.hpp \
           tracker/SquareRootBase.hpp \
           tracker/SquareRootExtendedKalmanFilter.hpp \
           tracker/SquareRootFilterBase.hpp \
           tracker/SquareRootUnscentedKalmanFilter.hpp \
           tracker/StandardBase.hpp \
           tracker/StandardFilterBase.hpp \
           tracker/SystemModel.hpp \
           tracker/Types.hpp \
           tracker/UnscentedKalmanFilter.hpp \
           tracker/UnscentedKalmanFilterBase.hpp
