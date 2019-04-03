#pragma once

#define AB_KEY_NONE 0
#define AB_KEY_7KEYS_1P_1 1
#define AB_KEY_7KEYS_1P_2 2
#define AB_KEY_7KEYS_1P_3 3
#define AB_KEY_7KEYS_1P_4 4
#define AB_KEY_7KEYS_1P_5 5
#define AB_KEY_7KEYS_1P_6 6
#define AB_KEY_7KEYS_1P_7 7
#define AB_KEY_7KEYS_1P_SL 8
#define AB_KEY_7KEYS_1P_SR 9
#define AB_KEY_7KEYS_1P_START 10
#define AB_KEY_7KEYS_1P_SELECT 11
#define AB_KEY_7KEYS_2P_1 12
#define AB_KEY_7KEYS_2P_2 13
#define AB_KEY_7KEYS_2P_3 14
#define AB_KEY_7KEYS_2P_4 15
#define AB_KEY_7KEYS_2P_5 16
#define AB_KEY_7KEYS_2P_6 17
#define AB_KEY_7KEYS_2P_7 18
#define AB_KEY_7KEYS_2P_SL 19
#define AB_KEY_7KEYS_2P_SR 20
#define AB_KEY_7KEYS_2P_START 21
#define AB_KEY_7KEYS_2P_SELECT 22
#define AB_KEY_9BUTTONS_1 23
#define AB_KEY_9BUTTONS_2 24
#define AB_KEY_9BUTTONS_3 25
#define AB_KEY_9BUTTONS_4 26
#define AB_KEY_9BUTTONS_5 27
#define AB_KEY_9BUTTONS_6 28
#define AB_KEY_9BUTTONS_7 29
#define AB_KEY_9BUTTONS_8 30
#define AB_KEY_9BUTTONS_9 31
#define AB_KEY_9BUTTONS_START 32
#define AB_KEY_9BUTTONS_SELECT 33
#define AB_KEY_5KEYS_1P_1 34
#define AB_KEY_5KEYS_1P_2 35
#define AB_KEY_5KEYS_1P_3 36
#define AB_KEY_5KEYS_1P_4 37
#define AB_KEY_5KEYS_1P_5 38
#define AB_KEY_5KEYS_1P_SL 39
#define AB_KEY_5KEYS_1P_SR 40
#define AB_KEY_5KEYS_1P_START 41
#define AB_KEY_5KEYS_1P_SELECT 42
#define AB_KEY_5KEYS_2P_1 43
#define AB_KEY_5KEYS_2P_2 44
#define AB_KEY_5KEYS_2P_3 45
#define AB_KEY_5KEYS_2P_4 46
#define AB_KEY_5KEYS_2P_5 47
#define AB_KEY_5KEYS_2P_SL 48
#define AB_KEY_5KEYS_2P_SR 49
#define AB_KEY_5KEYS_2P_START 50
#define AB_KEY_5KEYS_2P_SELECT 51

#define AB_KEY_COUNT 52
#define AB_KEY_7KEYS_COUNT 22
#define AB_KEY_9BUTTONS_COUNT 11
#define AB_KEY_5KEYS_COUNT 18

#define AB_INPUT_DOWN 0
#define AB_INPUT_LEFT 1
#define AB_INPUT_RIGHT 2
#define AB_INPUT_UP 3
#define AB_INPUT_1 4
#define AB_INPUT_2 5
#define AB_INPUT_3 6
#define AB_INPUT_4 7
#define AB_INPUT_5 8
#define AB_INPUT_6 9
#define AB_INPUT_7 10
#define AB_INPUT_8 11
#define AB_INPUT_9 12
#define AB_INPUT_10 13
#define AB_INPUT_11	14
#define AB_INPUT_12	15
#define AB_INPUT_13	16
#define AB_INPUT_14	17
#define AB_INPUT_15	18
#define AB_INPUT_16	19
#define AB_INPUT_17	20
#define AB_INPUT_18	21
#define AB_INPUT_19	22
#define AB_INPUT_20	23
#define AB_INPUT_21	24
#define AB_INPUT_22	25
#define AB_INPUT_23	26
#define AB_INPUT_24	27
#define AB_INPUT_25	28
#define AB_INPUT_26	29
#define AB_INPUT_27	30
#define AB_INPUT_28	31

#define AB_KEY_CONFIG_PATH L"ABFiles/Settings/KeyConfig"

class KeyConfig
{
public:
	enum KeyType : char
	{
		None,
		Keyboard,
		JoyPad1,
		JoyPad2,
		JoyPad3,
		JoyPad4
	};

	struct Key
	{
		KeyType type = KeyType::None;
		int id = 0;

		static Key make(KeyType t = KeyType::None, int i = 0)
		{
			Key k;
			k.type = t;
			k.id = i;
			return k;
		}
	};

private:
	std::array<std::array<Key, 10>, AB_KEY_COUNT> _keys;

public:
	KeyConfig() { reset(); }
	virtual ~KeyConfig() { }

	static bool load(KeyConfig& config, std::wstring path = AB_KEY_CONFIG_PATH);
	bool save(std::wstring path = AB_KEY_CONFIG_PATH);
	void clear();
	void reset();
	static std::wstring name(Key key);

	auto& keys() { return _keys; }
};
