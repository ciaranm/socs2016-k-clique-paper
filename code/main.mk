BUILD_DIR := intermediate
TARGET_DIR := ./
SUBMAKEFILES := file.mk about.mk

boost_ldlibs := -lboost_regex -lboost_thread -lboost_system -lboost_program_options

override CXXFLAGS += -O3 -march=native -std=c++14 -I./ -W -Wall -g -ggdb3 -pthread
override LDFLAGS += -pthread

TARGET := libclique.a

SOURCES := \
    sequential.cc \
    bit_graph.cc \
    graph.cc \
    degree_sort.cc \
    dimacs.cc \
    net.cc \
    metis.cc \
    power.cc \
    graph_file_error.cc \
    kneighbours.cc

TGT_LDLIBS := $(boost_ldlibs)

