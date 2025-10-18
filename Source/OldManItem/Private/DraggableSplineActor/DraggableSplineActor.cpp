// DraggableSplineActor.cpp
#include "DraggableSplineActor/DraggableSplineActor.h"
#include "Engine/World.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

ADraggableSplineActor::ADraggableSplineActor()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    SplineComponent->SetupAttachment(RootComponent);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    CurrentSplinePosition = 0.0f;
    bIsBeingDragged = false;
    MovementAlpha = 1.0f;

    Tags.Add(DragableItem);
}

void ADraggableSplineActor::BeginPlay()
{
    Super::BeginPlay();

    // �洢������ĳ�ʼλ�ú���ת
    InitialMeshLocation = MeshComponent->GetRelativeLocation();
    InitialMeshRotation = MeshComponent->GetRelativeRotation();

    TargetLocation = MeshComponent->GetComponentLocation();
    TargetRotation = MeshComponent->GetComponentRotation();
}

void ADraggableSplineActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ƽ���ƶ���ֵ
    if (MovementAlpha < 1.0f)
    {
        MovementAlpha = FMath::Min(MovementAlpha + DeltaTime * 8.0f, 1.0f);

        FVector NewLocation = FMath::Lerp(MeshComponent->GetComponentLocation(), TargetLocation, MovementAlpha);
        FRotator NewRotation = FMath::Lerp(MeshComponent->GetComponentRotation(), TargetRotation, MovementAlpha);

        SetMeshPositionAndRotation(NewLocation, NewRotation);
    }

    // �������������ߵ��ԣ���ʹ�����϶�״̬��
    if (bShowDebugVisualization && SplineComponent)
    {
        // ����������·��
        const int32 NumSegments = 50;
        for (int32 i = 0; i < NumSegments; i++)
        {
            float Time1 = (float)i / NumSegments;
            float Time2 = (float)(i + 1) / NumSegments;

            FVector Point1 = SplineComponent->GetLocationAtTime(Time1, ESplineCoordinateSpace::World);
            FVector Point2 = SplineComponent->GetLocationAtTime(Time2, ESplineCoordinateSpace::World);

            DrawDebugLine(GetWorld(), Point1, Point2, FColor::Green, false, -1.0f, 0, 2.0f);
        }

        // ���Ƶ�ǰλ�ñ��
        FVector CurrentPos = SplineComponent->GetLocationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
        DrawDebugSphere(GetWorld(), CurrentPos, 10.0f, 8, FColor::Yellow, false, -1.0f, 0);

        // ���Ʊ༭��Ԥ��λ�ñ�ǣ�������ã�
#if WITH_EDITOR
        if (bEnableEditorPreview && !GetWorld()->IsGameWorld())
        {
            FVector PreviewPos = SplineComponent->GetLocationAtTime(EditorPreviewPosition, ESplineCoordinateSpace::World);
            DrawDebugSphere(GetWorld(), PreviewPos, 15.0f, 12, FColor::Cyan, false, -1.0f, 0);
            DrawDebugString(GetWorld(), PreviewPos + FVector(0, 0, 40),
                *FString::Printf(TEXT("Ԥ��λ��: %.2f"), EditorPreviewPosition),
                nullptr, FColor::Cyan, 0.0f, true);
        }
#endif
    }
}

#if WITH_EDITOR
void ADraggableSplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    // ���༭��Ԥ��λ�øı�ʱ����������
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ADraggableSplineActor, EditorPreviewPosition))
    {
        UpdatePreviewPosition();
    }

    // ������/����Ԥ��ʱ������λ��
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ADraggableSplineActor, bEnableEditorPreview))
    {
        if (bEnableEditorPreview)
        {
            UpdatePreviewPosition();
        }
        else
        {
            // �ָ�ԭʼλ�� - ����������λ��
            MeshComponent->SetRelativeLocation(InitialMeshLocation);
            MeshComponent->SetRelativeRotation(InitialMeshRotation);
        }
    }
}
#endif

void ADraggableSplineActor::StartDragging()
{
    bIsBeingDragged = true;
    MovementAlpha = 0.0f;

    // ��������˱༭��Ԥ������ʱ������
#if WITH_EDITOR
    if (bEnableEditorPreview)
    {
        bEnableEditorPreview = false;
    }
#endif
}

void ADraggableSplineActor::StopDragging()
{
    bIsBeingDragged = false;
}

void ADraggableSplineActor::HandleMouseData(const FVector& ViewDirection, float Intensity)
{
    MoveBasedOnViewDirection(ViewDirection, Intensity);
}

FVector ADraggableSplineActor::GetCurrentTangent() const
{
    if (SplineComponent)
    {
        return SplineComponent->GetTangentAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World).GetSafeNormal();
    }
    return FVector::ForwardVector;
}

float ADraggableSplineActor::CalculateNormalizedMovement(const FVector& ViewDirection, float Intensity)
{
    if (!SplineComponent) return 0.0f;

    // ��ȡ�������ڵ�ǰ������߷���·������
    FVector SplineTangent = GetCurrentTangent();

    // ���ӽ��ƶ�����ͶӰ��������������
    float ProjectedMovement = FVector::DotProduct(ViewDirection, SplineTangent);

    // Ӧ������ - С�����ƶ�����Ӧ
    if (FMath::Abs(ProjectedMovement) < DeadZone)
    {
        return 0.0f;
    }

    // Ӧ��������
    float ScaledMovement = ProjectedMovement * DragSensitivity;

    // ��������ٶ�
    float ClampedMovement = FMath::Clamp(ScaledMovement, -MaxDragSpeed, MaxDragSpeed);

    // Ӧ�÷�������Ӧ���ߣ������ָ�
    // С���ƶ������У�����ƶ���ƽ��
    float ResponsiveMovement = FMath::Sign(ClampedMovement) * FMath::Pow(FMath::Abs(ClampedMovement), 0.8f);

    return ResponsiveMovement;
}

void ADraggableSplineActor::MoveBasedOnViewDirection(const FVector& ViewDirection, float Intensity)
{
    if (!SplineComponent || !bIsBeingDragged) return;

    // �����һ���ƶ���
    float MovementDelta = CalculateNormalizedMovement(ViewDirection, Intensity);

    // ����ƶ�����С������
    if (FMath::Abs(MovementDelta) < 0.001f) return;

    // ����λ��
    CurrentSplinePosition = FMath::Clamp(CurrentSplinePosition + MovementDelta, 0.0f, 1.0f);

    // ������λ�ú���ת
    TargetLocation = SplineComponent->GetLocationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
    TargetRotation = SplineComponent->GetRotationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);

    MovementAlpha = 0.0f;

    // ���Ƶ��Կ��ӻ�
    if (bShowDebugVisualization)
    {
        FVector SplineTangent = GetCurrentTangent();
        float ProjectedMovement = FVector::DotProduct(ViewDirection, SplineTangent);
        DrawDebugVisualization(ViewDirection, ProjectedMovement);

        // ��ʾ�ƶ���Ϣ
        if (GEngine)
        {
            FString DebugText = FString::Printf(
                TEXT("ͶӰֵ: %.2f\nλ��: %.2f\n�ƶ���: %.4f"),
                ProjectedMovement, CurrentSplinePosition, MovementDelta
            );
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::White, DebugText);
        }
    }
}

void ADraggableSplineActor::DrawDebugVisualization(const FVector& ViewDirection, float ProjectedMovement)
{
    FVector CurrentLocation = MeshComponent->GetComponentLocation();
    FVector SplineTangent = GetCurrentTangent();

    // 1. �������������߷�����ɫ��
    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + SplineTangent * DebugLineLength, DebugArrowSize, FColor::Green, false, 0.1f, 0, 3.0f);

    // 2. �����ӽ��ƶ�������ɫ��
    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + ViewDirection * DebugLineLength, DebugArrowSize, FColor::Blue, false, 0.1f, 0, 3.0f);

    // 3. ����ͶӰ�������ɫ��
    FVector ProjectedVector = SplineTangent * ProjectedMovement * DebugLineLength;
    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + ProjectedVector, DebugArrowSize, FColor::Red, false, 0.1f, 0, 4.0f);

    // 4. ���������������յ�
    FVector StartLocation = SplineComponent->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World);
    FVector EndLocation = SplineComponent->GetLocationAtTime(1.0f, ESplineCoordinateSpace::World);

    DrawDebugSphere(GetWorld(), StartLocation, 15.0f, 12, FColor::Green, false, 0.1f, 0);
    DrawDebugSphere(GetWorld(), EndLocation, 15.0f, 12, FColor::Red, false, 0.1f, 0);

    // 5. ���������߷����ǩ
    DrawDebugString(GetWorld(), StartLocation + FVector(0, 0, 30), TEXT("StartPoint"), nullptr, FColor::Green, 0.1f, false);
    DrawDebugString(GetWorld(), EndLocation + FVector(0, 0, 30), TEXT("EndPoint"), nullptr, FColor::Red, 0.1f, false);

    // 6. ���Ƶ�ǰ�����߷����ǩ
    DrawDebugString(GetWorld(), CurrentLocation + FVector(0, 0, 50),
        *FString::Printf(TEXT("����: (%.1f, %.1f, %.1f)"), SplineTangent.X, SplineTangent.Y, SplineTangent.Z),
        nullptr, FColor::Green, 0.1f, false);
}

// �༭��Ԥ������ʵ��
void ADraggableSplineActor::UpdateEditorPreview()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        UpdatePreviewPosition();
    }
#endif
}

void ADraggableSplineActor::ToggleEditorPreview()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        bEnableEditorPreview = !bEnableEditorPreview;
        if (bEnableEditorPreview)
        {
            UpdatePreviewPosition();
        }
        else
        {
            // ����������λ��
            MeshComponent->SetRelativeLocation(InitialMeshLocation);
            MeshComponent->SetRelativeRotation(InitialMeshRotation);
        }
    }
#endif
}

void ADraggableSplineActor::ResetEditorPreview()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        EditorPreviewPosition = 0.0f;
        UpdatePreviewPosition();
    }
#endif
}

void ADraggableSplineActor::UpdatePreviewPosition()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld() && SplineComponent)
    {
        FVector NewLocation = SplineComponent->GetLocationAtTime(EditorPreviewPosition, ESplineCoordinateSpace::World);
        FRotator NewRotation = SplineComponent->GetRotationAtTime(EditorPreviewPosition, ESplineCoordinateSpace::World);

        SetMeshPositionAndRotation(NewLocation, NewRotation);

        // �ڱ༭���б��Ϊ��Ҫ�ػ�
        MarkComponentsRenderStateDirty();
    }
#endif
}

void ADraggableSplineActor::SetMeshPositionAndRotation(const FVector& Location, const FRotator& Rotation)
{
    if (MeshComponent)
    {
        // ����������ת��Ϊ�����Actor�ľֲ�����
        FVector LocalLocation = GetActorTransform().InverseTransformPosition(Location);
        FRotator LocalRotation = (GetActorTransform().InverseTransformRotation(Rotation.Quaternion())).Rotator();

        MeshComponent->SetRelativeLocation(LocalLocation);
        MeshComponent->SetRelativeRotation(LocalRotation);
    }
}