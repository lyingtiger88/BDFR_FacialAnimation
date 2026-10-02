#include "BDFRRetargetProfileAsset.h"

float UBDFRRetargetProfileAsset::EvaluateMapping(
    const FBDFRRetargetMapping& Mapping,
    float SourceValue) const
{
    float Value = FMath::Clamp(SourceValue, 0.0f, 1.0f);

    if (Mapping.bInvert)
    {
        Value = 1.0f - Value;
    }

    if (FMath::Abs(Value) < FMath::Max(0.0f, Mapping.DeadZone))
    {
        Value = 0.0f;
    }

    Value = Value * Mapping.Scale + Mapping.Bias;
    return FMath::Clamp(Value, Mapping.MinValue, Mapping.MaxValue);
}
