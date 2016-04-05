TARGET := solve_max_clique

SOURCES := \
    solve_max_clique.cc

TGT_LDLIBS := -lclique $(boost_ldlibs)
TGT_LDFLAGS := -L${TARGET_DIR}
TGT_PREREQS := libclique.a

