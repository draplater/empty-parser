cmake_minimum_required(VERSION 2.8)
project(xEisner)

SET(CMAKE_CXX_FLAGS "-std=c++11")
include_directories(${PROJECT_SOURCE_DIR})

if(NOT $ENV{MAX_SENTENCE_SIZE} MATCHES "^[0-9]+$")
    message(FATAL_ERROR "Must set valid MAX_SENTENCE_SIZE")
endif()
add_definitions(-DMAX_SENTENCE_SIZE=$ENV{MAX_SENTENCE_SIZE})

set(SOURCE_FILES
        common/parser/graph_dd/dual_decomposition2ec3rd.cpp
        common/parser/graph_dd/dual_decomposition2ec3rd.h
        common/parser/graph_dd/dual_decomposition2nd.cpp
        common/parser/graph_dd/dual_decomposition2nd.h
        common/parser/graph_dd/dual_decomposition2ndf.cpp
        common/parser/graph_dd/dual_decomposition2ndf.h
        common/parser/graph_dd/dual_decomposition3ec2nd.cpp
        common/parser/graph_dd/dual_decomposition3ec2nd.h
        common/parser/graph_dd/dual_decomposition3rd.cpp
        common/parser/graph_dd/dual_decomposition3rd.h
        common/parser/graph_dp/eisner2nd/eisner2nd_depparser.cpp
        common/parser/graph_dp/eisner2nd/eisner2nd_depparser.h
        common/parser/graph_dp/eisner2nd/eisner2nd_macros.cpp
        common/parser/graph_dp/eisner2nd/eisner2nd_macros.h
        common/parser/graph_dp/eisner2nd/eisner2nd_run.cpp
        common/parser/graph_dp/eisner2nd/eisner2nd_run.h
        common/parser/graph_dp/eisner2nd/eisner2nd_state.cpp
        common/parser/graph_dp/eisner2nd/eisner2nd_state.h
        common/parser/graph_dp/eisner3rd/eisner3rd_depparser.cpp
        common/parser/graph_dp/eisner3rd/eisner3rd_depparser.h
        common/parser/graph_dp/eisner3rd/eisner3rd_macros.cpp
        common/parser/graph_dp/eisner3rd/eisner3rd_macros.h
        common/parser/graph_dp/eisner3rd/eisner3rd_run.cpp
        common/parser/graph_dp/eisner3rd/eisner3rd_run.h
        common/parser/graph_dp/eisner3rd/eisner3rd_state.cpp
        common/parser/graph_dp/eisner3rd/eisner3rd_state.h
        common/parser/graph_dp/eisner3rd/eisner3rd_weight.cpp
        common/parser/graph_dp/eisner3rd/eisner3rd_weight.h
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_combdepparser.cpp
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_combdepparser.h
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_combrun.cpp
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_combrun.h
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_depparser.cpp
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_depparser.h
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_macros.cpp
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_macros.h
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_run.cpp
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_run.h
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_state.cpp
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_state.h
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_combdepparser.cpp
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_combdepparser.h
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_combrun.cpp
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_combrun.h
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_depparser.cpp
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_depparser.h
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_macros.cpp
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_macros.h
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_run.cpp
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_run.h
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_state.cpp
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_state.h
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_combdepparser.cpp
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_combdepparser.h
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_combrun.cpp
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_combrun.h
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_depparser.cpp
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_depparser.h
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_macros.cpp
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_macros.h
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_run.cpp
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_run.h
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_state.cpp
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_state.h
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_weight.cpp
        common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_weight.h
        common/parser/graph_dp/features/weight1st.cpp
        common/parser/graph_dp/features/weight1st.h
        common/parser/graph_dp/features/weight2nd.cpp
        common/parser/graph_dp/features/weight2nd.h
        common/parser/graph_dp/features/weight3rd.cpp
        common/parser/graph_dp/features/weight3rd.h
        common/parser/graph_dp/features/weightec2nd.cpp
        common/parser/graph_dp/features/weightec2nd.h
        common/parser/graph_dp/features/weightec3rd.cpp
        common/parser/graph_dp/features/weightec3rd.h
        common/parser/agenda.h
        common/parser/depparser_base.h
        common/parser/macros_base.cpp
        common/parser/macros_base.h
        common/parser/run_base.h
        common/parser/weight_base.h
        common/token/token.h
        include/learning/perceptron/packed_score.h
        include/learning/perceptron/score.h
        include/learning/perceptron/score_array.h
        include/ngram.h
        )

add_library(xEisner SHARED ${SOURCE_FILES}
        include/matrix_view.h
        common/parser/graph_dp/eisner2nd/eisner2nd_decode.cpp
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_decoder.cpp
        common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_decoder.h
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_decoder.cpp
        common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_decoder.h)
add_executable(xEisner_test ${SOURCE_FILES} common/parser/graph_dp/eisner2nd/eisner2nd_decode.cpp)
add_executable(xEisner_main ${SOURCE_FILES} main.cpp)
