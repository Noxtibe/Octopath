#include "Manager/TurnBasedCombatComponent.h"
#include "Manager/StatComponent.h"
#include "Enemy/EnemyAbilityComponent.h"
#include "Character/AllyAbilityComponent.h"

#include "Blueprint/UserWidget.h"
#include "Widget/TurnOrderWidget.h"
#include "Widget/PlayerTurnMenuWidget.h"
#include "Widget/PlayerStatsWidget.h"
#include "Widget/PlayerAbilitiesMenuWidget.h"
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

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Warning, TEXT("BeginPlay - World is not valid"));
        return;
    }

    // Cache Player Character
    AActor* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (IsValid(PlayerCharacter))
    {
        Combatants.Add(PlayerCharacter);
        UE_LOG(LogTemp, Log, TEXT("BeginPlay - Added player %s to Combatants"), *PlayerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BeginPlay - PlayerCharacter is not valid"));
    }

    // Add all enemies tagged "Enemy".
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), FoundEnemies);
    for (AActor* Enemy : FoundEnemies)
    {
        if (!IsValid(Enemy))
        {
            continue;
        }
        Combatants.Add(Enemy);
        UE_LOG(LogTemp, Log, TEXT("BeginPlay - Added enemy %s to Combatants"), *Enemy->GetName());
    }

    // Cache Player Controller
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (TurnOrderWidgetClass && IsValid(PC))
    {
        TurnOrderWidget = CreateWidget<UTurnOrderWidget>(PC, TurnOrderWidgetClass);
        if (IsValid(TurnOrderWidget))
        {
            TurnOrderWidget->AddToViewport();
            UE_LOG(LogTemp, Log, TEXT("BeginPlay - TurnOrderWidget created and added to viewport"));
        }
    }

    if (PlayerStatsWidgetClass && IsValid(PC))
    {
        PlayerStatsWidget = CreateWidget<UPlayerStatsWidget>(PC, PlayerStatsWidgetClass);
        if (IsValid(PlayerStatsWidget))
        {
            PlayerStatsWidget->AddToViewport();
            UE_LOG(LogTemp, Log, TEXT("BeginPlay - PlayerStatsWidget created and added to viewport"));
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

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Warning, TEXT("StartCombat - World is invalid"));
        return;
    }

    // Cache Player Actor and Controller
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!IsValid(PlayerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("StartCombat - Player actor is invalid"));
        return;
    }

    TArray<FCombatantTurnInfo> TurnInfos;
    for (AActor* Actor : Combatants)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
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

    Combatants.Empty();
    for (const FCombatantTurnInfo& Info : TurnInfos)
    {
        Combatants.Add(Info.Combatant);
        UE_LOG(LogTemp, Log, TEXT("StartCombat - Sorted Combatant: %s"), *Info.Combatant->GetName());
    }

    if (IsValid(TurnOrderWidget))
    {
        TurnOrderWidget->UpdateTurnOrder(CurrentRoundInfos, FullTurnInfos, SelectedEnemy);
        UE_LOG(LogTemp, Log, TEXT("StartCombat - TurnOrderWidget updated"));
    }

    // Check whose turn it is.
    if (Combatants.IsValidIndex(CurrentTurnIndex) && Combatants[CurrentTurnIndex] == PlayerActor)
    {
        if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerActor))
        {
            PlayerChar->GetCharacterMovement()->DisableMovement();
            UE_LOG(LogTemp, Log, TEXT("StartCombat - Player movement disabled"));
        }

        if (PlayerTurnMenuWidgetClass && IsValid(PC))
        {
            if (!IsValid(PlayerTurnMenuWidget))
            {
                PlayerTurnMenuWidget = CreateWidget<UPlayerTurnMenuWidget>(PC, PlayerTurnMenuWidgetClass);
                if (IsValid(PlayerTurnMenuWidget))
                {
                    PlayerTurnMenuWidget->AddToViewport();
                    PlayerTurnMenuWidget->OnAttackSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerAttack);
                    PlayerTurnMenuWidget->OnAbilitiesSelected.AddDynamic(this, &UTurnBasedCombatComponent::ShowAbilitiesMenu);
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

        if (AHikariPlayerController* HPC = Cast<AHikariPlayerController>(PC))
        {
            HPC->EnableCombatInputMode();
            UE_LOG(LogTemp, Log, TEXT("StartCombat - Combat input mode enabled for player"));
        }
        bIsSelectingTarget = false;
        bTargetLocked = false;
        UE_LOG(LogTemp, Log, TEXT("StartCombat - Player's turn setup complete"));
    }
    else
    {
        if (IsValid(PlayerTurnMenuWidget))
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
            UE_LOG(LogTemp, Log, TEXT("StartCombat - PlayerTurnMenuWidget hidden for enemy turn"));
        }
        if (AHikariPlayerController* HPC = Cast<AHikariPlayerController>(PC))
        {
            HPC->DisableCombatInputMode();
            UE_LOG(LogTemp, Log, TEXT("StartCombat - Combat input mode disabled for player"));
        }
        OnEnemyTurn();
    }
    UE_LOG(LogTemp, Log, TEXT("StartCombat - End"));
}

void UTurnBasedCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsSelectingTarget)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!IsValid(PC))
    {
        return;
    }

    FHitResult Hit;
    if (!PC->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit))
    {
        return;
    }

    AActor* HitActor = Hit.GetActor();
    if (!IsValid(HitActor) || !HitActor->ActorHasTag("Enemy"))
    {
        return;
    }

    if (SelectedEnemy != HitActor)
    {
        if (IsValid(SelectedEnemy))
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

//////////////////////////////////////////////////////////////////////////
// Target Selection and Feedback

void UTurnBasedCombatComponent::SetSelectedEnemy(AActor* NewSelectedEnemy)
{
    UE_LOG(LogTemp, Log, TEXT("SetSelectedEnemy - Called with enemy: %s"), IsValid(NewSelectedEnemy) ? *NewSelectedEnemy->GetName() : TEXT("None"));

    if (IsValid(SelectedEnemy) && SelectedEnemy != NewSelectedEnemy)
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
        UE_LOG(LogTemp, Log, TEXT("SetSelectedEnemy - Removed feedback from previous enemy: %s"), *SelectedEnemy->GetName());
    }

    SelectedEnemy = NewSelectedEnemy;

    if (!IsValid(EnemyIndicatorWidgetClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("SetSelectedEnemy - EnemyIndicatorWidgetClass is not set"));
        return;
    }

    UWorld* World = GetWorld();
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!IsValid(CurrentEnemyIndicatorWidget) && IsValid(PC))
    {
        CurrentEnemyIndicatorWidget = CreateWidget<UEnemyIndicatorWidget>(PC, EnemyIndicatorWidgetClass);
        if (IsValid(CurrentEnemyIndicatorWidget))
        {
            CurrentEnemyIndicatorWidget->AddToViewport();
            UE_LOG(LogTemp, Log, TEXT("SetSelectedEnemy - EnemyIndicatorWidget created and added to viewport"));
        }
    }

    UpdateEnemyIndicatorPosition();
    ApplyFeedbackToEnemy(NewSelectedEnemy);

    if (IsValid(CurrentEnemyIndicatorWidget) && IsValid(NewSelectedEnemy))
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
    if (!IsValid(SelectedEnemy) || !IsValid(CurrentEnemyIndicatorWidget))
    {
        return;
    }

    UWorld* World = GetWorld();
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!IsValid(PC))
    {
        return;
    }

    FVector WorldLocation = SelectedEnemy->GetActorLocation() + FVector(0.f, 0.f, IndicatorWorldVerticalOffset);
    FVector2D ScreenPosition;
    if (!PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition))
    {
        return;
    }

    ScreenPosition += IndicatorScreenOffset;

    CurrentEnemyIndicatorWidget->SetAlignmentInViewport(FVector2D(0.5f, 1.0f));
    CurrentEnemyIndicatorWidget->SetPositionInViewport(ScreenPosition, false);
    UE_LOG(LogTemp, Log, TEXT("UpdateEnemyIndicatorPosition - Updated enemy indicator at screen position: X=%f, Y=%f"), ScreenPosition.X, ScreenPosition.Y);
}

void UTurnBasedCombatComponent::OnPlayerAttack()
{
    UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Called"));

    if (!bIsSelectingTarget)
    {
        bIsSelectingTarget = true;
        bTargetLocked = false;
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Target selection mode activated"));
        return;
    }

    if (bIsSelectingTarget && !bTargetLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - No target locked. Please hover over an enemy to lock target."));
        return;
    }

    UWorld* World = GetWorld();
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!IsValid(PlayerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - Player actor not found"));
        return;
    }

    if (UAllyAbilityComponent* AllyAbility = PlayerActor->FindComponentByClass<UAllyAbilityComponent>())
    {
        float DamageAmount = AllyAbility->ExecuteDefaultAttack();
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Player attack damage: %f"), DamageAmount);
        if (IsValid(SelectedEnemy))
        {
            if (UStatComponent* EnemyStat = SelectedEnemy->FindComponentByClass<UStatComponent>())
            {
                EnemyStat->ApplyDamage(DamageAmount, false);
                UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Applied damage to enemy %s. New HP: %f/%f"),
                    *SelectedEnemy->GetName(), EnemyStat->Health, EnemyStat->MaxHealth);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - Player has no AllyAbilityComponent"));
    }

    if (IsValid(SelectedEnemy))
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Removed enemy feedback for: %s"), *SelectedEnemy->GetName());
    }
    bIsSelectingTarget = false;
    bTargetLocked = false;
    SelectedEnemy = nullptr;

    if (IsValid(CurrentEnemyIndicatorWidget))
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Enemy indicator widget removed"));
    }
    NextTurn();
    UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - End"));
}

void UTurnBasedCombatComponent::ShowAbilitiesMenu()
{
    UE_LOG(LogTemp, Log, TEXT("ShowAbilitiesMenu called"));

    // Hide the main action menu
    if (IsValid(PlayerTurnMenuWidget))
    {
        PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
    }

    // Create or show the abilities widget
    if (!IsValid(PlayerAbilitiesMenuWidget) && IsValid(GetWorld()))
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (IsValid(PC) && PlayerAbilitiesMenuWidgetClass)
        {
            PlayerAbilitiesMenuWidget = CreateWidget<UPlayerAbilitiesMenuWidget>(PC, PlayerAbilitiesMenuWidgetClass);
            if (IsValid(PlayerAbilitiesMenuWidget))
            {
                PlayerAbilitiesMenuWidget->AddToViewport();
                PlayerAbilitiesMenuWidget->PopulateAbilitiesMenu();
                UE_LOG(LogTemp, Log, TEXT("ShowAbilitiesMenu - PlayerAbilitiesMenuWidget created and added to viewport"));
            }
        }
    }
    else if (IsValid(PlayerAbilitiesMenuWidget))
    {
        PlayerAbilitiesMenuWidget->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Log, TEXT("ShowAbilitiesMenu - PlayerAbilitiesMenuWidget set to visible"));
    }
}

void UTurnBasedCombatComponent::OnPlayerDefense()
{
    UE_LOG(LogTemp, Log, TEXT("OnPlayerDefense - Called"));

    static int32 DefenseInvocationCount = 0;
    DefenseInvocationCount++;
    UE_LOG(LogTemp, Log, TEXT("OnPlayerDefense - Invocation count: %d"), DefenseInvocationCount);

    UWorld* World = GetWorld();
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!IsValid(PlayerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerDefense - Player actor not found"));
        return;
    }

    if (UStatComponent* StatComp = PlayerActor->FindComponentByClass<UStatComponent>())
    {
        StatComp->bIsDefending = true;
        UE_LOG(LogTemp, Log, TEXT("OnPlayerDefense - Player is defending (bIsDefending activated)"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerDefense - Player StatComponent not found"));
    }

    bPlayerDefendedThisRound = true;
    bDefenseConsumed = false;

    if (IsValid(PlayerTurnMenuWidget))
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

    UWorld* World = GetWorld();
    AActor* EnemyActor = Combatants.IsValidIndex(CurrentTurnIndex) ? Combatants[CurrentTurnIndex] : nullptr;
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!IsValid(EnemyActor) || !IsValid(PlayerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnEnemyTurn - Invalid enemy or player actor"));
        NextTurn();
        return;
    }

    float DamageValue = 0.f;
    if (UEnemyAbilityComponent* AbilityComp = EnemyActor->FindComponentByClass<UEnemyAbilityComponent>())
    {
        DamageValue = AbilityComp->ExecuteDefaultAttack();
        UE_LOG(LogTemp, Log, TEXT("OnEnemyTurn - Enemy %s calculated attack damage: %f"), *EnemyActor->GetName(), DamageValue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnEnemyTurn - Enemy %s has no EnemyAbilityComponent"), *EnemyActor->GetName());
    }

    if (IsValid(PlayerActor))
    {
        if (UStatComponent* PlayerStat = PlayerActor->FindComponentByClass<UStatComponent>())
        {
            PlayerStat->ApplyDamage(DamageValue, false);
            UE_LOG(LogTemp, Log, TEXT("OnEnemyTurn - Damage applied. Player HP after attack: %f/%f"),
                PlayerStat->Health, PlayerStat->MaxHealth);
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
    UE_LOG(LogTemp, Log, TEXT("NextTurn - Called. CurrentTurnIndex: %d, Total Combatants: %d"),
        CurrentTurnIndex, Combatants.Num());

    bIsSelectingTarget = false;
    bTargetLocked = false;

    if (IsValid(SelectedEnemy))
    {
        RemoveFeedbackFromEnemy(SelectedEnemy);
        UE_LOG(LogTemp, Log, TEXT("NextTurn - Removed feedback from selected enemy: %s"), *SelectedEnemy->GetName());
        SelectedEnemy = nullptr;
    }

    if (IsValid(CurrentEnemyIndicatorWidget))
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("NextTurn - Removed enemy indicator widget"));
    }

    ++CurrentTurnIndex;
    UE_LOG(LogTemp, Log, TEXT("NextTurn - Incremented turn index to: %d"), CurrentTurnIndex);

    if (CurrentTurnIndex >= Combatants.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("NextTurn - All combatants have acted. Calling EndRound."));
        EndRound();
        return;
    }

    {
        TArray<AActor*> RemainingCombatants;
        RemainingCombatants.Reserve(Combatants.Num() - CurrentTurnIndex);
        for (int32 i = CurrentTurnIndex; i < Combatants.Num(); i++)
        {
            if (IsValid(Combatants[i]))
            {
                RemainingCombatants.Add(Combatants[i]);
            }
        }

        RemainingCombatants.Sort([](const AActor& A, const AActor& B)
            {
                float SpeedA = 0.f, SpeedB = 0.f;
                if (const UStatComponent* StatA = A.FindComponentByClass<UStatComponent>())
                {
                    SpeedA = StatA->Speed;
                }
                if (const UStatComponent* StatB = B.FindComponentByClass<UStatComponent>())
                {
                    SpeedB = StatB->Speed;
                }
                return SpeedA > SpeedB;
            });

        for (int32 i = CurrentTurnIndex; i < Combatants.Num(); i++)
        {
            Combatants[i] = RemainingCombatants[i - CurrentTurnIndex];
            UE_LOG(LogTemp, Log, TEXT("NextTurn - Updated remaining combatant: %s"), *Combatants[i]->GetName());
        }
    }

    UpdateTurnOrderHUD();

    UWorld* World = GetWorld();
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!IsValid(PlayerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("NextTurn - Player actor not valid"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (Combatants[CurrentTurnIndex] == PlayerActor)
    {
        if (!IsValid(PlayerTurnMenuWidget) && IsValid(PC))
        {
            PlayerTurnMenuWidget = CreateWidget<UPlayerTurnMenuWidget>(PC, PlayerTurnMenuWidgetClass);
            if (IsValid(PlayerTurnMenuWidget))
            {
                PlayerTurnMenuWidget->AddToViewport();
                PlayerTurnMenuWidget->OnAttackSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerAttack);
                PlayerTurnMenuWidget->OnAbilitiesSelected.AddDynamic(this, &UTurnBasedCombatComponent::ShowAbilitiesMenu);
                PlayerTurnMenuWidget->OnDefenseSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerDefense);
                PlayerTurnMenuWidget->OnFleeSelected.AddDynamic(this, &UTurnBasedCombatComponent::OnPlayerFlee);
                UE_LOG(LogTemp, Log, TEXT("NextTurn - PlayerTurnMenuWidget created and events bound"));
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
        if (AHikariPlayerController* HPC = Cast<AHikariPlayerController>(PC))
        {
            HPC->EnableCombatInputMode();
            UE_LOG(LogTemp, Log, TEXT("NextTurn - Combat input mode enabled for player"));
        }
    }
    else // Enemy turn.
    {
        if (IsValid(PlayerTurnMenuWidget))
        {
            PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
            UE_LOG(LogTemp, Log, TEXT("NextTurn - PlayerTurnMenuWidget hidden for enemy turn"));
        }
        if (AHikariPlayerController* HPC = Cast<AHikariPlayerController>(PC))
        {
            HPC->DisableCombatInputMode();
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

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Warning, TEXT("EndRound - World is invalid"));
        return;
    }

    ACharacter* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
    TArray<AActor*> NewCombatants;
    TArray<AActor*> AliveEnemies;

    bool bPlayerAlive = false;
    if (IsValid(PlayerActor))
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
                UGameplayStatics::OpenLevel(World, OriginalMapName);
                return;
            }
        }
    }

    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), FoundEnemies);
    for (AActor* Enemy : FoundEnemies)
    {
        if (!IsValid(Enemy))
        {
            continue;
        }
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

    if (bPlayerFled)
    {
        UE_LOG(LogTemp, Log, TEXT("EndRound - Player fled. Returning to base level."));
        UGameplayStatics::OpenLevel(World, OriginalMapName);
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
                    if (IsValid(LoadedWorld) && LoadedWorld->GetName() == MapToLoad.ToString())
                    {
                        TArray<AActor*> Triggers;
                        UGameplayStatics::GetAllActorsWithTag(LoadedWorld, FName("EnemyTrigger"), Triggers);
                        for (AActor* Trigger : Triggers)
                        {
                            if (IsValid(Trigger))
                            {
                                Trigger->Destroy();
                            }
                        }
                    }
                });
        }
        UGameplayStatics::OpenLevel(World, MapToLoad);
        return;
    }

    if (bPlayerAlive && IsValid(PlayerActor))
    {
        if (bPlayerDefendedThisRound)
        {
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
                    {
                        SpeedA = StatA->Speed;
                    }
                    if (const UStatComponent* StatB = B.FindComponentByClass<UStatComponent>())
                    {
                        SpeedB = StatB->Speed;
                    }
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
                    {
                        SpeedA = StatA->Speed;
                    }
                    if (const UStatComponent* StatB = B.FindComponentByClass<UStatComponent>())
                    {
                        SpeedB = StatB->Speed;
                    }
                    return SpeedA > SpeedB;
                });
            UE_LOG(LogTemp, Log, TEXT("EndRound - New combatants order set with player included."));
        }
    }

    Combatants = NewCombatants;
    CurrentTurnIndex = 0;
    UpdateTurnOrderHUD();
    UE_LOG(LogTemp, Log, TEXT("EndRound - Turn order HUD updated"));

    if (Combatants.Num() > 0)
    {
        if (Combatants[0] == PlayerActor)
        {
            if (IsValid(PlayerTurnMenuWidget))
            {
                PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Visible);
                UE_LOG(LogTemp, Log, TEXT("EndRound - PlayerTurnMenuWidget set to visible"));
            }
            APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
            if (AHikariPlayerController* HPC = Cast<AHikariPlayerController>(PC))
            {
                HPC->EnableCombatInputMode();
                UE_LOG(LogTemp, Log, TEXT("EndRound - Combat input mode enabled for player"));
            }
        }
        else
        {
            if (IsValid(PlayerTurnMenuWidget))
            {
                PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
                UE_LOG(LogTemp, Log, TEXT("EndRound - PlayerTurnMenuWidget hidden for enemy turn"));
            }
            APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
            if (AHikariPlayerController* HPC = Cast<AHikariPlayerController>(PC))
            {
                HPC->DisableCombatInputMode();
                UE_LOG(LogTemp, Log, TEXT("EndRound - Combat input mode disabled for player"));
            }
            OnEnemyTurn();
        }
    }

    if (IsValid(PlayerActor))
    {
        if (UStatComponent* PlayerStat = PlayerActor->FindComponentByClass<UStatComponent>())
        {
            PlayerStat->bIsDefending = false;
            UE_LOG(LogTemp, Log, TEXT("EndRound - Player defense bonus cleared."));
        }
    }

    bPlayerDefendedThisRound = false;
    bDefenseConsumed = false;

    UE_LOG(LogTemp, Log, TEXT("EndRound - End"));
}

void UTurnBasedCombatComponent::UpdateTurnOrderHUD()
{
    UE_LOG(LogTemp, Log, TEXT("UpdateTurnOrderHUD - Called"));
    if (!IsValid(TurnOrderWidget))
    {
        return;
    }

    UWorld* World = GetWorld();
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!IsValid(PlayerActor))
    {
        return;
    }

    FullTurnInfos.Empty();
    CurrentRoundInfos.Empty();

    for (AActor* Actor : Combatants)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
        if (UStatComponent* StatComp = Actor->FindComponentByClass<UStatComponent>())
        {
            FCombatantTurnInfo Info;
            Info.Combatant = Actor;
            Info.Speed = StatComp->Speed;
            Info.Icon = (Actor == SelectedEnemy && SelectedIconTexture) ? SelectedIconTexture : (Actor == PlayerActor ? PlayerIconTexture : EnemyIconTexture);
            FullTurnInfos.Add(Info);
        }
    }

    if (CurrentTurnIndex < FullTurnInfos.Num())
    {
        for (int32 i = CurrentTurnIndex; i < FullTurnInfos.Num(); i++)
        {
            CurrentRoundInfos.Add(FullTurnInfos[i]);
        }
    }

    TurnOrderWidget->UpdateTurnOrder(CurrentRoundInfos, FullTurnInfos, SelectedEnemy);
    UE_LOG(LogTemp, Log, TEXT("UpdateTurnOrderHUD - TurnOrderWidget updated"));
}

void UTurnBasedCombatComponent::OnPlayerFlee()
{
    UE_LOG(LogTemp, Log, TEXT("OnPlayerFlee - Called"));
    bPlayerFled = true;
    if (IsValid(PlayerTurnMenuWidget))
    {
        PlayerTurnMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Log, TEXT("OnPlayerFlee - PlayerTurnMenuWidget hidden"));
    }
    UWorld* World = GetWorld();
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (AHikariPlayerController* HPC = Cast<AHikariPlayerController>(PC))
    {
        HPC->DisableCombatInputMode();
        UE_LOG(LogTemp, Log, TEXT("OnPlayerFlee - Combat input mode disabled for player"));
    }
    UGameplayStatics::OpenLevel(World, OriginalMapName);
    UE_LOG(LogTemp, Log, TEXT("OnPlayerFlee - Level change triggered"));
}

//////////////////////////////////////////////////////////////////////////
// Feedback Helper Functions

void UTurnBasedCombatComponent::ApplyFeedbackToEnemy(AActor* Enemy)
{
    UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEnemy - Called for enemy: %s"), IsValid(Enemy) ? *Enemy->GetName() : TEXT("None"));
    if (!IsValid(Enemy) || !IsValid(SelectedEnemyLightFunctionMaterial))
    {
        return;
    }

    UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    if (!IsValid(Mesh))
    {
        Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    }
    if (!IsValid(Mesh))
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyFeedbackToEnemy - Mesh not found for enemy: %s"), *Enemy->GetName());
        return;
    }

    if (!OriginalMaterials.Contains(Enemy))
    {
        UMaterialInterface* OrigMat = Mesh->GetMaterial(0);
        OriginalMaterials.Add(Enemy, OrigMat);
        UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEnemy - Original material stored for enemy: %s"), *Enemy->GetName());
    }

    Mesh->SetMaterial(0, SelectedEnemyLightFunctionMaterial);
    UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEnemy - Applied selected enemy light function material to enemy: %s"), *Enemy->GetName());
}

void UTurnBasedCombatComponent::RemoveFeedbackFromEnemy(AActor* Enemy)
{
    UE_LOG(LogTemp, Log, TEXT("RemoveFeedbackFromEnemy - Called for enemy: %s"), IsValid(Enemy) ? *Enemy->GetName() : TEXT("None"));
    if (!IsValid(Enemy))
    {
        return;
    }

    UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    if (!IsValid(Mesh))
    {
        Mesh = Cast<UPrimitiveComponent>(Enemy->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    }
    if (!IsValid(Mesh))
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