#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Queue.h"
#include "BDFRLiveReceiverComponent.generated.h"

class FBDFRLiveLinkSource;
class FSocket;
class FRunnableThread;
class UBDFRFacialComponent;
class ILiveLinkClient;

USTRUCT(BlueprintType)
struct FBDFRLiveReceiverStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="BDFR|Live")
    int64 PacketsReceived = 0;

    UPROPERTY(BlueprintReadOnly, Category="BDFR|Live")
    int64 DecodeFailures = 0;

    UPROPERTY(BlueprintReadOnly, Category="BDFR|Live")
    int64 SequenceGaps = 0;

    UPROPERTY(BlueprintReadOnly, Category="BDFR|Live")
    int64 LastSequence = 0;

    UPROPERTY(BlueprintReadOnly, Category="BDFR|Live")
    int32 LastCurveCount = 0;
};

struct FBDFRReceivedCurveFrame
{
    uint64 Sequence = 0;
    FString SourceId;
    double TimestampSeconds = 0.0;
    float Confidence = 0.0f;
    FVector3f HeadEuler = FVector3f::ZeroVector;
    FVector3f Gaze = FVector3f::ZeroVector;
    TMap<FName, float> Curves;
};

UCLASS(ClassGroup=(BDFR), meta=(BlueprintSpawnableComponent))
class BDFRFACIALANIMATION_API UBDFRLiveReceiverComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBDFRLiveReceiverComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="BDFR|Live")
    bool StartReceiver();

    UFUNCTION(BlueprintCallable, Category="BDFR|Live")
    void StopReceiver();

    UFUNCTION(BlueprintPure, Category="BDFR|Live")
    bool IsReceiving() const;

    UFUNCTION(BlueprintPure, Category="BDFR|Live")
    FBDFRLiveReceiverStats GetStats() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR|Live")
    int32 ListenPort = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR|LiveLink")
    bool bPublishLiveLink = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR|LiveLink")
    FName LiveLinkSubjectName = TEXT("BDFR_Face");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR|Runtime")
    bool bApplyToFacialComponent = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BDFR|Runtime")
    bool bAutoStart = true;

private:
    class FReceiverWorker;

    void DrainFrames();
    void PublishLiveLink(const FBDFRReceivedCurveFrame& Frame);
    void ApplyToOwner(const FBDFRReceivedCurveFrame& Frame);
    bool RegisterLiveLinkSource();
    void UnregisterLiveLinkSource();

    FSocket* Socket = nullptr;
    TUniquePtr<FReceiverWorker> Worker;
    FRunnableThread* WorkerThread = nullptr;

    TQueue<FBDFRReceivedCurveFrame, EQueueMode::Mpsc> ReceivedFrames;

    TSharedPtr<FBDFRLiveLinkSource> LiveLinkSource;
    ILiveLinkClient* LiveLinkClient = nullptr;

    TWeakObjectPtr<UBDFRFacialComponent> CachedFacialComponent;

    mutable FCriticalSection StatsMutex;
    FBDFRLiveReceiverStats Stats;
};
