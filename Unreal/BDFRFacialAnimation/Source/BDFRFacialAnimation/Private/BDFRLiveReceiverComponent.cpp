#include "BDFRLiveReceiverComponent.h"

#include "BDFRFacialComponent.h"
#include "BDFRLiveLinkSource.h"

#include "Components/SkeletalMeshComponent.h"
#include "Common/UdpSocketBuilder.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "ILiveLinkClient.h"
#include "IModularFeatures.h"
#include "IPAddress.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

namespace
{

class FBDFRByteReader
{
public:
    FBDFRByteReader(const uint8* InData, int32 InSize)
        : Data(InData), Size(InSize)
    {
    }

    bool ReadU16(uint16& Out)
    {
        if (!Require(2)) return false;
        Out =
            static_cast<uint16>(Data[Offset]) |
            (static_cast<uint16>(Data[Offset + 1]) << 8);
        Offset += 2;
        return true;
    }

    bool ReadU32(uint32& Out)
    {
        if (!Require(4)) return false;
        Out =
            static_cast<uint32>(Data[Offset]) |
            (static_cast<uint32>(Data[Offset + 1]) << 8) |
            (static_cast<uint32>(Data[Offset + 2]) << 16) |
            (static_cast<uint32>(Data[Offset + 3]) << 24);
        Offset += 4;
        return true;
    }

    bool ReadU64(uint64& Out)
    {
        if (!Require(8)) return false;
        Out = 0;
        for (int32 Index = 0; Index < 8; ++Index)
        {
            Out |= static_cast<uint64>(Data[Offset + Index]) << (Index * 8);
        }
        Offset += 8;
        return true;
    }

    bool ReadFloat(float& Out)
    {
        uint32 Bits = 0;
        if (!ReadU32(Bits)) return false;
        FMemory::Memcpy(&Out, &Bits, sizeof(float));
        return FMath::IsFinite(Out);
    }

    bool ReadDouble(double& Out)
    {
        uint64 Bits = 0;
        if (!ReadU64(Bits)) return false;
        FMemory::Memcpy(&Out, &Bits, sizeof(double));
        return FMath::IsFinite(Out);
    }

    bool ReadUtf8(uint16 Length, FString& Out)
    {
        if (!Require(Length)) return false;

        const ANSICHAR* Ptr =
            reinterpret_cast<const ANSICHAR*>(Data + Offset);

        FUTF8ToTCHAR Converted(Ptr, Length);
        Out = FString(Converted.Length(), Converted.Get());

        Offset += Length;
        return true;
    }

    bool Skip(int32 Count)
    {
        if (!Require(Count)) return false;
        Offset += Count;
        return true;
    }

    int32 Remaining() const
    {
        return Size - Offset;
    }

private:
    bool Require(int32 Count) const
    {
        return Count >= 0 &&
               Offset >= 0 &&
               Offset + Count <= Size;
    }

    const uint8* Data = nullptr;
    int32 Size = 0;
    int32 Offset = 0;
};

bool DecodeBDFRFrame(
    const uint8* Data,
    int32 Size,
    FBDFRReceivedCurveFrame& OutFrame)
{
    constexpr uint32 BDFRMagic = 0x52464442u;
    constexpr uint16 CodecVersion = 1u;
    constexpr uint32 MaxCurves = 4096u;
    constexpr uint16 MaxNameBytes = 1024u;

    FBDFRByteReader Reader(Data, Size);

    uint32 Magic = 0;
    uint16 Version = 0;
    uint32 SchemaVersion = 0;

    if (!Reader.ReadU32(Magic) ||
        Magic != BDFRMagic ||
        !Reader.ReadU16(Version) ||
        Version != CodecVersion ||
        !Reader.ReadU32(SchemaVersion) ||
        !Reader.ReadDouble(OutFrame.TimestampSeconds) ||
        !Reader.ReadFloat(OutFrame.Confidence))
    {
        return false;
    }

    float Pitch = 0.0f;
    float Yaw = 0.0f;
    float Roll = 0.0f;

    float GazeX = 0.0f;
    float GazeY = 0.0f;
    float GazeConfidence = 0.0f;

    if (!Reader.ReadFloat(Pitch) ||
        !Reader.ReadFloat(Yaw) ||
        !Reader.ReadFloat(Roll) ||
        !Reader.ReadFloat(GazeX) ||
        !Reader.ReadFloat(GazeY) ||
        !Reader.ReadFloat(GazeConfidence))
    {
        return false;
    }

    OutFrame.HeadEuler = FVector3f(Pitch, Yaw, Roll);
    OutFrame.Gaze = FVector3f(GazeX, GazeY, GazeConfidence);

    uint32 CurveCount = 0;
    if (!Reader.ReadU32(CurveCount) ||
        CurveCount > MaxCurves)
    {
        return false;
    }

    OutFrame.Curves.Reset();
    OutFrame.Curves.Reserve(static_cast<int32>(CurveCount));

    for (uint32 Index = 0; Index < CurveCount; ++Index)
    {
        uint16 NameLength = 0;
        if (!Reader.ReadU16(NameLength) ||
            NameLength == 0 ||
            NameLength > MaxNameBytes)
        {
            return false;
        }

        FString Name;
        float Value = 0.0f;

        if (!Reader.ReadUtf8(NameLength, Name) ||
            !Reader.ReadFloat(Value))
        {
            return false;
        }

        OutFrame.Curves.Add(
            FName(*Name),
            FMath::Clamp(Value, 0.0f, 1.0f));
    }

    return Reader.Remaining() == 0;
}

bool DecodeBDFPPacket(
    const uint8* Data,
    int32 Size,
    FBDFRReceivedCurveFrame& OutFrame)
{
    constexpr uint32 BDFPMagic = 0x50464442u;
    constexpr uint16 PacketVersion = 1u;
    constexpr uint16 MaxSourceBytes = 1024u;
    constexpr uint32 MaxFrameBytes = 1024u * 1024u;

    FBDFRByteReader Reader(Data, Size);

    uint32 Magic = 0;
    uint16 Version = 0;
    uint16 SourceLength = 0;
    uint32 FrameLength = 0;

    if (!Reader.ReadU32(Magic) ||
        Magic != BDFPMagic ||
        !Reader.ReadU16(Version) ||
        Version != PacketVersion ||
        !Reader.ReadU64(OutFrame.Sequence) ||
        !Reader.ReadU16(SourceLength) ||
        SourceLength > MaxSourceBytes)
    {
        return false;
    }

    if (!Reader.ReadUtf8(SourceLength, OutFrame.SourceId) ||
        !Reader.ReadU32(FrameLength) ||
        FrameLength == 0 ||
        FrameLength > MaxFrameBytes ||
        Reader.Remaining() != static_cast<int32>(FrameLength))
    {
        return false;
    }

    const uint8* FrameData =
        Data + (Size - Reader.Remaining());

    return DecodeBDFRFrame(
        FrameData,
        static_cast<int32>(FrameLength),
        OutFrame);
}

} // namespace

class UBDFRLiveReceiverComponent::FReceiverWorker final
    : public FRunnable
{
public:
    FReceiverWorker(
        FSocket* InSocket,
        TQueue<FBDFRReceivedCurveFrame, EQueueMode::Mpsc>& InQueue,
        FCriticalSection& InStatsMutex,
        FBDFRLiveReceiverStats& InStats)
        : Socket(InSocket)
        , Queue(InQueue)
        , StatsMutex(InStatsMutex)
        , Stats(InStats)
    {
    }

    virtual uint32 Run() override
    {
        TArray<uint8> Buffer;
        Buffer.SetNumUninitialized(65507);

        while (!bStopRequested)
        {
            uint32 PendingSize = 0;

            if (!Socket ||
                !Socket->HasPendingData(PendingSize))
            {
                FPlatformProcess::SleepNoStats(0.001f);
                continue;
            }

            int32 BytesRead = 0;
            const int32 ReadSize =
                FMath::Min<int32>(
                    static_cast<int32>(PendingSize),
                    Buffer.Num());

            if (!Socket->Recv(
                    Buffer.GetData(),
                    ReadSize,
                    BytesRead,
                    ESocketReceiveFlags::None) ||
                BytesRead <= 0)
            {
                FScopeLock Lock(&StatsMutex);
                ++Stats.DecodeFailures;
                continue;
            }

            FBDFRReceivedCurveFrame Frame;

            if (!DecodeBDFPPacket(
                    Buffer.GetData(),
                    BytesRead,
                    Frame))
            {
                FScopeLock Lock(&StatsMutex);
                ++Stats.DecodeFailures;
                continue;
            }

            {
                FScopeLock Lock(&StatsMutex);

                if (Stats.PacketsReceived > 0 &&
                    Frame.Sequence >
                        static_cast<uint64>(Stats.LastSequence + 1))
                {
                    Stats.SequenceGaps +=
                        static_cast<int64>(
                            Frame.Sequence -
                            static_cast<uint64>(Stats.LastSequence) -
                            1u);
                }

                Stats.LastSequence =
                    static_cast<int64>(Frame.Sequence);

                ++Stats.PacketsReceived;
                Stats.LastCurveCount =
                    Frame.Curves.Num();
            }

            Queue.Enqueue(MoveTemp(Frame));
        }

        return 0;
    }

    virtual void Stop() override
    {
        bStopRequested = true;
    }

private:
    FSocket* Socket = nullptr;

    TQueue<
        FBDFRReceivedCurveFrame,
        EQueueMode::Mpsc>& Queue;

    FCriticalSection& StatsMutex;
    FBDFRLiveReceiverStats& Stats;

    FThreadSafeBool bStopRequested = false;
};

UBDFRLiveReceiverComponent::UBDFRLiveReceiverComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f;
}

void UBDFRLiveReceiverComponent::BeginPlay()
{
    Super::BeginPlay();

    CachedFacialComponent =
        GetOwner()
            ? GetOwner()->FindComponentByClass<
                  UBDFRFacialComponent>()
            : nullptr;

    if (TargetSkeletalMesh)
    {
        CachedSkeletalMesh = TargetSkeletalMesh;
    }
    else
    {
        CachedSkeletalMesh =
            GetOwner()
                ? GetOwner()->FindComponentByClass<
                      USkeletalMeshComponent>()
                : nullptr;
    }

    if (bAutoStart)
    {
        StartReceiver();
    }
}

void UBDFRLiveReceiverComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    StopReceiver();
    Super::EndPlay(EndPlayReason);
}

void UBDFRLiveReceiverComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(
        DeltaTime,
        TickType,
        ThisTickFunction);

    DrainFrames();
}

bool UBDFRLiveReceiverComponent::StartReceiver()
{
    if (IsReceiving())
    {
        return true;
    }

    if (ListenPort <= 0 || ListenPort > 65535)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("BDFR: invalid listen port %d"),
            ListenPort);
        return false;
    }

    Socket =
        FUdpSocketBuilder(TEXT("BDFR_UDP_Receiver"))
            .AsNonBlocking()
            .AsReusable()
            .BoundToAddress(FIPv4Address::Any)
            .BoundToPort(ListenPort)
            .WithReceiveBufferSize(2 * 1024 * 1024);

    if (!Socket)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("BDFR: failed to create UDP receiver on port %d"),
            ListenPort);
        return false;
    }

    {
        FScopeLock Lock(&StatsMutex);
        Stats = {};
    }

    if (bPublishLiveLink)
    {
        RegisterLiveLinkSource();
    }

    Worker =
        MakeUnique<FReceiverWorker>(
            Socket,
            ReceivedFrames,
            StatsMutex,
            Stats);

    WorkerThread =
        FRunnableThread::Create(
            Worker.Get(),
            TEXT("BDFR_UDP_Receiver"),
            0,
            TPri_AboveNormal);

    if (!WorkerThread)
    {
        StopReceiver();
        return false;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("BDFR: listening for BDFP on UDP %d"),
        ListenPort);

    return true;
}

void UBDFRLiveReceiverComponent::StopReceiver()
{
    if (Worker)
    {
        Worker->Stop();
    }

    if (WorkerThread)
    {
        WorkerThread->WaitForCompletion();
        delete WorkerThread;
        WorkerThread = nullptr;
    }

    Worker.Reset();

    if (Socket)
    {
        Socket->Close();

        if (ISocketSubsystem* SocketSubsystem =
                ISocketSubsystem::Get(
                    PLATFORM_SOCKETSUBSYSTEM))
        {
            SocketSubsystem->DestroySocket(Socket);
        }

        Socket = nullptr;
    }

    UnregisterLiveLinkSource();

    FBDFRReceivedCurveFrame Discarded;
    while (ReceivedFrames.Dequeue(Discarded))
    {
    }
}

bool UBDFRLiveReceiverComponent::IsReceiving() const
{
    return Socket != nullptr &&
           WorkerThread != nullptr;
}

FBDFRLiveReceiverStats
UBDFRLiveReceiverComponent::GetStats() const
{
    FScopeLock Lock(&StatsMutex);
    return Stats;
}

void UBDFRLiveReceiverComponent::DrainFrames()
{
    FBDFRReceivedCurveFrame Frame;
    FBDFRReceivedCurveFrame Latest;
    bool bHasFrame = false;

    // Collapse a burst to the newest frame to keep live latency bounded.
    while (ReceivedFrames.Dequeue(Frame))
    {
        Latest = MoveTemp(Frame);
        bHasFrame = true;
    }

    if (!bHasFrame)
    {
        return;
    }

    if (bApplyToFacialComponent)
    {
        ApplyToOwner(Latest);
    }

    if (bPublishLiveLink &&
        LiveLinkSource.IsValid())
    {
        PublishLiveLink(Latest);
    }
}

void UBDFRLiveReceiverComponent::ApplyToOwner(
    const FBDFRReceivedCurveFrame& Frame)
{
    UBDFRFacialComponent* FacialComponent =
        CachedFacialComponent.Get();

    if (!FacialComponent && GetOwner())
    {
        FacialComponent =
            GetOwner()->FindComponentByClass<
                UBDFRFacialComponent>();

        CachedFacialComponent =
            FacialComponent;
    }

    if (FacialComponent)
    {
        FacialComponent->SetCurveValues(
            Frame.Curves);

        if (bApplyToSkeletalMesh)
        {
            USkeletalMeshComponent* SkeletalMesh =
                CachedSkeletalMesh.Get();

            if (!SkeletalMesh && GetOwner())
            {
                SkeletalMesh =
                    GetOwner()->FindComponentByClass<
                        USkeletalMeshComponent>();

                CachedSkeletalMesh =
                    SkeletalMesh;
            }

            if (SkeletalMesh)
            {
                FacialComponent->ApplyToSkeletalMesh(
                    SkeletalMesh);
            }
        }
    }
}

void UBDFRLiveReceiverComponent::PublishLiveLink(
    const FBDFRReceivedCurveFrame& Frame)
{
    if (!LiveLinkSource.IsValid())
    {
        return;
    }

    FBDFRLiveLinkCurveFrame LiveFrame;
    LiveFrame.TimestampSeconds =
        Frame.TimestampSeconds;
    LiveFrame.Curves =
        Frame.Curves;

    LiveLinkSource->EnqueueFrame(
        MoveTemp(LiveFrame));
}

bool UBDFRLiveReceiverComponent::RegisterLiveLinkSource()
{
    if (LiveLinkSource.IsValid())
    {
        return true;
    }

    if (!IModularFeatures::Get().IsModularFeatureAvailable(
            ILiveLinkClient::ModularFeatureName))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("BDFR: Live Link client is not available."));
        return false;
    }

    LiveLinkClient =
        &IModularFeatures::Get()
             .GetModularFeature<ILiveLinkClient>(
                 ILiveLinkClient::ModularFeatureName);

    LiveLinkSource =
        MakeShared<FBDFRLiveLinkSource>(
            LiveLinkSubjectName);

    LiveLinkClient->AddSource(
        LiveLinkSource);

    return true;
}

void UBDFRLiveReceiverComponent::UnregisterLiveLinkSource()
{
    if (LiveLinkClient &&
        LiveLinkSource.IsValid())
    {
        LiveLinkClient->RemoveSource(
            LiveLinkSource);
    }

    LiveLinkSource.Reset();
    LiveLinkClient = nullptr;
}
