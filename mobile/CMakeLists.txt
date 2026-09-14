cmake_minimum_required(VERSION 3.20)

project(AlienMobilePrototype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ALIEN_MOBILE_CORE_SOURCES
    core/src/CpuPhysicsBackend.cpp
    core/src/Damage.cpp
    core/src/StructuralMutation.cpp
    core/src/Development.cpp
    core/src/Genome.cpp
    core/src/CatalogGenomes.cpp
    core/src/Behavior.cpp
    core/src/Ecology.cpp
    core/src/PhysicsScenario.cpp
    core/src/World.cpp
    core/src/Simulation.cpp
)

add_library(AlienMobileCore STATIC ${ALIEN_MOBILE_CORE_SOURCES})
target_include_directories(AlienMobileCore PUBLIC core/include)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    enable_language(OBJCXX)

    find_library(UIKIT_FRAMEWORK UIKit REQUIRED)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)

    add_executable(AlienMobileApp MACOSX_BUNDLE
        ios/main.mm
        ios/AppDelegate.mm
        ios/GameViewController.mm
        ios/CreatureEditor.mm
        ios/MetalPhysicsBackend.mm
        ios/Renderer.mm
    )
    target_compile_options(AlienMobileApp PRIVATE -fobjc-arc)
    if(CMAKE_OSX_SYSROOT MATCHES "iphonesimulator")
        set(ALIEN_METAL_SDK iphonesimulator)
    else()
        set(ALIEN_METAL_SDK iphoneos)
    endif()
    set(METAL_AIR "${CMAKE_CURRENT_BINARY_DIR}/AlienMobileShaders.air")
    set(METAL_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/AlienMobileShaders.metallib")
    add_custom_command(
        OUTPUT "${METAL_AIR}"
        COMMAND xcrun -sdk ${ALIEN_METAL_SDK} metal -c "${CMAKE_CURRENT_SOURCE_DIR}/ios/Shaders.metal" -o "${METAL_AIR}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ios/Shaders.metal"
        COMMENT "Compiling Metal shaders"
    )
    add_custom_command(
        OUTPUT "${METAL_LIBRARY}"
        COMMAND xcrun -sdk ${ALIEN_METAL_SDK} metallib "${METAL_AIR}" -o "${METAL_LIBRARY}"
        DEPENDS "${METAL_AIR}"
        COMMENT "Linking Metal shaders"
    )
    add_custom_target(AlienMobileShaders ALL DEPENDS "${METAL_LIBRARY}")
    add_dependencies(AlienMobileApp AlienMobileShaders)
    set_source_files_properties("${METAL_LIBRARY}" PROPERTIES GENERATED TRUE MACOSX_PACKAGE_LOCATION Resources)
    target_sources(AlienMobileApp PRIVATE "${METAL_LIBRARY}")
    target_include_directories(AlienMobileApp PRIVATE core/include ios)
    target_compile_definitions(AlienMobileApp PRIVATE ALIENMOBILE_IOS=1)
    target_link_libraries(AlienMobileApp PRIVATE
        AlienMobileCore
        ${UIKIT_FRAMEWORK}
        ${METAL_FRAMEWORK}
        ${METALKIT_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
        ${COREGRAPHICS_FRAMEWORK}
    )
    set_target_properties(AlienMobileApp PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.example.AlienMobilePrototype"
    )
else()
    enable_testing()
    add_executable(AlienMobileHotpathTests tests/HotpathTests.cpp)
    target_compile_options(AlienMobileHotpathTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileHotpathTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileHotpathTests COMMAND AlienMobileHotpathTests)
    add_executable(AlienMobileHotspotBenchmark tests/HotspotBenchmark.cpp)
    target_link_libraries(AlienMobileHotspotBenchmark PRIVATE AlienMobileCore)
    add_executable(AlienMobileCompetitionResearch tests/CompetitionResearch.cpp)
    target_link_libraries(AlienMobileCompetitionResearch PRIVATE AlienMobileCore)
    add_executable(AlienMobileCompetitionObserverTests tests/CompetitionObserverTests.cpp)
    target_compile_options(AlienMobileCompetitionObserverTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileCompetitionObserverTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileCompetitionObserverTests COMMAND AlienMobileCompetitionObserverTests)
    add_executable(AlienMobileResourceBedTests tests/ResourceBedTests.cpp)
    target_compile_options(AlienMobileResourceBedTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileResourceBedTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileResourceBedTests COMMAND AlienMobileResourceBedTests)
    add_executable(AlienMobilePlaytestWorldTests tests/PlaytestWorldTests.cpp)
    target_compile_options(AlienMobilePlaytestWorldTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobilePlaytestWorldTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobilePlaytestWorldTests COMMAND AlienMobilePlaytestWorldTests)
    add_executable(AlienMobileDevelopingOrgansTests tests/DevelopingOrgansTests.cpp)
    target_compile_options(AlienMobileDevelopingOrgansTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileDevelopingOrgansTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileDevelopingOrgansTests COMMAND AlienMobileDevelopingOrgansTests)
    add_executable(AlienMobileEvolvabilityTests tests/EvolvabilityTests.cpp)
    target_compile_options(AlienMobileEvolvabilityTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileEvolvabilityTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileEvolvabilityTests COMMAND AlienMobileEvolvabilityTests)
    add_executable(AlienMobileComplexityEconomyTests tests/ComplexityEconomyTests.cpp)
    target_compile_options(AlienMobileComplexityEconomyTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileComplexityEconomyTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileComplexityEconomyTests COMMAND AlienMobileComplexityEconomyTests)
    add_executable(AlienMobileEvolutionResearch tests/EvolutionResearch.cpp)
    target_link_libraries(AlienMobileEvolutionResearch PRIVATE AlienMobileCore)
    add_executable(AlienMobileMutationContinuityTests tests/MutationContinuityTests.cpp)
    target_compile_options(AlienMobileMutationContinuityTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileMutationContinuityTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileMutationContinuityTests COMMAND AlienMobileMutationContinuityTests)
    add_executable(AlienMobileSpatialTests tests/SpatialTests.cpp)
    target_compile_options(AlienMobileSpatialTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileSpatialTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileSpatialTests COMMAND AlienMobileSpatialTests)
    add_executable(AlienMobileFunctionalTests tests/FunctionalTests.cpp)
    target_compile_options(AlienMobileFunctionalTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileFunctionalTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileFunctionalTests COMMAND AlienMobileFunctionalTests)
    add_executable(AlienMobileDamageTests tests/DamageTests.cpp)
    target_compile_options(AlienMobileDamageTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileDamageTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileDamageTests COMMAND AlienMobileDamageTests)
    add_executable(AlienMobileArticulationTests tests/ArticulationTests.cpp)
    target_compile_options(AlienMobileArticulationTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileArticulationTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileArticulationTests COMMAND AlienMobileArticulationTests)
    add_executable(AlienMobileDepthSession tests/DepthSessionTests.cpp)
    target_compile_options(AlienMobileDepthSession PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileDepthSession PRIVATE AlienMobileCore)
    add_executable(AlienMobileStructuralMutationTests tests/StructuralMutationTests.cpp)
    target_compile_options(AlienMobileStructuralMutationTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileStructuralMutationTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileStructuralMutationTests COMMAND AlienMobileStructuralMutationTests)
    add_executable(AlienMobileDevelopmentTests tests/DevelopmentTests.cpp)
    target_compile_options(AlienMobileDevelopmentTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileDevelopmentTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileDevelopmentTests COMMAND AlienMobileDevelopmentTests)
    add_executable(AlienMobileLocomotionDiversityTests tests/LocomotionDiversityTests.cpp)
    target_compile_options(AlienMobileLocomotionDiversityTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileLocomotionDiversityTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileLocomotionDiversityTests COMMAND AlienMobileLocomotionDiversityTests)
    add_executable(AlienMobileHuntingTests tests/HuntingTests.cpp)
    target_compile_options(AlienMobileHuntingTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileHuntingTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileHuntingTests COMMAND AlienMobileHuntingTests)
    add_executable(AlienMobileEcologyTests tests/EcologyTests.cpp)
    target_compile_options(AlienMobileEcologyTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileEcologyTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileEcologyTests COMMAND AlienMobileEcologyTests)
    add_executable(AlienMobileBehaviorSelectionTests tests/BehaviorSelectionTests.cpp)
    target_compile_options(AlienMobileBehaviorSelectionTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileBehaviorSelectionTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileBehaviorSelectionTests COMMAND AlienMobileBehaviorSelectionTests)
    add_executable(AlienMobileBehaviorMutationTests tests/BehaviorMutationTests.cpp)
    target_compile_options(AlienMobileBehaviorMutationTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileBehaviorMutationTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileBehaviorMutationTests COMMAND AlienMobileBehaviorMutationTests)
    add_executable(AlienMobileBehaviorTests tests/BehaviorTests.cpp)
    target_compile_options(AlienMobileBehaviorTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileBehaviorTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileBehaviorTests COMMAND AlienMobileBehaviorTests)
    add_executable(AlienMobileGardenContinuityTests tests/GardenContinuityTests.cpp)
    target_compile_options(AlienMobileGardenContinuityTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileGardenContinuityTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileGardenContinuityTests COMMAND AlienMobileGardenContinuityTests)
    add_executable(AlienMobileSpecimenCatalogTests tests/SpecimenCatalogTests.cpp)
    target_compile_options(AlienMobileSpecimenCatalogTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileSpecimenCatalogTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileSpecimenCatalogTests COMMAND AlienMobileSpecimenCatalogTests)
    add_executable(AlienMobileCatalogEcologyTests tests/CatalogEcologyTests.cpp)
    target_compile_options(AlienMobileCatalogEcologyTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileCatalogEcologyTests PRIVATE AlienMobileCore)
    add_test(NAME AlienMobileCatalogEcologyTests COMMAND AlienMobileCatalogEcologyTests)

    add_executable(AlienMobileCoreTests tests/CoreTests.cpp)
    target_compile_options(AlienMobileCoreTests PRIVATE -UNDEBUG)
    target_link_libraries(AlienMobileCoreTests PRIVATE AlienMobileCore)
    target_include_directories(AlienMobileCoreTests PRIVATE core/include)

    add_test(NAME AlienMobileCoreTests COMMAND AlienMobileCoreTests)
    add_executable(AlienMobileSessionTests tests/SessionTests.cpp)
    target_link_libraries(AlienMobileSessionTests PRIVATE AlienMobileCore)
    target_compile_options(AlienMobileSessionTests PRIVATE -UNDEBUG)
    add_test(NAME AlienMobileSessionTests COMMAND AlienMobileSessionTests)
    if(APPLE)
        enable_language(OBJCXX)
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/HostShaders.metallib"
            COMMAND xcrun -sdk macosx metal -c "${CMAKE_CURRENT_SOURCE_DIR}/ios/Shaders.metal" -o "${CMAKE_CURRENT_BINARY_DIR}/HostShaders.air"
            COMMAND xcrun -sdk macosx metallib "${CMAKE_CURRENT_BINARY_DIR}/HostShaders.air" -o "${CMAKE_CURRENT_BINARY_DIR}/HostShaders.metallib"
            DEPENDS ios/Shaders.metal)
        add_custom_target(AlienMobileHostShaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/HostShaders.metallib")
        add_executable(AlienMobileMetalTests tests/MetalTests.mm ios/MetalPhysicsBackend.mm)
        target_include_directories(AlienMobileMetalTests PRIVATE ios)
        target_compile_options(AlienMobileMetalTests PRIVATE -fobjc-arc -UNDEBUG)
        target_link_libraries(AlienMobileMetalTests PRIVATE AlienMobileCore "-framework Metal" "-framework Foundation" "-framework QuartzCore")
        add_dependencies(AlienMobileMetalTests AlienMobileHostShaders)
        add_executable(AlienMobileMetalGridTests tests/MetalGridTests.mm ios/MetalPhysicsBackend.mm)
        target_include_directories(AlienMobileMetalGridTests PRIVATE ios)
        target_compile_options(AlienMobileMetalGridTests PRIVATE -fobjc-arc -UNDEBUG)
        target_link_libraries(AlienMobileMetalGridTests PRIVATE AlienMobileCore "-framework Metal" "-framework Foundation" "-framework QuartzCore")
        add_dependencies(AlienMobileMetalGridTests AlienMobileHostShaders)
        add_test(NAME AlienMobileMetalGridTests COMMAND AlienMobileMetalGridTests "${CMAKE_CURRENT_BINARY_DIR}/HostShaders.metallib")
        add_test(NAME AlienMobileMetalTests COMMAND AlienMobileMetalTests "${CMAKE_CURRENT_BINARY_DIR}/HostShaders.metallib")
        set_tests_properties(AlienMobileMetalTests PROPERTIES SKIP_RETURN_CODE 77)
    endif()
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobileCatalogPortfolio tests/CatalogPortfolio.cpp)
    target_link_libraries(AlienMobileCatalogPortfolio PRIVATE AlienMobileCore)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobileCatalogDiscovery tests/CatalogDiscovery.cpp)
    target_link_libraries(AlienMobileCatalogDiscovery PRIVATE AlienMobileCore)
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobileDiscoveryCatalogTests tests/DiscoveryCatalogTests.cpp)
    target_link_libraries(AlienMobileDiscoveryCatalogTests PRIVATE AlienMobileCore)
    target_compile_options(AlienMobileDiscoveryCatalogTests PRIVATE -UNDEBUG)
    target_compile_definitions(AlienMobileDiscoveryCatalogTests PRIVATE ALIEN_DISCOVERY_ARCHIVE="${CMAKE_CURRENT_SOURCE_DIR}/docs/discovery/finalists")
    add_test(NAME AlienMobileDiscoveryCatalogTests COMMAND AlienMobileDiscoveryCatalogTests)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobileMutagenTests tests/MutagenTests.cpp)
    target_link_libraries(AlienMobileMutagenTests PRIVATE AlienMobileCore)
    target_compile_options(AlienMobileMutagenTests PRIVATE -UNDEBUG)
    add_test(NAME AlienMobileMutagenTests COMMAND AlienMobileMutagenTests)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobilePlayerExperimentTests tests/PlayerExperimentTests.cpp)
    target_link_libraries(AlienMobilePlayerExperimentTests PRIVATE AlienMobileCore)
    target_compile_options(AlienMobilePlayerExperimentTests PRIVATE -UNDEBUG)
    add_test(NAME AlienMobilePlayerExperimentTests COMMAND AlienMobilePlayerExperimentTests)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobileCreatorTests tests/CreatorTests.cpp)
    target_link_libraries(AlienMobileCreatorTests PRIVATE AlienMobileCore)
    target_compile_options(AlienMobileCreatorTests PRIVATE -UNDEBUG)
    add_test(NAME AlienMobileCreatorTests COMMAND AlienMobileCreatorTests)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobileHybridPlaytestObserver tests/HybridPlaytestObserver.cpp)
    target_link_libraries(AlienMobileHybridPlaytestObserver PRIVATE AlienMobileCore)
    target_compile_options(AlienMobileHybridPlaytestObserver PRIVATE -UNDEBUG)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobileCreatorCompetenceObserver tests/CreatorCompetenceObserver.cpp)
    target_link_libraries(AlienMobileCreatorCompetenceObserver PRIVATE AlienMobileCore)
    target_compile_options(AlienMobileCreatorCompetenceObserver PRIVATE -UNDEBUG)
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(AlienMobileCreatorCompetenceTests tests/CreatorCompetenceTests.cpp)
    target_link_libraries(AlienMobileCreatorCompetenceTests PRIVATE AlienMobileCore)
    target_compile_options(AlienMobileCreatorCompetenceTests PRIVATE -UNDEBUG)
    add_test(NAME AlienMobileCreatorCompetenceTests COMMAND AlienMobileCreatorCompetenceTests)
endif()
