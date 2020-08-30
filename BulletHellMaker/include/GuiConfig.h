#pragma once

// Height of any tgui::Widget that involves text, excluding buttons
const static float TEXT_BOX_HEIGHT = 16;
// Height of any button that involves text
const static float TEXT_BUTTON_HEIGHT = 16;
// Height of any slider
const static float SLIDER_HEIGHT = 15;
const static int TEXT_SIZE = 12;
const static float CHECKBOX_SIZE = 15;
const static float SMALL_BUTTON_SIZE = 20;
// Width of every tab in a tguI::Tabs button
const static float TAB_WIDTH = 100.0f;
// Padding around widgets, when applicable
const static float GUI_PADDING_X = 20;
const static float GUI_PADDING_Y = 10;
// Y padding inbetween labels and the widget they're explaining
const static float GUI_LABEL_PADDING_Y = 5;
// Scroll amount for every Panel
const static float SCROLL_AMOUNT = 20;
// Opacity of a disabled widget
const static float WIDGET_OPACITY_DISABLED = 0.6f;

const static float DIALOGUE_BOX_PADDING = 5;
// Height of dialogue box if there is no portrait
const static float DIALOGUE_BOX_HEIGHT = 200;

/*
CopyPaste IDs
*/
const static std::string ATTACK_PATTERN_TO_ATTACK_USE_RELATIONSHIP_COPY_PASTE_ID = "AttackPatternToAttackUseRelationship";