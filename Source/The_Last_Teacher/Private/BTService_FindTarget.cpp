#include "BTService_FindTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UBTService_FindTarget::UBTService_FindTarget()
{
    NodeName = TEXT("Find Target (Player or Bot)");
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    bNotifyTick = true;
    bTickIntervals = true;
}

void UBTService_FindTarget::TickNode(UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return;

    APawn* Monster = AICon->GetPawn();
    if (!Monster) return;

    FVector MonsterLocation = Monster->GetActorLocation();
    UWorld* World = Monster->GetWorld();
    if (!World) return;

    // ===== ѕошук найближчоњ ц≥л≥ в рад≥ус≥ =====
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    AActor* NearestTarget = nullptr;
    float NearestDistance = FLT_MAX;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !IsValid(Actor)) continue;
        if (Actor == Monster) continue;

        FString ClassName = Actor->GetClass()->GetName();
        if (ClassName.Contains(TEXT("Monster"))) continue;

        float Distance = FVector::Dist(MonsterLocation, Actor->GetActorLocation());
        if (Distance > AggroRadius) continue;

        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestTarget = Actor;
        }
    }

    // ===== якщо н≥кого в рад≥ус≥ Ч очищаЇмо ≥ скидаЇмо таймер =====
    if (!NearestTarget)
    {
        BB->ClearValue(TargetActorKey);
        TimeWithoutProgress = 0.0f;
        LastDistanceToTarget = FLT_MAX;
        return;
    }

    // ===== ѕерев≥рка прогресу Ч чи монстр наближаЇтьс€ до ц≥л≥? =====
    if (NearestDistance < LastDistanceToTarget - MinProgressDistance)
    {
        // ћонстр наближаЇтьс€ Ч все ок, скидаЇмо таймер
        TimeWithoutProgress = 0.0f;
        LastDistanceToTarget = NearestDistance;
    }
    else
    {
        // ћонстр не наближаЇтьс€ Ч накопичуЇмо час
        TimeWithoutProgress += DeltaSeconds;
    }

    // ===== якщо застр€г Ч здаЇмось =====
    if (TimeWithoutProgress >= GiveUpTime)
    {
        UE_LOG(LogTemp, Log, TEXT("[Monster] Can't reach %s Ч giving up!"),
            *NearestTarget->GetName());

        BB->ClearValue(TargetActorKey);
        TimeWithoutProgress = 0.0f;
        LastDistanceToTarget = FLT_MAX;
        return;
    }

    // ===== ¬се ок Ч пересл≥дуЇмо =====
    BB->SetValueAsObject(TargetActorKey, NearestTarget);

    UE_LOG(LogTemp, Log, TEXT("[Monster] Chasing: %s | Dist: %.0f | StuckTime: %.1f/%.1f"),
        *NearestTarget->GetName(), NearestDistance,
        TimeWithoutProgress, GiveUpTime);
}