#pragma once
#include "EditorUtilities.h"
#include "LevelPack.h"
#include "Attack.h"
#include "AttackPattern.h"
#include "EditorMovablePoint.h"
#include "Enemy.h"
#include "EnemyPhase.h"
#include "Level.h"
#include "Player.h"
#include "SpriteLoader.h"
#include "Constants.h"
#include "SymbolTableEditor.h"

class EditorWindow;

/*
The LevelPack passed into this object's constructor will be cloned and should be synced with the
LevelPack being edited in the editor such that this object's LevelPack, at any time, contains
all unsaved changes in the editor's LevelPack.
*/
class LevelPackObjectPreviewPanel : public SimpleEngineRenderer {
public:
	LevelPackObjectPreviewPanel(EditorWindow& parentWindow, std::string levelPackName);
	~LevelPackObjectPreviewPanel();
	static std::shared_ptr<LevelPackObjectPreviewPanel> create(EditorWindow& parentWindow, std::string levelPackName) {
		return std::make_shared<LevelPackObjectPreviewPanel>(parentWindow, levelPackName);
	}

	/*
	The preview will use a clone of the attack.
	*/
	void previewAttack(const std::shared_ptr<EditorAttack> attack);

	bool handleEvent(sf::Event event) override;

	void setPreviewSource(float x, float y);
	void setAttackLoopDelay(float attackLoopDelay);
	void setAttackPatternLoopDelay(float attackPatternLoopDelay);
	
	float getPreviewSourceX() const;
	float getPreviewSourceY() const;
	float getAttackLoopDelay() const;
	float getAttackPatternLoopDelay() const;
	std::shared_ptr<LevelPack> getLevelPack();

private:
	enum class PREVIEW_OBJECT {
		NONE,
		ATTACK,
		ATTACK_PATTERN,
		ENEMY,
		ENEMY_PHASE
	};

	// Type of object currently being previewed
	PREVIEW_OBJECT currentPreviewObjectType = PREVIEW_OBJECT::NONE;
	int currentPreviewObjectID = -1;

	// Gui of the EditorWindow
	std::shared_ptr<tgui::Gui> gui;

	// Symbol table editor child window.
	// The window is added to the GUI directly and will be removed in this widget's destructor.
	std::shared_ptr<tgui::ChildWindow> symbolTableEditorWindow;
	// Symbol table editor
	std::shared_ptr<ValueSymbolTableEditor> symbolTableEditor;
	// Table containing variables for testing
	ValueSymbolTable testTable;

	// Spawn location of a previewed attack/attack pattern/enemy phase/enemy
	float previewSourceX = MAP_WIDTH / 2.0f;
	float previewSourceY = 2.0f * MAP_HEIGHT / 3.0f;
	// Seconds between attacks when previewing an attack
	float attackLoopDelay = 2.0f;
	// Seconds between attack patterns when previewing an attack pattern
	float attackPatternLoopDelay = 2.0f;

	std::shared_ptr<EditorPlayer> defaultPlayer;

	// Objects used to preview an attack
	std::shared_ptr<Level> levelForAttack;
	std::shared_ptr<EditorEnemyPhase> enemyPhaseForAttack;
	std::shared_ptr<EditorAttackPattern> attackPatternForAttack;

	/*
	Resets a preview to start back at time 0.
	*/
	void resetPreview();

	std::shared_ptr<EditorPlayer> getPlayer() override;
};