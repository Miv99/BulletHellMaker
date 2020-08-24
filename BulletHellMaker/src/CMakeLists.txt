set(BHM_SRC
    Main.cpp
    DataStructs/IDGenerator.cpp
    DataStructs/MovablePoint.cpp
    DataStructs/SpriteEffectAnimation.cpp
    DataStructs/SpriteLoader.cpp
    DataStructs/SymbolTable.cpp
    DataStructs/TimeFunctionVariable.cpp
    DataStructs/UndoStack.cpp
    Editor/CopyPaste.cpp
    Editor/EditorWindow.cpp
    Editor/ViewController.cpp
    Editor/Attack/AttackEditorPanel.cpp
    Editor/Attack/AttackEditorPropertiesPanel.cpp
    Editor/Attack/EditorMovablePointTreePanel.cpp
    Editor/AttackPattern/AttackPatternEditorPanel.cpp
    Editor/AttackPattern/AttackPatternEditorPropertiesPanel.cpp
    Editor/CustomWidgets/AnimatableChooser.cpp
    Editor/CustomWidgets/AnimatablePicture.cpp
    Editor/CustomWidgets/BezierControlPointsPlacer.cpp
    Editor/CustomWidgets/ClickableTimeline.cpp
    Editor/CustomWidgets/DelayedSlider.cpp
    Editor/CustomWidgets/EditBox.cpp
    Editor/CustomWidgets/EMPAAngleOffsetGroup.cpp
    Editor/CustomWidgets/EMPAListVisualizer.cpp
    Editor/CustomWidgets/HideableGroup.cpp
    Editor/CustomWidgets/ListBoxScrollablePanel.cpp
    Editor/CustomWidgets/ListViewScrollablePanel.cpp
    Editor/CustomWidgets/MarkerPlacer.cpp
    Editor/CustomWidgets/NumericalEditBoxWithLimits.cpp
    Editor/CustomWidgets/SimpleEngineRenderer.cpp
    Editor/CustomWidgets/SingleMarkerPlacer.cpp
    Editor/CustomWidgets/Slider.cpp
    Editor/CustomWidgets/SliderWithEditBox.cpp
    Editor/CustomWidgets/SoundSettingsGroup.cpp
    Editor/CustomWidgets/SymbolTableEditor.cpp
    Editor/CustomWidgets/TabsWithPanel.cpp
    Editor/CustomWidgets/TextNotification.cpp
    Editor/CustomWidgets/TFVGroup.cpp
    Editor/CustomWidgets/TimedLabel.cpp
    Editor/EMP/EditorMovablePointPanel.cpp
    Editor/EMPA/EditorMovablePointActionPanel.cpp
    Editor/EMPA/EMPABasedMovementEditorPanel.cpp
    Editor/LevelPackObjectList/LevelPackObjectsListPanel.cpp
    Editor/LevelPackObjectList/LevelPackObjectsListView.cpp
    Editor/LevelPackObjectUseRelationship/LevelPackObjectUseRelationshipEditor.cpp
    Editor/Previewer/LevelPackObjectPreviewPanel.cpp
    Editor/Util/EditorUtils.cpp
    Editor/Util/ExtraSignals.cpp
    Game/AudioPlayer.cpp
    Game/EntityCreationQueue.cpp
    Game/GameInstance.cpp
    Game/Components/AnimatableSetComponent.cpp
    Game/Components/CollectibleComponent.cpp
    Game/Components/DespawnComponent.cpp
    Game/Components/EMPSpawnerComponent.cpp
    Game/Components/EnemyBulletComponent.cpp
    Game/Components/EnemyComponent.cpp
    Game/Components/HealthComponent.cpp
    Game/Components/HitboxComponent.cpp
    Game/Components/LevelManagerTag.cpp
    Game/Components/MovementPathComponent.cpp
    Game/Components/PlayerBulletComponent.cpp
    Game/Components/PlayerTag.cpp
    Game/Components/PositionComponent.cpp
    Game/Components/ShadowTrailComponent.cpp
    Game/Components/SpriteComponent.cpp
    Game/Systems/CollectibleSystem.cpp
    Game/Systems/CollisionSystem.cpp
    Game/Systems/DebugRenderSystem.cpp
    Game/Systems/DespawnSystem.cpp
    Game/Systems/EnemySystem.cpp
    Game/Systems/MovementSystem.cpp
    Game/Systems/PlayerSystem.cpp
    Game/Systems/ShadowTrailSystem.cpp
    Game/Systems/SpriteAnimationSystem.cpp
    Game/Systems/RenderSystem/BlurEffect.cpp
    Game/Systems/RenderSystem/PostEffect.cpp
    Game/Systems/RenderSystem/RenderSystem.cpp
    LevelPack/Animatable.cpp
    LevelPack/Animation.cpp
    LevelPack/Attack.cpp
    LevelPack/AttackPattern.cpp
    LevelPack/DeathAction.cpp
    LevelPack/EditorMovablePoint.cpp
    LevelPack/EditorMovablePointAction.cpp
    LevelPack/EditorMovablePointSpawnType.cpp
    LevelPack/Enemy.cpp
    LevelPack/EnemyPhase.cpp
    LevelPack/EnemyPhaseAction.cpp
    LevelPack/EnemyPhaseStartCondition.cpp
    LevelPack/EnemySpawn.cpp
    LevelPack/EntityAnimatableSet.cpp
    LevelPack/Item.cpp
    LevelPack/Level.cpp
    LevelPack/LevelEvent.cpp
    LevelPack/LevelEventStartCondition.cpp
    LevelPack/LevelPack.cpp
    LevelPack/LevelPackObject.cpp
    LevelPack/Player.cpp
    LevelPack/TextMarshallable.cpp
    Util/MathUtils.cpp
    Util/StringUtils.cpp
    Util/TextFileParser.cpp
)

add_executable(BHM ${BHM_SRC})

target_compile_options(BHM PRIVATE /bigobj)

# Add <project>/include as public include directory
include_directories($<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/BulletHellMaker/include>)

# Include SFMl, TGUI, entt, and Python
target_include_directories(BHM PRIVATE ${SFML_ROOT}/include)
target_include_directories(BHM PRIVATE ${TGUI_ROOT}/include)
target_include_directories(BHM PRIVATE ${ENTT_ROOT}/src)
target_include_directories(BHM PRIVATE ${PYTHON_ROOT}/include)
target_include_directories(BHM PRIVATE ${PYTHON_ROOT}/Lib/site-packages/numpy/core/include)

target_link_libraries(BHM ${PYTHON_ROOT}/Lib/site-packages/numpy/core/lib/npymath.lib)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(BHM ${PYTHON_ROOT}/libs/python27_d.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-window-d.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-system-d.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-main-d.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-graphics-d.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-audio-d.lib)
    target_link_libraries(BHM ${TGUI_BUILD_DIR}/lib/Debug/tgui-d.lib)
    
    # Add .lib for test to link to later
    if(BUILD_TESTS)
        add_library(BHM_lib ${BHM_SRC})
        target_compile_options(BHM_lib PRIVATE /bigobj)
        target_include_directories(BHM_lib PRIVATE ${SFML_ROOT}/include)
        target_include_directories(BHM_lib PRIVATE ${TGUI_ROOT}/include)
        target_include_directories(BHM_lib PRIVATE ${ENTT_ROOT}/src)
        target_include_directories(BHM_lib PRIVATE ${PYTHON_ROOT}/include)
        target_include_directories(BHM_lib PRIVATE ${PYTHON_ROOT}/Lib/site-packages/numpy/core/include)
        target_link_libraries(BHM_lib ${PYTHON_ROOT}/Lib/site-packages/numpy/core/lib/npymath.lib)
        target_link_libraries(BHM_lib ${PYTHON_ROOT}/libs/python27_d.lib)
        target_link_libraries(BHM_lib ${SFML_ROOT}/lib/sfml-window-d.lib)
        target_link_libraries(BHM_lib ${SFML_ROOT}/lib/sfml-system-d.lib)
        target_link_libraries(BHM_lib ${SFML_ROOT}/lib/sfml-main-d.lib)
        target_link_libraries(BHM_lib ${SFML_ROOT}/lib/sfml-graphics-d.lib)
        target_link_libraries(BHM_lib ${SFML_ROOT}/lib/sfml-audio-d.lib)
        target_link_libraries(BHM_lib ${TGUI_BUILD_DIR}/lib/Debug/tgui-d.lib)
    endif()
else()
    target_link_libraries(BHM ${PYTHON_ROOT}/libs/python27.lib)

    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-window.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-system.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-main.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-graphics.lib)
    target_link_libraries(BHM ${SFML_ROOT}/lib/sfml-audio.lib)
    
    target_link_libraries(BHM ${TGUI_BUILD_DIR}/lib/Release/tgui.lib)
endif()