#include "OctopathGameMode.h"
#include "Character/Hikari.h"
#include "Game/HikariPlayerController.h"

#include "UObject/ConstructorHelpers.h"

AOctopathGameMode::AOctopathGameMode()
{
     static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_Hikari"));
     if (PlayerPawnBPClass.Succeeded())
     {
         DefaultPawnClass = PlayerPawnBPClass.Class;
     }
     else
     {
         DefaultPawnClass = AHikari::StaticClass();
     }

    DefaultPawnClass = AHikari::StaticClass();

    PlayerControllerClass = AHikariPlayerController::StaticClass();
}

void AOctopathGameMode::BeginPlay()
{
    Super::BeginPlay();
}
