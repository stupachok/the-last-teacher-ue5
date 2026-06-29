#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FindTarget.generated.h"

UCLASS()
class THE_LAST_TEACHER_API UBTService_FindTarget : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_FindTarget();

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;

protected:
    // –ад≥ус агро
    UPROPERTY(EditAnywhere, Category = "Detection")
    float AggroRadius = 3000.0f;

    // якщо монстр не наближаЇтьс€ до ц≥л≥ за цей час (сек) Ч здаЇтьс€
    UPROPERTY(EditAnywhere, Category = "Detection")
    float GiveUpTime = 5.0f;

    // ћ≥н≥мальне наближенн€ за GiveUpTime щоб не здаватись
    UPROPERTY(EditAnywhere, Category = "Detection")
    float MinProgressDistance = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName TargetActorKey = TEXT("TargetActor");

private:
    // ¬нутр≥шн≥й стан Ч ск≥льки часу пересл≥дуЇмо без прогресу
    float TimeWithoutProgress = 0.0f;
    float LastDistanceToTarget = FLT_MAX;
};