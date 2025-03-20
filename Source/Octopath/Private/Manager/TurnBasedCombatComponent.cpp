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
    EntityIndicatorTarget = nullptr;
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
        TurnOrderWidget->UpdateTurnOrder(CurrentRoundInfos, FullTurnInfos, EntityIndicatorTarget);
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

    // --- Ability Target Selection Mode ---
    if (bIsSelectingAbilityTarget && CurrentSelectedAbility)
    {
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

        // If the ability is self-targeted or a Heal ability, the target is always the player.
        if (CurrentSelectedAbility->TargetType == ETargetType::Self || CurrentSelectedAbility->AbilityCategory == EAbilityCategory::Heal)
        {
            AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
            if (IsValid(PlayerActor))
            {
                SetEntityIndicator(PlayerActor);
            }
            // Wait for confirmation.
            if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
            {
                TArray<AActor*> Targets;
                Targets.Add(PlayerActor);
                UAllyAbilityComponent* AllyAbilityComp = PlayerActor->FindComponentByClass<UAllyAbilityComponent>();
                if (IsValid(AllyAbilityComp))
                {
                    float EffectResult = AllyAbilityComp->ExecuteSkill(CurrentSelectedAbility, Targets);
                    UE_LOG(LogTemp, Log, TEXT("Self-target ability executed with result: %f"), EffectResult);
                }
                bIsSelectingAbilityTarget = false;
                CurrentSelectedAbility = nullptr;
                AbilityTarget = nullptr;
                if (IsValid(CurrentEnemyIndicatorWidget))
                {
                    CurrentEnemyIndicatorWidget->RemoveFromParent();
                    CurrentEnemyIndicatorWidget = nullptr;
                }
                if (IsValid(PlayerTurnMenuWidget))
                {
                    PlayerTurnMenuWidget->SetRenderOpacity(1.0f);
                }
                NextTurn();
            }
        }
        else
        {
            // For Enemy or Ally targets.
            FHitResult Hit;
            if (!PC->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit))
            {
                return;
            }
            AActor* HitActor = Hit.GetActor();
            bool bValidTarget = false;
            if (CurrentSelectedAbility->TargetType == ETargetType::Enemy)
            {
                bValidTarget = (IsValid(HitActor) && HitActor->ActorHasTag("Enemy"));
            }
            else if (CurrentSelectedAbility->TargetType == ETargetType::Ally)
            {
                bValidTarget = (IsValid(HitActor) && (HitActor->ActorHasTag("Player") || HitActor->ActorHasTag("Ally")));
            }
            if (bValidTarget)
            {
                // For modes Single, Multiple, or Random, auto-select a default target if not in "All" mode.
                if (CurrentSelectedAbility->TargetMode != ETargetMode::All)
                {
                    if (AbilityTarget != HitActor)
                    {
                        AbilityTarget = HitActor;
                        SetEntityIndicator(HitActor);
                        UE_LOG(LogTemp, Log, TEXT("TickComponent (Ability mode) - Target locked: %s"), *HitActor->GetName());
                    }
                }
                // On confirmation (left click).
                if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
                {
                    TArray<AActor*> Targets;
                    if (CurrentSelectedAbility->TargetMode == ETargetMode::All)
                    {
                        Targets = DefaultAbilityTargets;
                    }
                    else
                    {
                        Targets.Add(AbilityTarget);
                    }
                    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
                    if (!IsValid(PlayerActor))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Ability selection: Player actor not found"));
                        return;
                    }
                    UAllyAbilityComponent* AllyAbilityComp = PlayerActor->FindComponentByClass<UAllyAbilityComponent>();
                    if (!IsValid(AllyAbilityComp))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Ability selection: AllyAbilityComponent not found"));
                        return;
                    }
                    float EffectResult = AllyAbilityComp->ExecuteSkill(CurrentSelectedAbility, Targets);
                    UE_LOG(LogTemp, Log, TEXT("Ability executed on target(s) with result: %f"), EffectResult);

                    // Reset ability selection.
                    bIsSelectingAbilityTarget = false;
                    CurrentSelectedAbility = nullptr;
                    AbilityTarget = nullptr;
                    DefaultAbilityTargets.Empty();
                    if (IsValid(CurrentEnemyIndicatorWidget))
                    {
                        CurrentEnemyIndicatorWidget->RemoveFromParent();
                        CurrentEnemyIndicatorWidget = nullptr;
                    }
                    if (IsValid(PlayerTurnMenuWidget))
                    {
                        PlayerTurnMenuWidget->SetRenderOpacity(1.0f);
                    }
                    NextTurn();
                }
            }
        }
        return; // End of ability target selection mode.
    }

    // --- Basic Target Selection Mode (for attacks) ---
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
    if (EntityIndicatorTarget != HitActor)
    {
        if (IsValid(EntityIndicatorTarget))
        {
            RemoveFeedbackFromEntity(EntityIndicatorTarget);
            UE_LOG(LogTemp, Log, TEXT("TickComponent - Removed feedback from previous target: %s"), *EntityIndicatorTarget->GetName());
        }
        SetEntityIndicator(HitActor);
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

void UTurnBasedCombatComponent::SetEntityIndicator(AActor* NewTarget)
{
    UE_LOG(LogTemp, Log, TEXT("SetEntityIndicator - Called with target: %s"), IsValid(NewTarget) ? *NewTarget->GetName() : TEXT("None"));

    if (IsValid(EntityIndicatorTarget) && EntityIndicatorTarget != NewTarget)
    {
        RemoveFeedbackFromEntity(EntityIndicatorTarget);
        UE_LOG(LogTemp, Log, TEXT("SetEntityIndicator - Removed feedback from previous target: %s"), *EntityIndicatorTarget->GetName());
    }

    EntityIndicatorTarget = NewTarget;

    if (!IsValid(CurrentEnemyIndicatorWidget))
    {
        UWorld* World = GetWorld();
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (IsValid(PC) && IsValid(EnemyIndicatorWidgetClass))
        {
            CurrentEnemyIndicatorWidget = CreateWidget<UEnemyIndicatorWidget>(PC, EnemyIndicatorWidgetClass);
            if (IsValid(CurrentEnemyIndicatorWidget))
            {
                CurrentEnemyIndicatorWidget->AddToViewport();
                UE_LOG(LogTemp, Log, TEXT("SetEntityIndicator - Indicator widget created and added to viewport"));
            }
        }
    }

    UpdateEnemyIndicatorPosition();
    ApplyFeedbackToEntity(NewTarget);

    if (IsValid(CurrentEnemyIndicatorWidget) && IsValid(NewTarget))
    {
        if (UStatComponent* StatComp = NewTarget->FindComponentByClass<UStatComponent>())
        {
            CurrentEnemyIndicatorWidget->SetEnemyName(StatComp->EntityName);
            UE_LOG(LogTemp, Log, TEXT("SetEntityIndicator - Updated indicator name: %s"), *StatComp->EntityName.ToString());
        }
    }
}

void UTurnBasedCombatComponent::UpdateEnemyIndicatorPosition()
{
    UE_LOG(LogTemp, Log, TEXT("UpdateEnemyIndicatorPosition - Called"));
    if (!IsValid(EntityIndicatorTarget) || !IsValid(CurrentEnemyIndicatorWidget))
    {
        return;
    }

    UWorld* World = GetWorld();
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!IsValid(PC))
    {
        return;
    }

    FVector WorldLocation = EntityIndicatorTarget->GetActorLocation() + FVector(0.f, 0.f, IndicatorWorldVerticalOffset);
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
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!IsValid(PlayerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - Player actor not found"));
        return;
    }

    // If not already in target selection mode, auto-select a default enemy.
    if (!bIsSelectingTarget)
    {
        bIsSelectingTarget = true;
        // Auto-select the first enemy found (assuming non-player actors are enemies)
        for (AActor* Actor : Combatants)
        {
            if (IsValid(Actor) && !Actor->ActorHasTag("Player"))
            {
                SetEntityIndicator(Actor);
                bTargetLocked = true;
                UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Default target auto-selected: %s"), *Actor->GetName());
                break;
            }
        }
        // On attend la confirmation du clic (dans TickComponent ou dans un appel ultérieur)
        return;
    }

    if (bIsSelectingTarget && !bTargetLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - No target locked. Please hover over an enemy to lock target."));
        return;
    }

    // Execute the default attack.
    if (UAllyAbilityComponent* AllyAbility = PlayerActor->FindComponentByClass<UAllyAbilityComponent>())
    {
        float DamageAmount = AllyAbility->ExecuteDefaultAttack();
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Player attack damage: %f"), DamageAmount);
        if (IsValid(EntityIndicatorTarget))
        {
            if (UStatComponent* EnemyStat = EntityIndicatorTarget->FindComponentByClass<UStatComponent>())
            {
                EnemyStat->ApplyDamage(DamageAmount, false);
                UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Applied damage to enemy %s. New HP: %f/%f"),
                    *EntityIndicatorTarget->GetName(), EnemyStat->Health, EnemyStat->MaxHealth);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerAttack - Player has no AllyAbilityComponent"));
    }

    // Remove feedback and reset selection.
    if (IsValid(EntityIndicatorTarget))
    {
        RemoveFeedbackFromEntity(EntityIndicatorTarget);
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Removed feedback from target: %s"), *EntityIndicatorTarget->GetName());
    }
    bIsSelectingTarget = false;
    bTargetLocked = false;
    EntityIndicatorTarget = nullptr;
    if (IsValid(CurrentEnemyIndicatorWidget))
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - Indicator widget removed"));
    }
    NextTurn();
    UE_LOG(LogTemp, Log, TEXT("OnPlayerAttack - End"));
}

void UTurnBasedCombatComponent::ShowAbilitiesMenu()
{
    UE_LOG(LogTemp, Log, TEXT("ShowAbilitiesMenu called"));

    // Instead of completely hiding the main menu, adjust its opacity.
    if (IsValid(PlayerTurnMenuWidget))
    {
        PlayerTurnMenuWidget->SetRenderOpacity(MainMenuOpacityWhenAbilitiesShown);
    }

    if (!IsValid(PlayerAbilitiesMenuWidget) && IsValid(GetWorld()))
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (IsValid(PC) && PlayerAbilitiesMenuWidgetClass)
        {
            PlayerAbilitiesMenuWidget = CreateWidget<UPlayerAbilitiesMenuWidget>(PC, PlayerAbilitiesMenuWidgetClass);
            if (IsValid(PlayerAbilitiesMenuWidget))
            {
                // Bind the ability selection event.
                PlayerAbilitiesMenuWidget->OnAbilitySelected.AddDynamic(this, &UTurnBasedCombatComponent::OnAbilitySelected);
                // Bind the back button event.
                PlayerAbilitiesMenuWidget->OnBackPressed.AddDynamic(this, &UTurnBasedCombatComponent::HideAbilitiesMenu);

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

void UTurnBasedCombatComponent::OnAbilitySelected(USkillData* SelectedSkill)
{
    if (!SelectedSkill)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnAbilitySelected: SelectedSkill is null"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("OnAbilitySelected called with skill: %s"), *SelectedSkill->SkillName.ToString());

    // Get player's character.
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!IsValid(PlayerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnAbilitySelected: Player actor not found"));
        return;
    }

    // Get player's ability component.
    UAllyAbilityComponent* AllyAbilityComp = PlayerActor->FindComponentByClass<UAllyAbilityComponent>();
    if (!IsValid(AllyAbilityComp))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnAbilitySelected: AllyAbilityComponent not found"));
        return;
    }

    // Activate ability target selection mode.
    CurrentSelectedAbility = SelectedSkill;
    bIsSelectingAbilityTarget = true;

    // Pour une capacité Self, la cible par défaut est le joueur.
    if (SelectedSkill->TargetType == ETargetType::Self || SelectedSkill->AbilityCategory == EAbilityCategory::Heal)
    {
        SetEntityIndicator(PlayerActor);
        UE_LOG(LogTemp, Log, TEXT("OnAbilitySelected - Self-target ability selection mode activated"));
    }
    // Pour les capacités offensives ou de debuff.
    else if (SelectedSkill->AbilityCategory == EAbilityCategory::Offensive ||
        SelectedSkill->AbilityCategory == EAbilityCategory::Debuff)
    {
        // Si le TargetMode est All, sélectionnez tous les ennemis par défaut.
        if (SelectedSkill->TargetMode == ETargetMode::All)
        {
            DefaultAbilityTargets.Empty();
            TArray<AActor*> AllEnemies;
            UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), AllEnemies);
            for (AActor* Enemy : AllEnemies)
            {
                if (IsValid(Enemy))
                {
                    DefaultAbilityTargets.Add(Enemy);
                    ApplyFeedbackToEntity(Enemy);
                }
            }
            UE_LOG(LogTemp, Log, TEXT("OnAbilitySelected - Default selection set to ALL enemies (%d found)"), DefaultAbilityTargets.Num());
        }
        else
        {
            // Pour Single, Multiple ou Random, sélectionnez par défaut le premier ennemi trouvé.
            TArray<AActor*> FoundEnemies;
            UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FoundEnemies);
            if (FoundEnemies.Num() > 0)
            {
                AActor* DefaultEnemy = FoundEnemies[0];
                SetEntityIndicator(DefaultEnemy);
                AbilityTarget = DefaultEnemy;
                UE_LOG(LogTemp, Log, TEXT("OnAbilitySelected - Default target set to enemy: %s"), *DefaultEnemy->GetName());
            }
        }
    }
    else if (SelectedSkill->TargetType == ETargetType::Ally)
    {
        // Pour les buffs sur alliés, on pourrait définir ici une cible par défaut (ex. le joueur)
        SetEntityIndicator(PlayerActor);
        UE_LOG(LogTemp, Log, TEXT("OnAbilitySelected - Ally-target ability selection mode activated (default set to self)"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("OnAbilitySelected - No default selection for this skill type"));
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

void UTurnBasedCombatComponent::HideAbilitiesMenu()
{
    UE_LOG(LogTemp, Log, TEXT("HideAbilitiesMenu called, returning to main action menu"));
    if (IsValid(PlayerAbilitiesMenuWidget))
    {
        PlayerAbilitiesMenuWidget->SetVisibility(ESlateVisibility::Hidden);
    }
    if (IsValid(PlayerTurnMenuWidget))
    {
        PlayerTurnMenuWidget->SetRenderOpacity(1.0f);
    }
    // Cancel any ability target selection.
    bIsSelectingAbilityTarget = false;
    CurrentSelectedAbility = nullptr;
    AbilityTarget = nullptr;
    // Remove the indicator widget if present.
    if (IsValid(CurrentEnemyIndicatorWidget))
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
    }
    // Remove feedback from default ability targets.
    for (AActor* Target : DefaultAbilityTargets)
    {
        if (IsValid(Target))
        {
            RemoveFeedbackFromEntity(Target);
        }
    }
    DefaultAbilityTargets.Empty();
    // Also remove feedback from any currently indicated entity.
    if (IsValid(EntityIndicatorTarget))
    {
        RemoveFeedbackFromEntity(EntityIndicatorTarget);
        EntityIndicatorTarget = nullptr;
    }
}

void UTurnBasedCombatComponent::NextTurn()
{
    UE_LOG(LogTemp, Log, TEXT("NextTurn - Called. CurrentTurnIndex: %d, Total Combatants: %d"), CurrentTurnIndex, Combatants.Num());

    // Reset targeting variables.
    bIsSelectingTarget = false;
    bTargetLocked = false;

    if (IsValid(EntityIndicatorTarget))
    {
        RemoveFeedbackFromEntity(EntityIndicatorTarget);
        UE_LOG(LogTemp, Log, TEXT("NextTurn - Removed feedback from selected target: %s"), *EntityIndicatorTarget->GetName());
        EntityIndicatorTarget = nullptr;
    }

    if (IsValid(CurrentEnemyIndicatorWidget))
    {
        CurrentEnemyIndicatorWidget->RemoveFromParent();
        CurrentEnemyIndicatorWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("NextTurn - Removed enemy indicator widget"));
    }

    ++CurrentTurnIndex;
    UE_LOG(LogTemp, Log, TEXT("NextTurn - Incremented turn index to: %d"), CurrentTurnIndex);

    // If all combatants have acted, end the round.
    if (CurrentTurnIndex >= Combatants.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("NextTurn - All combatants have acted. Calling EndRound."));
        EndRound();
        return;
    }

    // Re-sort remaining combatants based on current Speed.
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

    // Decrement modifiers on all combatants so expired buffs/debuffs are removed.
    for (AActor* Combatant : Combatants)
    {
        if (UStatComponent* StatComp = Combatant->FindComponentByClass<UStatComponent>())
        {
            StatComp->DecrementStatModifiers();
        }
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
            Info.Icon = (Actor == EntityIndicatorTarget && SelectedIconTexture) ? SelectedIconTexture : (Actor == PlayerActor ? PlayerIconTexture : EnemyIconTexture);
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

    TurnOrderWidget->UpdateTurnOrder(CurrentRoundInfos, FullTurnInfos, EntityIndicatorTarget);
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

void UTurnBasedCombatComponent::ApplyFeedbackToEntity(AActor* Enemy)
{
    UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEntity - Called for enemy: %s"), IsValid(Enemy) ? *Enemy->GetName() : TEXT("None"));
    if (!IsValid(Enemy) || !IsValid(EntityIndicatorLightFunctionMaterial))
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
        UE_LOG(LogTemp, Warning, TEXT("ApplyFeedbackToEntity - Mesh not found for enemy: %s"), *Enemy->GetName());
        return;
    }

    if (!OriginalMaterials.Contains(Enemy))
    {
        UMaterialInterface* OrigMat = Mesh->GetMaterial(0);
        OriginalMaterials.Add(Enemy, OrigMat);
        UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEntity - Original material stored for enemy: %s"), *Enemy->GetName());
    }

    Mesh->SetMaterial(0, EntityIndicatorLightFunctionMaterial);
    UE_LOG(LogTemp, Log, TEXT("ApplyFeedbackToEntity - Applied selected enemy light function material to enemy: %s"), *Enemy->GetName());
}

void UTurnBasedCombatComponent::RemoveFeedbackFromEntity(AActor* Enemy)
{
    UE_LOG(LogTemp, Log, TEXT("RemoveFeedbackFromEntity - Called for enemy: %s"), IsValid(Enemy) ? *Enemy->GetName() : TEXT("None"));
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
        UE_LOG(LogTemp, Warning, TEXT("RemoveFeedbackFromEntity - Mesh not found for enemy: %s"), *Enemy->GetName());
        return;
    }

    if (OriginalMaterials.Contains(Enemy))
    {
        Mesh->SetMaterial(0, OriginalMaterials[Enemy]);
        OriginalMaterials.Remove(Enemy);
        UE_LOG(LogTemp, Log, TEXT("RemoveFeedbackFromEntity - Restored original material for enemy: %s"), *Enemy->GetName());
    }
}