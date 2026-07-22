#include "MathUtility.h"
#include <cmath>

using namespace KamataEngine;

Vector3 Add(const Vector3& v1, const Vector3& v2) 
{
	return
	{
		v1.x + v2.x, v1.y + v2.y, v1.z + v2.z
    }; 
}

Vector3 Subtract(const Vector3& v1, const Vector3& v2) 
{ 
	return
	{
		v1.x - v2.x, v1.y - v2.y, v1.z - v2.z
	};
}

Vector3 Multiply(float scalar, const Vector3& vector)
{
	return
	{
		vector.x * scalar, vector.y * scalar, vector.z * scalar
	};
}

float Length(const Vector3& vector) 
{ 
	return std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

Vector3 Normalize(const Vector3& vector)
{
	const float length = Length(vector);

	if (length == 0.0f) 
	{
		return {};
	}

	return {vector.x / length, vector.y / length, vector.z / length};
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) 
{
	const float sinX = std::sin(rotate.x);
	const float cosX = std::cos(rotate.x);
	const float sinY = std::sin(rotate.y);
	const float cosY = std::cos(rotate.y);
	const float sinZ = std::sin(rotate.z);
	const float cosZ = std::cos(rotate.z);

	Matrix4x4 result{};

	result.m[0][0] = scale.x * (cosY * cosZ);
	result.m[0][1] = scale.x * (cosY * sinZ);
	result.m[0][2] = scale.x * (-sinY);
	result.m[0][3] = 0.0f;

	result.m[1][0] = scale.y * (sinX * sinY * cosZ - cosX * sinZ);
	result.m[1][1] = scale.y * (sinX * sinY * sinZ + cosX * cosZ);
	result.m[1][2] = scale.y * (sinX * cosY);
	result.m[1][3] = 0.0f;

	result.m[2][0] = scale.z * (cosX * sinY * cosZ + sinX * sinZ);
	result.m[2][1] = scale.z * (cosX * sinY * sinZ - sinX * cosZ);
	result.m[2][2] = scale.z * (cosX * cosY);
	result.m[2][3] = 0.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}