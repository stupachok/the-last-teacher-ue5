#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_DecideAction.generated.h"

UCLASS()
class THE_LAST_TEACHER_API UBTService_DecideAction : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_DecideAction();

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;

protected:
    // Мінімальний скілл для дії (менше = нічого не робить)
    UPROPERTY(EditAnywhere, Category = "Decision")
    float SkillThreshold = 10.0f;

    // Максимальний радіус дії — далі бот ігнорує цілі
    UPROPERTY(EditAnywhere, Category = "Decision")
    float ActionRadius = 50000.0f;

    UPROPERTY(EditAnywhere, Category = "Decision")
    float MaxDetectionRange = 100000.0f;

    UPROPERTY(EditAnywhere, Category = "Decision")
    bool bEnableDebugLog = true;

    UPROPERTY(EditAnywhere, Category = "Blackboard Keys")
    FName ChosenActionKey = TEXT("ChosenAction");

    UPROPERTY(EditAnywhere, Category = "Blackboard Keys")
    FName TargetMonsterKey = TEXT("TargetMonster");

    UPROPERTY(EditAnywhere, Category = "Blackboard Keys")
    FName TargetBlockKey = TEXT("TargetBlock");

    UPROPERTY(EditAnywhere, Category = "Blackboard Keys")
    FName PlayerActorKey = TEXT("PlayerActor");

    UPROPERTY(EditAnywhere, Category = "Blackboard Keys")
    FName IsFollowingKey = TEXT("IsFollowing");

    UPROPERTY(EditAnywhere, Category = "Blackboard Keys")
    FName RepairSkillKey = TEXT("RepairSkill");
};