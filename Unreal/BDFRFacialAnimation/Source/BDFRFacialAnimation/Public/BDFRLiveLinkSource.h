#pragma once

#include "CoreMinimal.h"
#include "ILiveLinkSource.h"
#include "LiveLinkTypes.h"

class ILiveLinkClient;

struct FBDFRLiveLinkCurveFrame
{
    double TimestampSeconds = 0.0;
    TMap<FName, float> Curves;
};

class BDFRFACIALANIMATION_API FBDFRLiveLinkSource final
    : public ILiveLinkSource
    , public TSharedFromThis<FBDFRLiveLinkSource>
{
public:
    explicit FBDFRLiveLinkSource(
        FName InSubjectName = TEXT("BDFR_Face"));

    virtual ~FBDFRLiveLinkSource() override;

    virtual void ReceiveClient(
        ILiveLinkClient* InClient,
        FGuid InSourceGuid) override;

    virtual bool IsSourceStillValid() const override;
    virtual bool RequestSourceShutdown() override;

    virtual FText GetSourceType() const override;
    virtual FText GetSourceMachineName() const override;
    virtual FText GetSourceStatus() const override;

    virtual void Update() override;

    void EnqueueFrame(FBDFRLiveLinkCurveFrame InFrame);

    FName GetSubjectName() const { return SubjectName; }

private:
    void EnsureStaticData(const TArray<FName>& CurveNames);
    void PushFrame(const FBDFRLiveLinkCurveFrame& Frame);

    ILiveLinkClient* Client = nullptr;
    FGuid SourceGuid;
    FName SubjectName;

    mutable FCriticalSection QueueMutex;
    TArray<FBDFRLiveLinkCurveFrame> PendingFrames;

    TArray<FName> CurrentCurveNames;

    FThreadSafeBool bRunning = true;
    FThreadSafeBool bClientReady = false;
};
