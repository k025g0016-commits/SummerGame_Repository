#include "Fade.h"
#include <algorithm>

void Fade::Initialize()
{
	sprite_ = Sprite::Create(TextureManager::Load("white1x1.png"), {0.0f, 0.0f});

	sprite_->SetSize(Vector2(1280.0f, 720.0f));

	// 最初は黒
	color_ = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	sprite_->SetColor(color_);
}

void Fade::Start(Status status, float duration) 
{
	// 色を指定しなかった場合は黒
	Start(status, duration, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Fade::Start(Status status, float duration, const Vector4& color) 
{
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;

	color_ = color;
}

void Fade::Stop()
{ 
	status_ = Status::None;
}

void Fade::Update() 
{
	switch (status_) 
	{
	case Status::None:
		break;

	case Status::FadeIn:
	{
		counter_ += 1.0f / 60.0f;

		if (counter_ >= duration_)
		{
			counter_ = duration_;
			status_ = Status::None;
		}

		const float alpha = std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f);

		sprite_->SetColor(Vector4(color_.x, color_.y, color_.z, alpha));

		break;
	}

	case Status::FadeOut:
	{
		counter_ += 1.0f / 60.0f;

		if (counter_ >= duration_) 
		{
			counter_ = duration_;
		}

		const float alpha = std::clamp(counter_ / duration_, 0.0f, 1.0f);

		sprite_->SetColor(Vector4(color_.x, color_.y, color_.z, alpha));

		break;
	}
	}
}

void Fade::Draw() 
{
	if (status_ == Status::None) 
	{
		return;
	}

	Sprite::PreDraw();

	sprite_->Draw();

	Sprite::PostDraw();
}

bool Fade::IsFinished() const
{
	switch (status_)
	{
	case Status::FadeIn:
	case Status::FadeOut:

		return counter_ >= duration_;
	}

	return true;
}