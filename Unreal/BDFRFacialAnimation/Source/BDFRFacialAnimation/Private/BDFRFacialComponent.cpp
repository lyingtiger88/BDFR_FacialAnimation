#include "BDFRFacialComponent.h"

#include "BDFRRetargetProfileAsset.h"
#include "Components/SkeletalMeshComponent.h"

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

void UBDFRFacialComponent::ApplyToSkeletalMesh(USkeletalMeshComponent* SkeletalMesh)
{
    if (!IsValid(SkeletalMesh))
    {
        return;
    }

    if (!IsValid(RetargetProfile))
    {
        for (const TPair<FName, float>& Pair : CurrentCurves)
        {
            SkeletalMesh->SetMorphTarget(Pair.Key, Pair.Value, false);
        }
        return;
    }

    for (const FBDFRRetargetMapping& Mapping : RetargetProfile->Mappings)
    {
        if (Mapping.SourceCurve.IsNone() || Mapping.TargetMorph.IsNone())
        {
            continue;
        }

        const float SourceValue = GetCurveValue(Mapping.SourceCurve);
        const float TargetValue =
            RetargetProfile->EvaluateMapping(Mapping, SourceValue);

        SkeletalMesh->SetMorphTarget(
            Mapping.TargetMorph,
            TargetValue,
            false);
    }
}
