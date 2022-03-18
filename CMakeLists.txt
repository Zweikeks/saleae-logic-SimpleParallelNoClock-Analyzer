cmake_minimum_required (VERSION 3.13)

project(SimpleParallelNoClock)

add_definitions( -DLOGIC2 )

# enable generation of compile_commands.json, helpful for IDEs to locate include files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES
include/AnalyzerHelpers.h
include/AnalyzerResults.h
include/SimulationChannelDescriptor.h
include/AnalyzerChannelData.h
include/Analyzer.h
include/LogicPublicTypes.h
include/AnalyzerSettingInterface.h
include/AnalyzerTypes.h
include/AnalyzerSettings.h
source/SimpleParallelNoClockAnalyzerResults.cpp
source/SimpleParallelNoClockAnalyzerSettings.cpp
source/SimpleParallelNoClockAnalyzer.h
source/SimpleParallelNoClockAnalyzer.cpp
source/SimpleParallelNoClockAnalyzerResults.h
source/SimpleParallelNoClockAnalyzerSettings.h
source/SimpleParallelNoClockSimulationDataGenerator.cpp
source/SimpleParallelNoClockSimulationDataGenerator.h
)

add_analyzer_plugin(${PROJECT_NAME} SOURCES ${SOURCES})
