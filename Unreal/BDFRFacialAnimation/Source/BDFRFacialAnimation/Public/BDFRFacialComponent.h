#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BDFRFacialComponent.generated.h"

class UBDFRRetargetProfileAsset;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(BDFR), meta=(BlueprintSpawnableComponent))
class BDFRFACIALANIMATION_API UBDFRFacialComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBDFRFacialComponent();

    UFUNCTION(BlueprintCallable, Category="BDFR|Facial")
    void SetCurveValue(FName CurveName, float Value);

    UFUNCTION(BlueprintCallable, Category="BDFR|Facial")
    void SetCurveValues(const TMap<FName, float>& Values);

    UFUNCTION(BlueprintPure, Category="BDFR|Facial")
    float GetCurveValue(FName CurveName) const;

    UFUNCTION(BlueprintCallable, Category="BDFR|Facial")
    void ClearCurves();

    UFUNCTION(BlueprintCallable, Category="BDFR|Facial")
    void ApplyToSkeletalMesh(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintPure, Category="BDFR|Facial")
    const TMap<FName, float>& GetCurrentCurves() const { return CurrentCurves; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR|Facial")
    bool bClampNormalizedCurves = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR|Retarget")
    TObjectPtr<UBDFRRetargetProfileAsset> RetargetProfile;

private:
    UPROPERTY(Transient)
    TMap<FName, float> CurrentCurves;
};
