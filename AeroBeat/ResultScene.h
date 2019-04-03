#pragma once

#include "SceneBase.h"
#include "SkinEngine.h"

class ResultScene : public SceneBase
{
public:
	struct ResultParameters
	{
		int difficulty, level, judge_rank;
		std::wstring title;
		long long total_notes;

		bool cleared;
		int dj_level;
		long long pgreat, great, good, bad, poor, cbrk, fast, slow, max_combo;
		double achievement;
	};

private:
	Skin& _skin;
	SkinEngine _engine;

public:
	ResultScene(Window* window, Skin& skin, ResultParameters params);
	virtual ~ResultScene() { }

	virtual bool update() override;
	virtual void draw() override;
};
