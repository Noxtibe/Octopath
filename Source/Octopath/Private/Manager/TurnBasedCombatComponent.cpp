#include "Manager/TurnBasedCombatComponent.h"
#include "Manager/StatComponent.h"
#include "Enemy/EnemyAbilityComponent.h"
#include "Character/AllyAbilityComponent.h"

#include "Blueprint/UserWidget.h"
#include "Widget/TurnOrderWidget.h"
#include "Widget/PlayerTurnMenuWidget.h"
#include "Widget/PlayerStatsWidget.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/HikariPlayerController.h"

#include "Engine/Engine.h"

UTurnBasedCombatComponent::UTurnBasedCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentTurnIndex = 0;
    SelectedEnemy = nullptr;
    CurrentEnemyIndicatorWidget = nullptr;
    bIsSelectingTarget = false;
    bTargetLocked = false;
    bWasLeftMouseDown = false;
    PlayerIconTexture = nullptr;
    EnemyIconTexture = nullptr;
    bPlayerFled = false;
}

//////////////////////////////////////////////////////////////////////////
// BeginPlay and Combat Flow

void UTurnBasedCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    // Assemble combatants: add the player and all enemies.
    if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        Combatants.Add(PlayerCharacter);
    }
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FoundEnemies);
    for (AActor* Enemy : FoundEnemies)
    {
        Combatants.Add(Enemy);
    }

    // Create and display the Turn Order widget.
    if (TurnOrderWidgetClass)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            TurnOrderWidget = CreateWidget<UTurnOrderWidget>(PC, TurnOrderWidgetClass);
            if (TurnOrderWidget)
            {
                TurnOrderWidget->AddToViewport();
            }
        }
    }

    // Create and display the Player Stats widget.
    if (PlayerStatsWidgetClass)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PlayerStatsWidget = CreateWidget<UPlayerStatsWidget>(PC, PlayerStatsWidgetClass);
            if (PlayerStatsWidget)
            {
                PlayerStatsWidget->AddToViewport();
            }
        }
    }

    StartCombat();
}

void UTurnBasedCombatComponent::StartCombat()
{
    bPlayerFled = false;
    CurrentTurnIndex = 0;

    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    TArray<FCombatantTurnInfo> TurnInfos;
    for (AActor* Actor : Combatants)
    {
        if (!Actor) continue;
        if (UStatComponent* StatComp = Actor->FindComponentByClass<UStatComponent>())
        {
            FCombatantTurnInfo Info;
            Info.Combatant = Actor;
            Info.Speed = StatComp->Speed;
            Info.Icon = (Actor == PlayerActor) ? PlayerIconTexture : EnemyIconTexture;
            TurnInfos.Add(Info);
        }
    }

    TurnInfos.Sort([](const FCombatantTurnInfo& A, const FCombatantTurnInfo& B)
        {
            return A.Speed > B.Speed;
        });

    Combatants.Empty();
    for (const FCombatantTurnInfo& Info : TurnInfos)
    {
        Combatants.Add(Info.Combatant);
    }

    if (TurnOrderWidget)
    {
        TurnOrderWidget->UpdateTurnOrder(TurnInfos, SelectedEnemy);

    }

    // Log initial HP for each combatant.
    for (AActor* Actor : Combatants)
    {
        if (Actor)
        {
            if (UStatComponent* StatComp = Actor->FindComponentByClass<UStatComponent>())
            {
                UE_LOG(LogTemp, Log, TEXT("Initial HP of %s: %f/%f"), *Actor->GetName(), StatComp->Health, StatComp->MaxHealth);
            }
        }
    }

    // If it's the player's turn, disable movement and show the action menu.
    if (Combatants.IsValidIndex(CurrentTurnIndex) && Combatants[CurrentTurnIndex] == PlayerActor)
    {
        if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerActor))
        {
            PlayerChar->GetCharacterMovement()->DisableMovement();
        }

        if (PlayerTurnMenuWidgetClass)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (!PlayerTurnMenuWidget)
                {
                    PlayerTurnMenuWidget = CreateWidget<UPlayerTurnMenuWidget>(PC, PlayerTurnMenuWidgetClass);
                    if (PlayerTurnMenuWidget)
                    {
                        PlayerTurnMenuWidget->AddToViewport();
                        PlayerTurnMenuWidget->OnAttackSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerAttack);
                        PlayerTurnMenuWidget->OnFleeSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerFlee);
                    }
                }
                else
                {
                    PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Visible);
                }
            }
        }

        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            UE_LOG(LogTemp, Log, TEXT("Player's turn: enabling combat input mode"));
            PC->EnableCombatInputMode();
        }

        bIsSelectingTarget = false;
        bTargetLocked = false;
        UE_LOG(LogTemp, Log, TEXT("Combat started. Target selection mode inactive. Press Attack to begin target selection."));
    }
    else
    {
        if (PlayerTurnMenuWidget)
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        }
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->DisableCombatInputMode();
        }
    }
}

void UTurnBasedCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // In target selection mode, automatically update the target based on the cursor.
    if (!bIsSelectingTarget)
        return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
        return;

    FHitResult Hit;
    if (!PC->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit))
        return;

    AActor* HitActor = Hit.GetActor();
    if (HitActor && HitActor->ActorHasTag("Enemy"))
    {
        if (SelectedEnemy != HitActor)
        {
            if (SelectedEnemy)
            {
                RemoveFeedbackFromEnemy(SelectedEnemy);
            }
            SetSelectedEnemy(HitActor);
            bTargetLocked = true;
            UE_LOG(LogTemp, Log, TEXT("Target locked on enemy: %s"), *HitActor->GetName());
        }
        else
        {
            UpdateEnemyIndicatorPosition();
        }
    }
    // If no enemy is hit, keep the last locked target.
}

//////////////////////////////////////////////////////////////////////////
// Target Selection and Feedback

void UTurnBasedCombatComponent::SetSelectedEnemy(AActor* NewSelectedEnemy)
{
    // Remove feedback from previous enemy if different.
    if (SelectedEnemy && SelectedEnemy != NewSelectedEnemy)
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
    }

    SelectedEnemy = NewSelectedEnemy;

    if (!EnemyIndicatorWidgetClass)
        return;

    if (!CurrentEnemyIndicatorWidget)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            CurrentEnemyIndicatorWidget = CreateWidget<UUserWidget>(PC, EnemyIndicatorWidgetClass);
            if (CurrentEnemyIndicatorWidget)
            {
                CurrentEnemyIndicatorWidget->AddToViewport();
            }
        }
    }

    UpdateEnemyIndicatorPosition();
    ApplyFeedbackToEnemy(NewSelectedEnemy);
}

void UTurnBasedCombatComponent::UpdateEnemyIndicatorPosition()
{
    if (!SelectedEnemy || !CurrentEnemyIndicatorWidget)
        return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
        return;

    // Calculate world position with editable vertical offset.
    FVector WorldLocation = SelectedEnemy->GetActorLocation() + FVector(0.f, 0.f, IndicatorWorldVerticalOffset);
    FVector2D ScreenPosition;
    if (!PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition))
        return;

    // Apply editable screen offset.
    ScreenPosition += IndicatorScreenOffset;

    // Set widget pivot so that its bottom-center aligns with the position.
    CurrentEnemyIndicatorWidget->SetAlignmentInViewport(FVector2D(0.5f, 1.0f));
    CurrentEnemyIndicatorWidget->SetPositionInViewport(ScreenPosition, false);
}

void UTurnBasedCombatComponent::OnPlayerAttack()
{
    UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack called."));

    if (!bIsSelectingTarget)
    {
        bIsSelectingTarget = true;
        bTargetLocked = false;
        UE_LOG(LogTemp, Log, TEXT("Target selection mode activated. Hover over an enemy to lock the target."));
        return;
    }

    if (bIsSelectingTarget && !bTargetLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("No target locked. Hover over an enemy to lock the target, then press Attack to validate."));
        return;
    }

    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player actor not found."));
        return;
    }

    if (UAllyAbilityComponent* AllyAbility = PlayerActor->FindComponentByClass<UAllyAbilityComponent>())
    {
        float DamageAmount = AllyAbility->ExecuteDefaultAttack();
        if (UStatComponent* EnemyStat = SelectedEnemy->FindComponentByClass<UStatComponent>())
        {
            EnemyStat->ApplyDamage(DamageAmount, false);
            UE_LOG(LogTemp, Log, TEXT("Player attacked enemy %s for %f damage. New HP: %f/%f"),
                *SelectedEnemy->GetName(), DamageAmount, EnemyStat->Health, EnemyStat->MaxHealth);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player has no AllyAbilityComponent."));
    }

    if (SelectedEnemy)
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
    }

    bIsSelectingTarget = false;
    bTargetLocked = false;
    SelectedEnemy = nullptr;
    if (CurrentEnemyIndicatorWidget)
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
    }
    NextTurn();
}

void UTurnBasedCombatComponent::OnEnemyTurn()
{
    AActor* EnemyActor = Combatants.IsValidIndex(CurrentTurnIndex) ? Combatants[CurrentTurnIndex] : nullptr;
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!EnemyActor || !PlayerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy turn: invalid actor(s)."));
        NextTurn();
        return;
    }

    if (UEnemyAbilityComponent* AbilityComp = EnemyActor->FindComponentByClass<UEnemyAbilityComponent>())
    {
        AbilityComp->ExecuteDefaultAttack();
        if (UStatComponent* EnemyStat = EnemyActor->FindComponentByClass<UStatComponent>())
        {
            float DamageValue = EnemyStat->PhysicalAttack;
            UE_LOG(LogTemp, Log, TEXT("Enemy %s attacked the player for %f damage."), *EnemyActor->GetName(), DamageValue);
        }
        if (UStatComponent* PlayerStat = PlayerActor->FindComponentByClass<UStatComponent>())
        {
            UE_LOG(LogTemp, Log, TEXT("Player HP after attack: %f/%f"), PlayerStat->Health, PlayerStat->MaxHealth);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s has no EnemyAbilityComponent. No attack executed."), *EnemyActor->GetName());
    }

    NextTurn();
}

void UTurnBasedCombatComponent::NextTurn()
{
    bIsSelectingTarget = false;
    bTargetLocked = false;
    if (SelectedEnemy)
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
        SelectedEnemy = nullptr;
    }
    if (CurrentEnemyIndicatorWidget)
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
    }
    if (Combatants.IsValidIndex(CurrentTurnIndex))
    {
        Combatants.RemoveAt(CurrentTurnIndex);
    }
    if (Combatants.Num() == 0)
    {
        EndRound();
        return;
    }
    CurrentTurnIndex = 0;
    UpdateTurnOrderHUD();

    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Combatants.IsValidIndex(CurrentTurnIndex))
    {
        EndRound();
        return;
    }
    if (Combatants[CurrentTurnIndex] == PlayerActor)
    {
        if (PlayerTurnMenuWidget)
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Visible);
        }
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->EnableCombatInputMode();
        }
    }
    else
    {
        if (PlayerTurnMenuWidget)
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        }
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->DisableCombatInputMode();
        }
        OnEnemyTurn();
    }
}

void UTurnBasedCombatComponent::EndRound()
{
    ACharacter* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    TArray<AActor*> NewCombatants;
    TArray<AActor*> AliveEnemies;

    if (PlayerActor)
    {
        if (UStatComponent* StatComp = PlayerActor->FindComponentByClass<UStatComponent>())
        {
            if (StatComp->Health > 0)
            {
                NewCombatants.Add(PlayerActor);
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("Player is defeated."));
                UGameplayStatics::OpenLevel(GetWorld(), OriginalMapName);
                return;
            }
        }
    }

    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FoundEnemies);
    for (AActor* Enemy : FoundEnemies)
    {
        if (Enemy)
        {
            if (UStatComponent* StatComp = Enemy->FindComponentByClass<UStatComponent>())
            {
                if (StatComp->Health > 0)
                {
                    NewCombatants.Add(Enemy);
                    AliveEnemies.Add(Enemy);
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("Enemy %s is defeated."), *Enemy->GetName());
                    Enemy->Destroy();
                }
            }
        }
    }

    if (bPlayerFled)
    {
        UE_LOG(LogTemp, Log, TEXT("Player fled. Returning to base level without destroying EnemyTrigger."));
        UGameplayStatics::OpenLevel(GetWorld(), OriginalMapName);
        return;
    }

    if (AliveEnemies.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("All enemies are defeated. Combat finished."));
        FName MapToLoad = OriginalMapName;
        if (!GIsPlayInEditorWorld)
        {
            FCoreUObjectDelegates::PostLoadMapWithWorld.AddLambda([MapToLoad](UWorld* LoadedWorld)
                {
                    if (LoadedWorld && LoadedWorld->GetName() == MapToLoad.ToString())
                    {
                        TArray<AActor*> Triggers;
                        UGameplayStatics::GetAllActorsWithTag(LoadedWorld, FName("EnemyTrigger"), Triggers);
                        for (AActor* Trigger : Triggers)
                        {
                            Trigger->Destroy();
                        }
                    }
                });
        }
        UGameplayStatics::OpenLevel(GetWorld(), MapToLoad);
        return;
    }

    Combatants = NewCombatants;
    CurrentTurnIndex = 0;
    UpdateTurnOrderHUD();

    if (Combatants[0] == PlayerActor)
    {
        if (PlayerTurnMenuWidget)
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Visible);
        }
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->EnableCombatInputMode();
        }
    }
    else
    {
        if (PlayerTurnMenuWidget)
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        }
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->DisableCombatInputMode();
        }
        OnEnemyTurn();
    }
}

void UTurnBasedCombatComponent::UpdateTurnOrderHUD()
{
    if (!TurnOrderWidget)
        return;

    TArray<FCombatantTurnInfo> TurnInfos;
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    for (AActor* Actor : Combatants)
    {
        if (!Actor) continue;
        if (UStatComponent* StatComp = Actor->FindComponentByClass<UStatComponent>())
        {
            FCombatantTurnInfo Info;
            Info.Combatant = Actor;
            Info.Speed = StatComp->Speed;
            // Use the selected icon texture for the selected enemy.
            if (Actor == SelectedEnemy && SelectedIconTexture)
            {
                Info.Icon = SelectedIconTexture;
            }
            else
            {
                Info.Icon = (Actor == PlayerActor) ? PlayerIconTexture : EnemyIconTexture;
            }
            TurnInfos.Add(Info);
        }
    }

    TurnOrderWidget->UpdateTurnOrder(TurnInfos, SelectedEnemy);
}

void UTurnBasedCombatComponent::OnPlayerFlee()
{
    bPlayerFled = true;
    if (PlayerTurnMenuWidget)
    {
        PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
    }

    if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
    {
        PC->DisableCombatInputMode();
    }

    UGameplayStatics::OpenLevel(GetWorld(), OriginalMapName);
}

//////////////////////////////////////////////////////////////////////////
// Feedback Helper Functions

void UTurnBasedCombatComponent::ApplyFeedbackToEnemy(AActor* Enemy)
{
    if (!Enemy || !SelectedEnemyLightFunctionMaterial)
        return;

    // Attempt to retrieve the enemy's mesh: try SkeletalMeshComponent first, then StaticMeshComponent.
    UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    if (!Mesh)
    {
        Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    }
    if (!Mesh)
        return;

    // Store the original material if not already stored.
    if (!OriginalMaterials.Contains(Enemy))
    {
        UMaterialInterface* OrigMat = Mesh->GetMaterial(0);
        OriginalMaterials.Add(Enemy, OrigMat);
    }

    // Apply the flashing (light function) material.
    Mesh->SetMaterial(0, SelectedEnemyLightFunctionMaterial);
}

void UTurnBasedCombatComponent::RemoveFeedbackFromEnemy(AActor* Enemy)
{
    if (!Enemy)
        return;

    UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    if (!Mesh)
    {
        Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    }
    if (!Mesh)
        return;

    if (OriginalMaterials.Contains(Enemy))
    {
        Mesh->SetMaterial(0, OriginalMaterials[Enemy]);
        OriginalMaterials.Remove(Enemy);
    }
}
