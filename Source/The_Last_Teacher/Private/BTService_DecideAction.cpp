#include "BTService_DecideAction.h"
#include "NeuralNetwork.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UBTService_DecideAction::UBTService_DecideAction()
{
    NodeName = TEXT("Decide Action (NN)");
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    bNotifyTick = true;
    bTickIntervals = true;
}

void UBTService_DecideAction::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return;

    APawn* Bot = AICon->GetPawn();
    if (!Bot) return;

    FVector BotLocation = Bot->GetActorLocation();

    // ===== „итаЇмо стан =====
    bool bIsFollowing = BB->GetValueAsBool(IsFollowingKey);
    float RepairSkill = BB->GetValueAsFloat(RepairSkillKey);
    float FightSkill = BB->GetValueAsFloat(TEXT("FightSkill"));

    // ===== ЎукаЇмо Ќј…ЅЋ»∆„ќ√ќ монстра =====
    AActor* TargetMonster = nullptr;
    float ClosestMonsterDist = FLT_MAX;

    TArray<AActor*> AllPawns;
    UGameplayStatics::GetAllActorsOfClass(Bot->GetWorld(), APawn::StaticClass(), AllPawns);
    for (AActor* M : AllPawns)
    {
        if (!M || !IsValid(M) || M == Bot) continue;
        FString CN = M->GetClass()->GetName();
        if (!CN.Contains(TEXT("Monster"))) continue;
        float D = FVector::Dist(BotLocation, M->GetActorLocation());
        if (D < ClosestMonsterDist)
        {
            ClosestMonsterDist = D;
            TargetMonster = M;
        }
    }

    float DistToMonster = (TargetMonster && ClosestMonsterDist < FLT_MAX)
        ? FMath::Clamp(ClosestMonsterDist / MaxDetectionRange, 0.0f, 1.0f) : 1.0f;
    float HasMonster = TargetMonster ? 1.0f : 0.0f;

    // ===== ЎукаЇмо Ќј…ЅЋ»∆„»… зламаний блок =====
    AActor* TargetBlock = nullptr;
    float ClosestBlockDist = FLT_MAX;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(Bot->GetWorld(), AActor::StaticClass(), AllActors);
    for (AActor* A : AllActors)
    {
        if (!A || !IsValid(A)) continue;
        FString CN = A->GetClass()->GetName();
        if (!CN.Contains(TEXT("RepairBlock"))) continue;

        // ѕерев≥р€Їмо IsBroken через рефлекс≥ю
        FBoolProperty* BrokenProp = FindFProperty<FBoolProperty>(
            A->GetClass(), TEXT("IsBroken"));
        if (!BrokenProp) continue;
        bool bIsBroken = BrokenProp->GetPropertyValue_InContainer(A);
        if (!bIsBroken) continue; // блок не зламаний Ч пропускаЇмо

        float D = FVector::Dist(BotLocation, A->GetActorLocation());
        if (D < ClosestBlockDist)
        {
            ClosestBlockDist = D;
            TargetBlock = A;
        }
    }

    // якщо в Blackboard вже Ї блок Ч перев≥р€Їмо чи в≥н зламаний ≥ пор≥внюЇмо
    AActor* BBBlock = Cast<AActor>(BB->GetValueAsObject(TargetBlockKey));
    if (BBBlock && IsValid(BBBlock))
    {
        FBoolProperty* BrokenProp = FindFProperty<FBoolProperty>(
            BBBlock->GetClass(), TEXT("IsBroken"));
        bool bIsBroken = BrokenProp ? BrokenProp->GetPropertyValue_InContainer(BBBlock) : false;

        if (bIsBroken)
        {
            float BBBlockDist = FVector::Dist(BotLocation, BBBlock->GetActorLocation());
            if (BBBlockDist < ClosestBlockDist)
            {
                ClosestBlockDist = BBBlockDist;
                TargetBlock = BBBlock;
            }
        }
    }

    float DistToBlock = (TargetBlock && ClosestBlockDist < FLT_MAX)
        ? FMath::Clamp(ClosestBlockDist / MaxDetectionRange, 0.0f, 1.0f) : 1.0f;
    float HasBlock = TargetBlock ? 1.0f : 0.0f;

    // ===== √равець =====
    AActor* Player = Cast<AActor>(BB->GetValueAsObject(PlayerActorKey));
    float DistToPlayer = 1.0f;
    if (Player && IsValid(Player))
    {
        float Dist = FVector::Dist(BotLocation, Player->GetActorLocation());
        DistToPlayer = FMath::Clamp(Dist / MaxDetectionRange, 0.0f, 1.0f);
    }

    float IsFollowingF = bIsFollowing ? 1.0f : 0.0f;
    float RepairSkillN = FMath::Clamp(RepairSkill / 100.0f, 0.0f, 1.0f);
    float FightSkillN = FMath::Clamp(FightSkill / 100.0f, 0.0f, 1.0f);

    TArray<float> Inputs;
    Inputs.Add(DistToMonster);
    Inputs.Add(HasMonster);
    Inputs.Add(1.0f);
    Inputs.Add(DistToBlock);
    Inputs.Add(HasBlock);
    Inputs.Add(DistToPlayer);
    Inputs.Add(IsFollowingF);
    Inputs.Add(RepairSkillN);
    Inputs.Add(FightSkillN);

    NeuralNetwork NN;
    int32 NNAction = NN.Predict(Inputs);

    // ===== ѕ–≤ќ–»“≈“Ќј Ћќ√≤ ј =====
    // 1. Follow Ч найвищий пр≥оритет
    // 2. ћонстр в рад≥ус≥ + FightSkill
    // 3. «ламаний блок + RepairSkill (без обмеженн€ дистанц≥њ)
    // 4. GoHome Ч т≥льки €кщо немаЇ Ќ≤ монстра Ќ≤ зламаного блоку

    int32 ChosenAction = 3;

    if (bIsFollowing)
    {
        ChosenAction = 2; // Follow
    }
    else if (RepairSkill < SkillThreshold && FightSkill < SkillThreshold)
    {
        ChosenAction = 3; // нема ск≥л≥в Ч додому
    }
    else if (TargetMonster && FightSkill >= SkillThreshold && ClosestMonsterDist <= ActionRadius)
    {
        BB->SetValueAsObject(TargetMonsterKey, TargetMonster);
        ChosenAction = 0; // Fight
    }
    else if (TargetBlock && RepairSkill >= SkillThreshold)
    {
        // ™ зламаний блок Ч йдемо незалежно в≥д дистанц≥њ
        BB->SetValueAsObject(TargetBlockKey, TargetBlock);
        ChosenAction = 1; // Repair
    }

    int32 CurrentAction = BB->GetValueAsInt(ChosenActionKey);
    if (CurrentAction != ChosenAction)
    {
        BB->SetValueAsInt(ChosenActionKey, ChosenAction);
    }

    if (bEnableDebugLog)
    {
        UE_LOG(LogTemp, Log,
            TEXT("[NN-Service] Bot %s | NN=%d Final=%d | Repair=%.0f Fight=%.0f | Follow=%d HasMon=%d MonDist=%.0f HasBlock=%d BlockDist=%.0f"),
            *Bot->GetName(), NNAction, ChosenAction,
            RepairSkill, FightSkill,
            bIsFollowing ? 1 : 0,
            (int)HasMonster,
            ClosestMonsterDist < FLT_MAX ? ClosestMonsterDist : -1.f,
            (int)HasBlock,
            ClosestBlockDist < FLT_MAX ? ClosestBlockDist : -1.f);
    }
}