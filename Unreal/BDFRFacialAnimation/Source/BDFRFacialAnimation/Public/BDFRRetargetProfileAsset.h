#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BDFRRetargetProfileAsset.generated.h"

USTRUCT(BlueprintType)
struct FBDFRRetargetMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR")
    FName SourceCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR")
    FName TargetMorph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR")
    float Bias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR")
    float MinValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR")
    float MaxValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR")
    float DeadZone = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR")
    bool bInvert = false;
};

UCLASS(BlueprintType)
class BDFRFACIALANIMATION_API UBDFRRetargetProfileAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BDFR")
    TArray<FBDFRRetargetMapping> Mappings;

    UFUNCTION(BlueprintPure, Category="BDFR|Retarget")
    float EvaluateMapping(const FBDFRRetargetMapping& Mapping, float SourceValue) const;
};
