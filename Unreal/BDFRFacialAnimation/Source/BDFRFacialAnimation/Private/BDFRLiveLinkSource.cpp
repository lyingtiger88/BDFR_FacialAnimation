#include "BDFRLiveLinkSource.h"

#include "ILiveLinkClient.h"
#include "Roles/LiveLinkBasicRole.h"
#include "Roles/LiveLinkBasicTypes.h"

FBDFRLiveLinkSource::FBDFRLiveLinkSource(
    FName InSubjectName)
    : SubjectName(InSubjectName)
{
}

FBDFRLiveLinkSource::~FBDFRLiveLinkSource()
{
    RequestSourceShutdown();
}

void FBDFRLiveLinkSource::ReceiveClient(
    ILiveLinkClient* InClient,
    FGuid InSourceGuid)
{
    Client = InClient;
    SourceGuid = InSourceGuid;
    bClientReady = Client != nullptr && SourceGuid.IsValid();
}

bool FBDFRLiveLinkSource::IsSourceStillValid() const
{
    return bRunning && bClientReady && Client != nullptr;
}

bool FBDFRLiveLinkSource::RequestSourceShutdown()
{
    bRunning = false;
    bClientReady = false;

    {
        FScopeLock Lock(&QueueMutex);
        PendingFrames.Reset();
    }

    Client = nullptr;
    return true;
}

FText FBDFRLiveLinkSource::GetSourceType() const
{
    return NSLOCTEXT(
        "BDFR",
        "BDFRLiveLinkSourceType",
        "BDFR Facial Animation");
}

FText FBDFRLiveLinkSource::GetSourceMachineName() const
{
    return FText::FromString(FPlatformProcess::ComputerName());
}

FText FBDFRLiveLinkSource::GetSourceStatus() const
{
    return IsSourceStillValid()
        ? NSLOCTEXT("BDFR", "BDFRLiveLinkConnected", "Connected")
        : NSLOCTEXT("BDFR", "BDFRLiveLinkDisconnected", "Disconnected");
}

void FBDFRLiveLinkSource::EnqueueFrame(
    FBDFRLiveLinkCurveFrame InFrame)
{
    if (!bRunning)
    {
        return;
    }

    FScopeLock Lock(&QueueMutex);

    // Keep latency bounded. Live facial animation benefits more from the
    // newest frames than from replaying a growing stale backlog.
    constexpr int32 MaxPendingFrames = 4;

    if (PendingFrames.Num() >= MaxPendingFrames)
    {
        PendingFrames.RemoveAt(
            0,
            PendingFrames.Num() - MaxPendingFrames + 1,
            EAllowShrinking::No);
    }

    PendingFrames.Add(MoveTemp(InFrame));
}

void FBDFRLiveLinkSource::Update()
{
    if (!IsSourceStillValid())
    {
        return;
    }

    TArray<FBDFRLiveLinkCurveFrame> LocalFrames;

    {
        FScopeLock Lock(&QueueMutex);
        Swap(LocalFrames, PendingFrames);
    }

    // Update() is on Live Link's critical game-thread path.
    // Do no socket IO, waiting, inference, or heavy processing here.
    for (const FBDFRLiveLinkCurveFrame& Frame : LocalFrames)
    {
        PushFrame(Frame);
    }
}

void FBDFRLiveLinkSource::EnsureStaticData(
    const TArray<FName>& CurveNames)
{
    if (!Client || CurveNames == CurrentCurveNames)
    {
        return;
    }

    CurrentCurveNames = CurveNames;

    FLiveLinkStaticDataStruct StaticData(
        FLiveLinkBaseStaticData::StaticStruct());

    FLiveLinkBaseStaticData* BaseStaticData =
        StaticData.Cast<FLiveLinkBaseStaticData>();

    if (!BaseStaticData)
    {
        return;
    }

    BaseStaticData->PropertyNames = CurrentCurveNames;

    const FLiveLinkSubjectKey SubjectKey(
        SourceGuid,
        FLiveLinkSubjectName(SubjectName));

    Client->PushSubjectStaticData_AnyThread(
        SubjectKey,
        ULiveLinkBasicRole::StaticClass(),
        MoveTemp(StaticData));
}

void FBDFRLiveLinkSource::PushFrame(
    const FBDFRLiveLinkCurveFrame& Frame)
{
    if (!Client)
    {
        return;
    }

    TArray<FName> Names;
    Frame.Curves.GetKeys(Names);
    Names.Sort(FNameLexicalLess());

    EnsureStaticData(Names);

    if (CurrentCurveNames.IsEmpty())
    {
        return;
    }

    FLiveLinkFrameDataStruct FrameData(
        FLiveLinkBaseFrameData::StaticStruct());

    FLiveLinkBaseFrameData* BaseFrameData =
        FrameData.Cast<FLiveLinkBaseFrameData>();

    if (!BaseFrameData)
    {
        return;
    }

    BaseFrameData->PropertyValues.Reserve(
        CurrentCurveNames.Num());

    for (const FName CurveName : CurrentCurveNames)
    {
        const float* Value = Frame.Curves.Find(CurveName);

        BaseFrameData->PropertyValues.Add(
            Value
                ? FMath::Clamp(*Value, 0.0f, 1.0f)
                : 0.0f);
    }

    BaseFrameData->WorldTime =
        FLiveLinkWorldTime(Frame.TimestampSeconds);

    BaseFrameData->Timestamps.Add(
        TEXT("BDFR_SourceTime"),
        Frame.TimestampSeconds);

    const FLiveLinkSubjectKey SubjectKey(
        SourceGuid,
        FLiveLinkSubjectName(SubjectName));

    Client->PushSubjectFrameData_AnyThread(
        SubjectKey,
        MoveTemp(FrameData));
}
