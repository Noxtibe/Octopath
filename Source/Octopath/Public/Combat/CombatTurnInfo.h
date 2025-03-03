#pragma once

#include "CoreMinimal.h"
#include "CombatTurnInfo.generated.h"

/**
 * Struct to hold combatant turn information.
 */
USTRUCT(BlueprintType)
struct FCombatantTurnInfo
{
	GENERATED_BODY()

	// The combatant actor (player or enemy)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info")
	AActor* Combatant;

	// The Speed stat (used for determining turn order)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info")
	float Speed;

	// Icon representing the combatant
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info", meta = (DisplayThumbnail = "true"))
	UTexture2D* Icon;
};
