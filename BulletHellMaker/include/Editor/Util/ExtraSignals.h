#pragma once
#include <type_traits>
#include <functional>
#include <typeindex>
#include <memory>
#include <vector>
#include <deque>
#include <utility>
#include <map>
#include <memory>

#include <TGUI/Global.hpp>
#include <TGUI/Animation.hpp>
#include <TGUI/Signal.hpp>
#include <SFML/System/String.hpp>

#include <DataStructs/SpriteLoader.h>
#include <LevelPack/Attack.h>
#include <Game/AudioPlayer.h>
#include <DataStructs/TimeFunctionVariable.h>
#include <LevelPack/EditorMovablePoint.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <DataStructs/SymbolTable.h>
#include <LevelPack/LevelPackObject.h>

namespace tgui {
	using SignalEditorAttack = SignalTyped<std::shared_ptr<EditorAttack>>;
	using SignalEditorMovablePoint = SignalTyped<std::shared_ptr<EditorMovablePoint>>;
	using SignalEditorAttackPattern = SignalTyped<std::shared_ptr<EditorAttackPattern>>;
	using SignalSoundSettings = SignalTyped<SoundSettings>;
	using SignalAnimatable = SignalTyped<Animatable>;
	using SignalEMPA = SignalTyped<std::shared_ptr<EMPAction>>;
	using SignalValueSymbolTable = SignalTyped<ValueSymbolTable>;
	using SignalExprSymbolTable = SignalTyped<ExprSymbolTable>;
	using SignalAttackPatternToAttackUseRelationship = SignalTyped<std::vector<std::tuple<std::string, int, ExprSymbolTable>>>;
	using SignalSpriteSheet = SignalTyped<std::shared_ptr<SpriteSheet>>;
	using SignalTwoTFVs = SignalTyped2<std::shared_ptr<TFV>, std::shared_ptr<TFV>>;
	using SignalTwoEMPAAngleOffsets = SignalTyped2<std::shared_ptr<EMPAAngleOffset>, std::shared_ptr<EMPAAngleOffset>>;
	using SignalTwoInts = SignalTyped2<int, int>;
	using SignalEMPAVectorAndFloat = SignalTyped2<std::vector<std::shared_ptr<EMPAction>>, float>;
	using SignalLevelPackObjectLegalResult = SignalTyped2<LevelPackObject::LEGAL_STATUS, std::vector<std::string>>;
}
