QT += core
QT -= gui

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

TARGET = video_labeler
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += src/main.cpp \
           tracker/kcftracker.cpp \
           tracker/fhog.cpp

LIBS += \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgcodecs \
        -lopencv_imgproc

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
