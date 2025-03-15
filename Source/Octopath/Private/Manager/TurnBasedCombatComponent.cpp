#include "Manager/TurnBasedCombatComponent.h"
#include "Manager/StatComponent.h"
#include "Enemy/EnemyAbilityComponent.h"
#include "Character/AllyAbilityComponent.h"

#include "Blueprint/UserWidget.h"
#include "Widget/TurnOrderWidget.h"
#include "Widget/PlayerTurnMenuWidget.h"
#include "Widget/PlayerStatsWidget.h"
#include "Widget/EnemyIndicatorWidget.h"

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
    bPlayerDefendedThisRound = false;
    bDefenseConsumed = false;
}

//////////////////////////////////////////////////////////////////////////
// BeginPlay and Combat Flow

void UTurnBasedCombatComponent::BeginPlay()
{
    UE_LOG(LogTemp, Log, TEXT("TurnBasedCombatComponent::BeginPlay - Called"));
    Super::BeginPlay();

    // Assemble combatants: add the player and all enemies tagged "Enemy".
    if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        Combatants.Add(PlayerCharacter);
        UE_LOG(LogTemp, Log, TEXT("BeginPlay - Added player %s to Combatants"), *PlayerCharacter->GetName());
    }
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FoundEnemies);
    for (AActor* Enemy : FoundEnemies)
    {
        Combatants.Add(Enemy);
        UE_LOG(LogTemp, Log, TEXT("BeginPlay - Added enemy %s to Combatants"), *Enemy->GetName());
    }

    // Create and add the Turn Order widget to the viewport.
    if (TurnOrderWidgetClass)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            TurnOrderWidget = CreateWidget<UTurnOrderWidget>(PC, TurnOrderWidgetClass);
            if (TurnOrderWidget)
            {
                TurnOrderWidget->AddToViewport();
                UE_LOG(LogTemp, Log, TEXT("BeginPlay - TurnOrderWidget created and added to viewport"));
            }
        }
    }

    // Create and add the Player Stats widget to the viewport.
    if (PlayerStatsWidgetClass)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PlayerStatsWidget = CreateWidget<UPlayerStatsWidget>(PC, PlayerStatsWidgetClass);
            if (PlayerStatsWidget)
            {
                PlayerStatsWidget->AddToViewport();
                UE_LOG(LogTemp, Log, TEXT("BeginPlay - PlayerStatsWidget created and added to viewport"));
            }
        }
    }

    StartCombat();
    UE_LOG(LogTemp, Log, TEXT("TurnBasedCombatComponent::BeginPlay - End"));
}

void UTurnBasedCombatComponent::StartCombat()
{
    UE_LOG(LogTemp, Log, TEXT("StartCombat - Called"));
    bPlayerFled = false;
    CurrentTurnIndex = 0;

    // Get the player actor.
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    TArray<FCombatantTurnInfo> TurnInfos;

    // Build turn info for each combatant based on their current Speed.
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
            UE_LOG(LogTemp, Log, TEXT("StartCombat - Added turn info for %s with Speed: %f"), *Actor->GetName(), Info.Speed);
        }
    }

    // Sort combatants in descending order based on Speed.
    TurnInfos.Sort([](const FCombatantTurnInfo& A, const FCombatantTurnInfo& B)
        {
            return A.Speed > B.Speed;
        });

    // Update Combatants array with sorted actors.
    Combatants.Empty();
    for (const FCombatantTurnInfo& Info : TurnInfos)
    {
        Combatants.Add(Info.Combatant);
        UE_LOG(LogTemp, Log, TEXT("StartCombat - Sorted Combatant: %s"), *Info.Combatant->GetName());
    }

    // Update the Turn Order HUD.
    if (TurnOrderWidget)
    {
        TurnOrderWidget->UpdateTurnOrder(CurrentRoundInfos, FullTurnInfos, SelectedEnemy);
        UE_LOG(LogTemp, Log, TEXT("StartCombat - TurnOrderWidget updated"));
    }

    // Check if it is the player's turn.
    if (Combatants.IsValidIndex(CurrentTurnIndex) && Combatants[CurrentTurnIndex] == PlayerActor)
    {
        // Disable player movement.
        if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerActor))
        {
            PlayerChar->GetCharacterMovement()->DisableMovement();
            UE_LOG(LogTemp, Log, TEXT("StartCombat - Player movement disabled"));
        }

        // Create and display the player's action menu.
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
                        PlayerTurnMenuWidget->OnDefenseSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerDefense);
                        PlayerTurnMenuWidget->OnFleeSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerFlee);
                        UE_LOG(LogTemp, Log, TEXT("StartCombat - PlayerTurnMenuWidget created and events bound"));
                    }
                }
                else
                {
                    PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Visible);
                    UE_LOG(LogTemp, Log, TEXT("StartCombat - PlayerTurnMenuWidget set to visible"));
                }
            }
        }

        // Enable combat input for the player.
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->EnableCombatInputMode();
            UE_LOG(LogTemp, Log, TEXT("StartCombat - Combat input mode enabled for player"));
        }

        bIsSelectingTarget = false;
        bTargetLocked = false;
        UE_LOG(LogTemp, Log, TEXT("StartCombat - Player's turn setup complete"));
    }
    else
    {
        // Enemy's turn: hide player's menu and disable input.
        if (PlayerTurnMenuWidget)
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
            UE_LOG(LogTemp, Log, TEXT("StartCombat - PlayerTurnMenuWidget hidden for enemy turn"));
        }
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->DisableCombatInputMode();
            UE_LOG(LogTemp, Log, TEXT("StartCombat - Combat input mode disabled for player"));
        }
        OnEnemyTurn();
    }
    UE_LOG(LogTemp, Log, TEXT("StartCombat - End"));
}

void UTurnBasedCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // In target selection mode, update target based on the mouse cursor.
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
                UE_LOG(LogTemp, Log, TEXT("TickComponent - Removed feedback from previous enemy: %s"), *SelectedEnemy->GetName());
            }
            SetSelectedEnemy(HitActor);
            bTargetLocked = true;
            UE_LOG(LogTemp, Log, TEXT("TickComponent - Target locked on enemy: %s"), *HitActor->GetName());
        }
        else
        {
            UpdateEnemyIndicatorPosition();
            UE_LOG(LogTemp, Log, TEXT("TickComponent - Updated enemy indicator position for: %s"), *HitActor->GetName());
        }
    }
    // If no enemy is hit, retain the current target.
}

//////////////////////////////////////////////////////////////////////////
// Target Selection and Feedback

void UTurnBasedCombatComponent::SetSelectedEnemy(AActor* NewSelectedEnemy)
{
    UE_LOG(LogTemp, Log, TEXT("SetSelectedEnemy - Called with enemy: %s"), NewSelectedEnemy ? *NewSelectedEnemy->GetName() : TEXT("None"));

    // Remove feedback from previous enemy if different.
    if (SelectedEnemy && SelectedEnemy != NewSelectedEnemy)
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
        UE_LOG(LogTemp, Log, TEXT("SetSelectedEnemy - Removed feedback from previous enemy: %s"), *SelectedEnemy->GetName());
    }

    SelectedEnemy = NewSelectedEnemy;

    if (!EnemyIndicatorWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetSelectedEnemy - EnemyIndicatorWidgetClass is not set"));
        return;
    }

    // Create enemy indicator widget if not already created.
    if (!CurrentEnemyIndicatorWidget)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            CurrentEnemyIndicatorWidget = CreateWidget<UEnemyIndicatorWidget>(PC, EnemyIndicatorWidgetClass);
            if (CurrentEnemyIndicatorWidget)
            {
                CurrentEnemyIndicatorWidget->AddToViewport();
                UE_LOG(LogTemp, Log, TEXT("SetSelectedEnemy - EnemyIndicatorWidget created and added to viewport"));
            }
        }
    }

    UpdateEnemyIndicatorPosition();
    ApplyFeedbackToEnemy(NewSelectedEnemy);

    // Update enemy name in the indicator widget.
    if (CurrentEnemyIndicatorWidget && NewSelectedEnemy)
    {
        if (UStatComponent* StatComp = NewSelectedEnemy->FindComponentByClass<UStatComponent>())
        {
            CurrentEnemyIndicatorWidget->SetEnemyName(StatComp->EntityName);
            UE_LOG(LogTemp, Log, TEXT("SetSelectedEnemy - Updated enemy indicator name: %s"), *StatComp->EntityName.ToString());
        }
    }
}

void UTurnBasedCombatComponent::UpdateEnemyIndicatorPosition()
{
    UE_LOG(LogTemp, Log, TEXT("UpdateEnemyIndicatorPosition - Called"));
    if (!SelectedEnemy || !CurrentEnemyIndicatorWidget)
        return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
        return;

    // Calculate world position with a vertical offset.
    FVector WorldLocation = SelectedEnemy->GetActorLocation() + FVector(0.f, 0.f, IndicatorWorldVerticalOffset);
    FVector2D ScreenPosition;
    if (!PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition))
        return;

    // Apply an additional screen offset.
    ScreenPosition += IndicatorScreenOffset;

    // Set widget alignment and position.
    CurrentEnemyIndicatorWidget->SetAlignmentInViewport(FVector2D(0.5f, 1.0f));
    CurrentEnemyIndicatorWidget->SetPositionInViewport(ScreenPosition, false);
    UE_LOG(LogTemp, Log, TEXT("UpdateEnemyIndicatorPosition - Updated enemy indicator at screen position: X=%f, Y=%f"), ScreenPosition.X, ScreenPosition.Y);
}

void UTurnBasedCombatComponent::OnPlayerAttack()
{
    UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Called"));

    // Activate target selection if not already active.
    if (!bIsSelectingTarget)
    {
        bIsSelectingTarget = true;
        bTargetLocked = false;
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Target selection mode activated"));
        return;
    }

    // Warn if target selection mode is active but no target is locked.
    if (bIsSelectingTarget && !bTargetLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - No target locked. Please hover over an enemy to lock target."));
        return;
    }

    // Retrieve player actor.
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - Player actor not found"));
        return;
    }

    // Execute the player's attack via the AllyAbilityComponent.
    if (UAllyAbilityComponent* AllyAbility = PlayerActor->FindComponentByClass<UAllyAbilityComponent>())
    {
        float DamageAmount = AllyAbility->ExecuteDefaultAttack();
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Player attack damage: %f"), DamageAmount);
        if (UStatComponent* EnemyStat = SelectedEnemy->FindComponentByClass<UStatComponent>())
        {
            EnemyStat->ApplyDamage(DamageAmount, false);
            UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Applied damage to enemy %s. New HP: %f/%f"), *SelectedEnemy->GetName(), EnemyStat->Health, EnemyStat->MaxHealth);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - Player has no AllyAbilityComponent"));
    }

    // Clear target selection and remove enemy feedback.
    if (SelectedEnemy)
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Removed enemy feedback for: %s"), *SelectedEnemy->GetName());
    }
    bIsSelectingTarget = false;
    bTargetLocked = false;
    SelectedEnemy = nullptr;
    if (CurrentEnemyIndicatorWidget)
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Enemy indicator widget removed"));
    }
    NextTurn();
    UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - End"));
}

void UTurnBasedCombatComponent::OnPlayerDefense()
{
    UE_LOG(LogTemp, Log, TEXT("OnPlayerDefense - Called"));

    static int32 DefenseInvocationCount = 0;
    DefenseInvocationCount++;
    UE_LOG(LogTemp, Log, TEXT("OnPlayerDefense - Invocation count: %d"), DefenseInvocationCount);

    // Retrieve player actor.
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerDefense - Player actor not found"));
        return;
    }

    // Activate defense flag on the player's StatComponent.
    if (UStatComponent* StatComp = PlayerActor->FindComponentByClass<UStatComponent>())
    {
        StatComp->bIsDefending = true;
        UE_LOG(LogTemp, Log, TEXT("OnPlayerDefense - Player is defending (bIsDefending activated)"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerDefense - Player StatComponent not found"));
    }

    // Set local defense flags.
    bPlayerDefendedThisRound = true;
    bDefenseConsumed = false;

    // Hide the player's action menu.
    if (PlayerTurnMenuWidget)
    {
        PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Log, TEXT("OnPlayerDefense - PlayerTurnMenuWidget hidden"));
    }
    NextTurn();
    UE_LOG(LogTemp, Log, TEXT("OnPlayerDefense - End"));
}

void UTurnBasedCombatComponent::OnEnemyTurn()
{
    UE_LOG(LogTemp, Log, TEXT("OnEnemyTurn - Called"));

    // Get the enemy actor based on the current turn index.
    AActor* EnemyActor = Combatants.IsValidIndex(CurrentTurnIndex) ? Combatants[CurrentTurnIndex] : nullptr;
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!EnemyActor || !PlayerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnEnemyTurn - Invalid enemy or player actor"));
        NextTurn();
        return;
    }

    float DamageValue = 0.f;
    // Calculate enemy's attack damage using its EnemyAbilityComponent.
    if (UEnemyAbilityComponent* AbilityComp = EnemyActor->FindComponentByClass<UEnemyAbilityComponent>())
    {
        DamageValue = AbilityComp->ExecuteDefaultAttack();
        UE_LOG(LogTemp, Log, TEXT("OnEnemyTurn - Enemy %s calculated attack damage: %f"), *EnemyActor->GetName(), DamageValue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnEnemyTurn - Enemy %s has no EnemyAbilityComponent"), *EnemyActor->GetName());
    }

    // Retrieve the player's StatComponent and apply the damage.
    if (PlayerActor)
    {
        if (UStatComponent* PlayerStat = PlayerActor->FindComponentByClass<UStatComponent>())
        {
            // The ApplyDamage function will reduce damage if bIsDefending is true.
            PlayerStat->ApplyDamage(DamageValue, false);
            UE_LOG(LogTemp, Log, TEXT("OnEnemyTurn - Damage applied. Player HP after attack: %f/%f"), PlayerStat->Health, PlayerStat->MaxHealth);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("OnEnemyTurn - Player StatComponent not found"));
        }
    }
    UE_LOG(LogTemp, Log, TEXT("OnEnemyTurn - End"));
    NextTurn();
}

void UTurnBasedCombatComponent::NextTurn()
{
    UE_LOG(LogTemp, Log, TEXT("NextTurn - Called. CurrentTurnIndex: %d, Total Combatants: %d"), CurrentTurnIndex, Combatants.Num());

    // Reset target selection variables.
    bIsSelectingTarget = false;
    bTargetLocked = false;

    if (SelectedEnemy)
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
        UE_LOG(LogTemp, Log, TEXT("NextTurn - Removed feedback from selected enemy: %s"), *SelectedEnemy->GetName());
        SelectedEnemy = nullptr;
    }

    if (CurrentEnemyIndicatorWidget)
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("NextTurn - Removed enemy indicator widget"));
    }

    // Increment the turn index.
    CurrentTurnIndex++;
    UE_LOG(LogTemp, Log, TEXT("NextTurn - Incremented turn index to: %d"), CurrentTurnIndex);

    // If all combatants have acted, end the round.
    if (CurrentTurnIndex >= Combatants.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("NextTurn - All combatants have acted. Calling EndRound."));
        EndRound();
        return;
    }

    // Dynamically update the remaining combatants based on current Speed.
    {
        TArray<AActor*> RemainingCombatants;
        for (int32 i = CurrentTurnIndex; i < Combatants.Num(); i++)
        {
            RemainingCombatants.Add(Combatants[i]);
        }
        RemainingCombatants.Sort([](const AActor& A, const AActor& B)
            {
                float SpeedA = 0.f, SpeedB = 0.f;
                if (const UStatComponent* StatA = A.FindComponentByClass<UStatComponent>())
                    SpeedA = StatA->Speed;
                if (const UStatComponent* StatB = B.FindComponentByClass<UStatComponent>())
                    SpeedB = StatB->Speed;
                return SpeedA > SpeedB;
            });
        // Update Combatants array for the remaining turns.
        for (int32 i = CurrentTurnIndex; i < Combatants.Num(); i++)
        {
            Combatants[i] = RemainingCombatants[i - CurrentTurnIndex];
            UE_LOG(LogTemp, Log, TEXT("NextTurn - Updated remaining combatant: %s"), *Combatants[i]->GetName());
        }
    }

    // Update the turn order HUD.
    UpdateTurnOrderHUD();

    // Retrieve the player actor.
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    // If it's the player's turn, show the player's turn menu.
    if (Combatants[CurrentTurnIndex] == PlayerActor)
    {
        if (!PlayerTurnMenuWidget)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                PlayerTurnMenuWidget = CreateWidget<UPlayerTurnMenuWidget>(PC, PlayerTurnMenuWidgetClass);
                if (PlayerTurnMenuWidget)
                {
                    PlayerTurnMenuWidget->AddToViewport();
                    PlayerTurnMenuWidget->OnAttackSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerAttack);
                    PlayerTurnMenuWidget->OnDefenseSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerDefense);
                    PlayerTurnMenuWidget->OnFleeSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerFlee);
                    UE_LOG(LogTemp, Log, TEXT("NextTurn - PlayerTurnMenuWidget created and events bound"));
                }
            }
        }
        else
        {
            if (!PlayerTurnMenuWidget->IsInViewport())
            {
                PlayerTurnMenuWidget->AddToViewport();
                UE_LOG(LogTemp, Log, TEXT("NextTurn - PlayerTurnMenuWidget added to viewport"));
            }
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Visible);
            UE_LOG(LogTemp, Log, TEXT("NextTurn - PlayerTurnMenuWidget set to visible"));
        }
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->EnableCombatInputMode();
            UE_LOG(LogTemp, Log, TEXT("NextTurn - Combat input mode enabled for player"));
        }
    }
    else // Enemy's turn.
    {
        if (PlayerTurnMenuWidget)
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
            UE_LOG(LogTemp, Log, TEXT("NextTurn - PlayerTurnMenuWidget hidden for enemy turn"));
        }
        if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->DisableCombatInputMode();
            UE_LOG(LogTemp, Log, TEXT("NextTurn - Combat input mode disabled for player"));
        }
        UE_LOG(LogTemp, Log, TEXT("NextTurn - Enemy turn: Calling OnEnemyTurn for combatant: %s"), *Combatants[CurrentTurnIndex]->GetName());
        OnEnemyTurn();
    }

    UE_LOG(LogTemp, Log, TEXT("NextTurn - End"));
}

void UTurnBasedCombatComponent::EndRound()
{
    UE_LOG(LogTemp, Log, TEXT("EndRound - Called"));
    ACharacter* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    TArray<AActor*> NewCombatants;
    TArray<AActor*> AliveEnemies;

    // Check if the player is alive.
    bool bPlayerAlive = false;
    if (PlayerActor)
    {
        if (UStatComponent* StatComp = PlayerActor->FindComponentByClass<UStatComponent>())
        {
            if (StatComp->Health > 0)
            {
                bPlayerAlive = true;
                UE_LOG(LogTemp, Log, TEXT("EndRound - Player is alive"));
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("EndRound - Player is defeated"));
                UGameplayStatics::OpenLevel(GetWorld(), OriginalMapName);
                return;
            }
        }
    }

    // Gather all living enemies.
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
                    UE_LOG(LogTemp, Log, TEXT("EndRound - Enemy %s is alive"), *Enemy->GetName());
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("EndRound - Enemy %s is defeated"), *Enemy->GetName());
                    Enemy->Destroy();
                }
            }
        }
    }

    if (bPlayerFled)
    {
        UE_LOG(LogTemp, Log, TEXT("EndRound - Player fled. Returning to base level."));
        UGameplayStatics::OpenLevel(GetWorld(), OriginalMapName);
        return;
    }

    if (AliveEnemies.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("EndRound - All enemies defeated. Combat finished."));
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

    // If the player is alive, add the player to the new combatants list.
    if (bPlayerAlive && PlayerActor)
    {
        if (bPlayerDefendedThisRound)
        {
            // If the player defended, let the player start the next round.
            TArray<AActor*> SortedEnemies;
            for (AActor* Actor : NewCombatants)
            {
                if (Actor != PlayerActor)
                {
                    SortedEnemies.Add(Actor);
                }
            }
            SortedEnemies.Sort([](const AActor& A, const AActor& B)
                {
                    float SpeedA = 0.f, SpeedB = 0.f;
                    if (const UStatComponent* StatA = A.FindComponentByClass<UStatComponent>())
                        SpeedA = StatA->Speed;
                    if (const UStatComponent* StatB = B.FindComponentByClass<UStatComponent>())
                        SpeedB = StatB->Speed;
                    return SpeedA > SpeedB;
                });
            NewCombatants.Empty();
            NewCombatants.Add(PlayerActor);
            NewCombatants.Append(SortedEnemies);
            UE_LOG(LogTemp, Log, TEXT("EndRound - Player defended. New combatants order set with player first."));
        }
        else
        {
            NewCombatants.Add(PlayerActor);
            NewCombatants.Sort([](const AActor& A, const AActor& B)
                {
                    float SpeedA = 0.f, SpeedB = 0.f;
                    if (const UStatComponent* StatA = A.FindComponentByClass<UStatComponent>())
                        SpeedA = StatA->Speed;
                    if (const UStatComponent* StatB = B.FindComponentByClass<UStatComponent>())
                        SpeedB = StatB->Speed;
                    return SpeedA > SpeedB;
                });
            UE_LOG(LogTemp, Log, TEXT("EndRound - New combatants order set with player included."));
        }
    }

    // Update full turn order for the next round.
    Combatants = NewCombatants;
    CurrentTurnIndex = 0;
    UpdateTurnOrderHUD();
    UE_LOG(LogTemp, Log, TEXT("EndRound - Turn order HUD updated"));

    // If the first combatant is the player, show the player's menu; otherwise, trigger enemy turn.
    if (Combatants.Num() > 0)
    {
        if (Combatants[0] == PlayerActor)
        {
            if (PlayerTurnMenuWidget)
            {
                PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Visible);
                UE_LOG(LogTemp, Log, TEXT("EndRound - PlayerTurnMenuWidget set to visible"));
            }
            if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
            {
                PC->EnableCombatInputMode();
                UE_LOG(LogTemp, Log, TEXT("EndRound - Combat input mode enabled for player"));
            }
        }
        else
        {
            if (PlayerTurnMenuWidget)
            {
                PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
                UE_LOG(LogTemp, Log, TEXT("EndRound - PlayerTurnMenuWidget hidden for enemy turn"));
            }
            if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
            {
                PC->DisableCombatInputMode();
                UE_LOG(LogTemp, Log, TEXT("EndRound - Combat input mode disabled for player"));
            }
            OnEnemyTurn();
        }
    }

    if (UStatComponent* PlayerStat = PlayerActor->FindComponentByClass<UStatComponent>())
    {
        PlayerStat->bIsDefending = false;
        UE_LOG(LogTemp, Log, TEXT("EndRound - Player defense bonus cleared."));
    }

    // Reset defense flags for the next round.
    bPlayerDefendedThisRound = false;
    bDefenseConsumed = false;

    UE_LOG(LogTemp, Log, TEXT("EndRound - End"));
}

void UTurnBasedCombatComponent::UpdateTurnOrderHUD()
{
    UE_LOG(LogTemp, Log, TEXT("UpdateTurnOrderHUD - Called"));
    if (!TurnOrderWidget)
        return;

    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    // Clear existing turn info arrays.
    FullTurnInfos.Empty();
    CurrentRoundInfos.Empty();

    // Build FullTurnInfos from all combatants.
    for (AActor* Actor : Combatants)
    {
        if (!Actor)
            continue;
        if (UStatComponent* StatComp = Actor->FindComponentByClass<UStatComponent>())
        {
            FCombatantTurnInfo Info;
            Info.Combatant = Actor;
            Info.Speed = StatComp->Speed;
            if (Actor == SelectedEnemy && SelectedIconTexture)
            {
                Info.Icon = SelectedIconTexture;
            }
            else
            {
                Info.Icon = (Actor == PlayerActor) ? PlayerIconTexture : EnemyIconTexture;
            }
            FullTurnInfos.Add(Info);
        }
    }

    // Populate CurrentRoundInfos with combatants who have not yet acted.
    if (CurrentTurnIndex < FullTurnInfos.Num())
    {
        for (int32 i = CurrentTurnIndex; i < FullTurnInfos.Num(); i++)
        {
            CurrentRoundInfos.Add(FullTurnInfos[i]);
        }
    }

    // Update the TurnOrderWidget with current and full turn info arrays.
    TurnOrderWidget->UpdateTurnOrder(CurrentRoundInfos, FullTurnInfos, SelectedEnemy);
    UE_LOG(LogTemp, Log, TEXT("UpdateTurnOrderHUD - TurnOrderWidget updated"));
}

void UTurnBasedCombatComponent::OnPlayerFlee()
{
    UE_LOG(LogTemp, Log, TEXT("OnPlayerFlee - Called"));
    bPlayerFled = true;
    if (PlayerTurnMenuWidget)
    {
        PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Log, TEXT("OnPlayerFlee - PlayerTurnMenuWidget hidden"));
    }
    if (AHikariPlayerController* PC = Cast<AHikariPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
    {
        PC->DisableCombatInputMode();
        UE_LOG(LogTemp, Log, TEXT("OnPlayerFlee - Combat input mode disabled for player"));
    }
    UGameplayStatics::OpenLevel(GetWorld(), OriginalMapName);
    UE_LOG(LogTemp, Log, TEXT("OnPlayerFlee - Level change triggered"));
}

//////////////////////////////////////////////////////////////////////////
// Feedback Helper Functions

void UTurnBasedCombatComponent::ApplyFeedbackToEnemy(AActor* Enemy)
{
    UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEnemy - Called for enemy: %s"), Enemy ? *Enemy->GetName() : TEXT("None"));
    if (!Enemy || !SelectedEnemyLightFunctionMaterial)
        return;

    // Try to get the enemy's mesh: check SkeletalMeshComponent first, then StaticMeshComponent.
    UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    if (!Mesh)
    {
        Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    }
    if (!Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyFeedbackToEnemy - Mesh not found for enemy: %s"), *Enemy->GetName());
        return;
    }

    // Store the original material if not already stored.
    if (!OriginalMaterials.Contains(Enemy))
    {
        UMaterialInterface* OrigMat = Mesh->GetMaterial(0);
        OriginalMaterials.Add(Enemy, OrigMat);
        UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEnemy - Original material stored for enemy: %s"), *Enemy->GetName());
    }

    // Apply the flashing material.
    Mesh->SetMaterial(0, SelectedEnemyLightFunctionMaterial);
    UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEnemy - Applied selected enemy light function material to enemy: %s"), *Enemy->GetName());
}

void UTurnBasedCombatComponent::RemoveFeedbackFromEnemy(AActor* Enemy)
{
    UE_LOG(LogTemp, Log, TEXT("RemoveFeedbackFromEnemy - Called for enemy: %s"), Enemy ? *Enemy->GetName() : TEXT("None"));
    if (!Enemy)
        return;

    UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    if (!Mesh)
    {
        Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    }
    if (!Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveFeedbackFromEnemy - Mesh not found for enemy: %s"), *Enemy->GetName());
        return;
    }

    if (OriginalMaterials.Contains(Enemy))
    {
        Mesh->SetMaterial(0, OriginalMaterials[Enemy]);
        OriginalMaterials.Remove(Enemy);
        UE_LOG(LogTemp, Log, TEXT("RemoveFeedbackFromEnemy - Restored original material for enemy: %s"), *Enemy->GetName());
    }
}