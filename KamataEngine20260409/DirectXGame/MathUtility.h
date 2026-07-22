#pragma once
#include "KamataEngine.h"

// ベクトルの加算
KamataEngine::Vector3 Add(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

// ベクトルの減算
KamataEngine::Vector3 Subtract(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

// ベクトルとスカラーの乗算
KamataEngine::Vector3 Multiply(float scalar, const KamataEngine::Vector3& vector);

// ベクトルの長さ
float Length(const KamataEngine::Vector3& vector);

// ベクトルの正規化
KamataEngine::Vector3 Normalize(const KamataEngine::Vector3& vector);

// アフィン変換行列
KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);