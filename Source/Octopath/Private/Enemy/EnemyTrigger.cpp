#include "Enemy/EnemyTrigger.h"

#include "Components/SphereComponent.h"

#include "Kismet/GameplayStatics.h"

#include "Character/Hikari.h"

#include "Game/OctopathGameInstance.h"

AEnemyTrigger::AEnemyTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create and configure the collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(100.f);
    CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = CollisionSphere;

    // Bind the overlap event to our custom function
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyTrigger::OnOverlapBegin);

    // Set the default name of the combat level
    CombatLevelName = FName("LVL_Fight");
}

void AEnemyTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor && (OtherActor != this))
    {
        // Check if the overlapping actor is the player (of type AHikari)
        if (OtherActor->IsA(AHikari::StaticClass()))
        {
            // Retrieve our custom game instance and set a random enemy count (between 1 and 3)
            if (UOctopathGameInstance* GI = Cast<UOctopathGameInstance>(UGameplayStatics::GetGameInstance(this)))
            {
                GI->NumberOfEnemies = FMath::RandRange(1, 3);
            }

            // Switch to the combat level
            UGameplayStatics::OpenLevel(this, CombatLevelName);
        }
    }
}
