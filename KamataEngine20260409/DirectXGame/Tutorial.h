#pragma once
#include "KamataEngine.h"

class Tutorial 
{
public:
	void Initialize(KamataEngine::Model* plateModel, KamataEngine::Model* tutorialModel, const KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	void Update();
	void Draw();

private:
	KamataEngine::Model* plateModel_ = nullptr;
	KamataEngine::Model* tutorialModel_ = nullptr;

	const KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::WorldTransform plateTransform_;
	KamataEngine::WorldTransform tutorialTransform_;
};