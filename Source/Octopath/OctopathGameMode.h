#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OctopathGameMode.generated.h"

/**
 * Game Mode 
 */
UCLASS()
class OCTOPATH_API AOctopathGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AOctopathGameMode();

protected:
    virtual void BeginPlay() override;
};
