#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OctopathGameInstance.generated.h"

/**
 * Game Instance
 */
UCLASS()
class OCTOPATH_API UOctopathGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UOctopathGameInstance();

    // Number of enemies to spawn in the combat phase (set by the EnemyTrigger)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 NumberOfEnemies;

    UFUNCTION(BlueprintCallable, Category = "Game Data")
    void ResetGameData();
};
