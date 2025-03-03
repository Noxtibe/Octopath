#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyTrigger.generated.h"

class USphereComponent;

UCLASS()
class OCTOPATH_API AEnemyTrigger : public AActor
{
    GENERATED_BODY()

public:
    // Constructor
    AEnemyTrigger();

protected:
    virtual void BeginPlay() override;

public:
    // Collision component used to detect overlaps with the player
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
    USphereComponent* CollisionSphere;

    // Name of the combat level to load when the player overlaps this trigger
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FName CombatLevelName;

    // Function called when an overlap begins
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);
};
