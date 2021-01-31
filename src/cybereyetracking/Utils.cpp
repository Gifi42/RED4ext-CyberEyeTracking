#include "stdafx.hpp"

#include <utility>
#include <cmath>

#include "Utils.hpp"

#define PI 3.1415926535
#define EXP 2.71828

float lengthSquare(std::pair<float, float> X, std::pair<float, float> Y)
{
    float xDiff = X.first - Y.first;
    float yDiff = X.second - Y.second;
    return xDiff * xDiff + yDiff * yDiff;
}

float CyberEyeTracking::Math::GetAngle(float x, float y)
{
    // Square of lengths be a2, b2, c2
    std::pair<float, float> A = std::make_pair(0.5, 0);
    std::pair<float, float> B = std::make_pair(0.5, 0.5);
    std::pair<float, float> C = std::make_pair(x, y);

    float a2 = lengthSquare(B, C);
    float b2 = lengthSquare(A, C);
    float c2 = lengthSquare(A, B);

    // length of sides be a, b, c
    float a = sqrt(a2);
    float c = sqrt(c2);

    // From Cosine law
    float betta = acos((a2 + c2 - b2) / (2 * a * c));

    // Converting to degree
    betta = betta * 180 / PI;
    if (x < 0.5)
        betta = 360 - betta;

    return betta;
}

float CyberEyeTracking::Math::GetLinearCurve(float a, float b, float x)
{
    return a - b * x;
}
//
//float CyberEyeTracking::Math::GetParametrizedExpPow(float a, float b, float expPow, float x)
//{
//    return a - b * pow(EXP, expPow*x);
//}
//
//float CyberEyeTracking::Math::GetParametrizedLn(float a, float b, float c, float d, float x)
//{
//    return a * log(b + c*x) + d;
//}


float CyberEyeTracking::Math::GetParametrizedParabola(float a, float b, float c, float x)
{
    return a * pow(x,2) + b*x + c;
}
