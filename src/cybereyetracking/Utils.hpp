#pragma once

namespace CyberEyeTracking
{
    class Math
    {
    public:
        static float GetAngle(float x, float y);
        static float GetLinearCurve(float a, float b, float x);
        /*static float GetParametrizedExpPow(float a, float b, float expPow, float x);
        static float GetParametrizedLn(float a, float b, float c, float d, float x);*/
        static float GetParametrizedParabola(float a, float b, float c, float x);
    };
}
