TARGET := about_graph

SOURCES := \
    about_graph.cc

TGT_LDLIBS := -lclique $(boost_ldlibs)
TGT_LDFLAGS := -L${TARGET_DIR}
TGT_PREREQS := libclique.a

