#include "stdafx.h"
#include "KeyConfig.h"

bool KeyConfig::load(KeyConfig& config, std::wstring path)
{
	std::ifstream ifs(path, std::ios::in | std::ios::binary);
	if (ifs.fail()) return false;
	for (auto& a : config._keys)
	{
		for (auto& b : a)
		{
			ifs.read((char *)&b.type, sizeof(decltype(b.type)));
			ifs.read((char *)&b.id, sizeof(decltype(b.id)));
		}
	}
	return true;
}

bool KeyConfig::save(std::wstring path)
{
	std::ofstream ofs(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (ofs.fail()) return false;
	for (auto& a : _keys)
	{
		for (auto& b : a)
		{
			ofs.write((char *)&b.type, sizeof(decltype(b.type)));
			ofs.write((char *)&b.id, sizeof(decltype(b.id)));
		}
	}
	return true;
}

void KeyConfig::clear()
{
	for (auto& a : _keys)
	{
		for (auto& b : a)
		{
			b = Key::make();
		}
	}
}

void KeyConfig::reset()
{
	clear();

	// Keyboards
	_keys[AB_KEY_7KEYS_1P_1][0] = Key::make(KeyType::Keyboard, KEY_INPUT_Z);
	_keys[AB_KEY_7KEYS_1P_2][0] = Key::make(KeyType::Keyboard, KEY_INPUT_S);
	_keys[AB_KEY_7KEYS_1P_3][0] = Key::make(KeyType::Keyboard, KEY_INPUT_X);
	_keys[AB_KEY_7KEYS_1P_4][0] = Key::make(KeyType::Keyboard, KEY_INPUT_D);
	_keys[AB_KEY_7KEYS_1P_5][0] = Key::make(KeyType::Keyboard, KEY_INPUT_C);
	_keys[AB_KEY_7KEYS_1P_6][0] = Key::make(KeyType::Keyboard, KEY_INPUT_F);
	_keys[AB_KEY_7KEYS_1P_7][0] = Key::make(KeyType::Keyboard, KEY_INPUT_V);
	_keys[AB_KEY_7KEYS_1P_SL][0] = Key::make(KeyType::Keyboard, KEY_INPUT_LCONTROL);
	_keys[AB_KEY_7KEYS_1P_SR][0] = Key::make(KeyType::Keyboard, KEY_INPUT_LSHIFT);
	_keys[AB_KEY_7KEYS_1P_START][0] = Key::make(KeyType::Keyboard, KEY_INPUT_Q);
	_keys[AB_KEY_7KEYS_1P_SELECT][0] = Key::make(KeyType::Keyboard, KEY_INPUT_W);
	_keys[AB_KEY_7KEYS_2P_1][0] = Key::make(KeyType::Keyboard, KEY_INPUT_COMMA);
	_keys[AB_KEY_7KEYS_2P_2][0] = Key::make(KeyType::Keyboard, KEY_INPUT_L);
	_keys[AB_KEY_7KEYS_2P_3][0] = Key::make(KeyType::Keyboard, KEY_INPUT_PERIOD);
	_keys[AB_KEY_7KEYS_2P_4][0] = Key::make(KeyType::Keyboard, KEY_INPUT_SEMICOLON);
	_keys[AB_KEY_7KEYS_2P_5][0] = Key::make(KeyType::Keyboard, KEY_INPUT_SLASH);
	_keys[AB_KEY_7KEYS_2P_6][0] = Key::make(KeyType::Keyboard, KEY_INPUT_COLON);
	_keys[AB_KEY_7KEYS_2P_7][0] = Key::make(KeyType::Keyboard, KEY_INPUT_BACKSLASH);
	_keys[AB_KEY_7KEYS_2P_SL][0] = Key::make(KeyType::Keyboard, KEY_INPUT_RCONTROL);
	_keys[AB_KEY_7KEYS_2P_SR][0] = Key::make(KeyType::Keyboard, KEY_INPUT_RSHIFT);
	_keys[AB_KEY_7KEYS_2P_START][0] = Key::make(KeyType::Keyboard, KEY_INPUT_LBRACKET);
	_keys[AB_KEY_7KEYS_2P_SELECT][0] = Key::make(KeyType::Keyboard, KEY_INPUT_AT);
}

std::wstring KeyConfig::name(Key key)
{
	if (key.type == KeyType::None) return L"-";
	if (key.type == KeyType::Keyboard)
	{
		if (key.id == KEY_INPUT_BACK) return L"Backspace";
		if (key.id == KEY_INPUT_TAB) return L"Tab";
		if (key.id == KEY_INPUT_RETURN) return L"Enter";

		if (key.id == KEY_INPUT_LSHIFT) return L"Left Shift";
		if (key.id == KEY_INPUT_RSHIFT) return L"Right Shift";
		if (key.id == KEY_INPUT_LCONTROL) return L"Left Ctrl";
		if (key.id == KEY_INPUT_RCONTROL) return L"Right Ctrl";
		if (key.id == KEY_INPUT_ESCAPE) return L"Escape";
		if (key.id == KEY_INPUT_SPACE) return L"Space";
		if (key.id == KEY_INPUT_PGUP) return L"Page Up";
		if (key.id == KEY_INPUT_PGDN) return L"Page Down";
		if (key.id == KEY_INPUT_END) return L"End";
		if (key.id == KEY_INPUT_HOME) return L"Home";
		if (key.id == KEY_INPUT_LEFT) return L"Left";
		if (key.id == KEY_INPUT_UP) return L"Up";
		if (key.id == KEY_INPUT_RIGHT) return L"Right";
		if (key.id == KEY_INPUT_DOWN) return L"Down";
		if (key.id == KEY_INPUT_INSERT) return L"Insert";
		if (key.id == KEY_INPUT_DELETE) return L"Delete";

		if (key.id == KEY_INPUT_MINUS) return L"Minus";
		if (key.id == KEY_INPUT_YEN) return L"Yen";
		if (key.id == KEY_INPUT_PREVTRACK) return L"Caret";
		if (key.id == KEY_INPUT_PERIOD) return L"Period";
		if (key.id == KEY_INPUT_SLASH) return L"Slash";
		if (key.id == KEY_INPUT_LALT) return L"Left Alt";
		if (key.id == KEY_INPUT_RALT) return L"Right Alt";
		if (key.id == KEY_INPUT_SCROLL) return L"Scroll Lock";
		if (key.id == KEY_INPUT_SEMICOLON) return L"Semicolon";
		if (key.id == KEY_INPUT_COLON) return L"Colon";
		if (key.id == KEY_INPUT_LBRACKET) return L"Left Bracket";
		if (key.id == KEY_INPUT_RBRACKET) return L"Right Bracket";
		if (key.id == KEY_INPUT_AT) return L"At";
		if (key.id == KEY_INPUT_BACKSLASH) return L"Backslash";
		if (key.id == KEY_INPUT_COMMA) return L"Comma";
		if (key.id == KEY_INPUT_KANJI) return L"Kanji";
		if (key.id == KEY_INPUT_CONVERT) return L"Conversion";
		if (key.id == KEY_INPUT_NOCONVERT) return L"Non Conversion";
		if (key.id == KEY_INPUT_KANA) return L"Kana";
		if (key.id == KEY_INPUT_APPS) return L"Menu";
		if (key.id == KEY_INPUT_CAPSLOCK) return L"Caps Lock";
		if (key.id == KEY_INPUT_SYSRQ) return L"System Request";
		if (key.id == KEY_INPUT_PAUSE) return L"Pause";
		if (key.id == KEY_INPUT_LWIN) return L"Left Windows";
		if (key.id == KEY_INPUT_RWIN) return L"Right Windows";

		if (key.id == KEY_INPUT_NUMLOCK) return L"Num Lock";
		if (key.id == KEY_INPUT_NUMPAD0) return L"Numpad 0";
		if (key.id == KEY_INPUT_NUMPAD1) return L"Numpad 1";
		if (key.id == KEY_INPUT_NUMPAD2) return L"Numpad 2";
		if (key.id == KEY_INPUT_NUMPAD3) return L"Numpad 3";
		if (key.id == KEY_INPUT_NUMPAD4) return L"Numpad 4";
		if (key.id == KEY_INPUT_NUMPAD5) return L"Numpad 5";
		if (key.id == KEY_INPUT_NUMPAD6) return L"Numpad 6";
		if (key.id == KEY_INPUT_NUMPAD7) return L"Numpad 7";
		if (key.id == KEY_INPUT_NUMPAD8) return L"Numpad 8";
		if (key.id == KEY_INPUT_NUMPAD9) return L"Numpad 9";
		if (key.id == KEY_INPUT_MULTIPLY) return L"Numpad Asterisk";
		if (key.id == KEY_INPUT_ADD) return L"Numpad Plus";
		if (key.id == KEY_INPUT_SUBTRACT) return L"Numpad Minus";
		if (key.id == KEY_INPUT_DECIMAL) return L"Numpad Period";
		if (key.id == KEY_INPUT_DIVIDE) return L"Numpad Slash";
		if (key.id == KEY_INPUT_NUMPADENTER) return L"Numpad Enter";

		if (key.id == KEY_INPUT_F1) return L"F1";
		if (key.id == KEY_INPUT_F2) return L"F2";
		if (key.id == KEY_INPUT_F3) return L"F3";
		if (key.id == KEY_INPUT_F4) return L"F4";
		if (key.id == KEY_INPUT_F5) return L"F5";
		if (key.id == KEY_INPUT_F6) return L"F6";
		if (key.id == KEY_INPUT_F7) return L"F7";
		if (key.id == KEY_INPUT_F8) return L"F8";
		if (key.id == KEY_INPUT_F9) return L"F9";
		if (key.id == KEY_INPUT_F10) return L"F10";
		if (key.id == KEY_INPUT_F11) return L"F11";
		if (key.id == KEY_INPUT_F12) return L"F12";

		if (key.id == KEY_INPUT_A) return L"A";
		if (key.id == KEY_INPUT_B) return L"B";
		if (key.id == KEY_INPUT_C) return L"C";
		if (key.id == KEY_INPUT_D) return L"D";
		if (key.id == KEY_INPUT_E) return L"E";
		if (key.id == KEY_INPUT_F) return L"F";
		if (key.id == KEY_INPUT_G) return L"G";
		if (key.id == KEY_INPUT_H) return L"H";
		if (key.id == KEY_INPUT_I) return L"I";
		if (key.id == KEY_INPUT_J) return L"J";
		if (key.id == KEY_INPUT_K) return L"K";
		if (key.id == KEY_INPUT_L) return L"L";
		if (key.id == KEY_INPUT_M) return L"M";
		if (key.id == KEY_INPUT_N) return L"N";
		if (key.id == KEY_INPUT_O) return L"O";
		if (key.id == KEY_INPUT_P) return L"P";
		if (key.id == KEY_INPUT_Q) return L"Q";
		if (key.id == KEY_INPUT_R) return L"R";
		if (key.id == KEY_INPUT_S) return L"S";
		if (key.id == KEY_INPUT_T) return L"T";
		if (key.id == KEY_INPUT_U) return L"U";
		if (key.id == KEY_INPUT_V) return L"V";
		if (key.id == KEY_INPUT_W) return L"W";
		if (key.id == KEY_INPUT_X) return L"X";
		if (key.id == KEY_INPUT_Y) return L"Y";
		if (key.id == KEY_INPUT_Z) return L"Z";

		if (key.id == KEY_INPUT_0) return L"0";
		if (key.id == KEY_INPUT_1) return L"1";
		if (key.id == KEY_INPUT_2) return L"2";
		if (key.id == KEY_INPUT_3) return L"3";
		if (key.id == KEY_INPUT_4) return L"4";
		if (key.id == KEY_INPUT_5) return L"5";
		if (key.id == KEY_INPUT_6) return L"6";
		if (key.id == KEY_INPUT_7) return L"7";
		if (key.id == KEY_INPUT_8) return L"8";
		if (key.id == KEY_INPUT_9) return L"9";
	}
	else if (key.type == KeyType::JoyPad1 || key.type == KeyType::JoyPad2 || key.type == KeyType::JoyPad3 || key.type == KeyType::JoyPad4)
	{
		int n = key.type - KeyType::JoyPad1 + 1;
		std::wstring prefix(L"P");
		prefix += std::to_wstring(n) + L": ";

		if (key.id == AB_INPUT_DOWN) return prefix + L"Down";
		if (key.id == AB_INPUT_LEFT) return prefix + L"Left";
		if (key.id == AB_INPUT_RIGHT) return prefix + L"Right";
		if (key.id == AB_INPUT_UP) return prefix + L"Up";
		if (key.id == AB_INPUT_1) return prefix + L"Button 1";
		if (key.id == AB_INPUT_2) return prefix + L"Button 2";
		if (key.id == AB_INPUT_3) return prefix + L"Button 3";
		if (key.id == AB_INPUT_4) return prefix + L"Button 4";
		if (key.id == AB_INPUT_5) return prefix + L"Button 5";
		if (key.id == AB_INPUT_6) return prefix + L"Button 6";
		if (key.id == AB_INPUT_7) return prefix + L"Button 7";
		if (key.id == AB_INPUT_8) return prefix + L"Button 8";
		if (key.id == AB_INPUT_9) return prefix + L"Button 9";
		if (key.id == AB_INPUT_10) return prefix + L"Button 10";
		if (key.id == AB_INPUT_11) return prefix + L"Button 11";
		if (key.id == AB_INPUT_12) return prefix + L"Button 12";
		if (key.id == AB_INPUT_13) return prefix + L"Button 13";
		if (key.id == AB_INPUT_14) return prefix + L"Button 14";
		if (key.id == AB_INPUT_15) return prefix + L"Button 15";
		if (key.id == AB_INPUT_16) return prefix + L"Button 16";
		if (key.id == AB_INPUT_17) return prefix + L"Button 17";
		if (key.id == AB_INPUT_18) return prefix + L"Button 18";
		if (key.id == AB_INPUT_19) return prefix + L"Button 19";
		if (key.id == AB_INPUT_20) return prefix + L"Button 20";
		if (key.id == AB_INPUT_21) return prefix + L"Button 21";
	}

	return L"?";
}
