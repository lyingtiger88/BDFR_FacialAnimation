#include "BDFRFacialComponent.h"

UBDFRFacialComponent::UBDFRFacialComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UBDFRFacialComponent::SetCurveValue(FName CurveName, float Value)
{
    if (CurveName.IsNone())
    {
        return;
    }

    CurrentCurves.FindOrAdd(CurveName) =
        bClampNormalizedCurves ? FMath::Clamp(Value, 0.0f, 1.0f) : Value;
}

void UBDFRFacialComponent::SetCurveValues(const TMap<FName, float>& Values)
{
    for (const TPair<FName, float>& Pair : Values)
    {
        SetCurveValue(Pair.Key, Pair.Value);
    }
}

float UBDFRFacialComponent::GetCurveValue(FName CurveName) const
{
    if (const float* Value = CurrentCurves.Find(CurveName))
    {
        return *Value;
    }
    return 0.0f;
}

void UBDFRFacialComponent::ClearCurves()
{
    CurrentCurves.Reset();
}
