#include "Widget/PlayerStatsWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/StatComponent.h"

void UPlayerStatsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Retrieve necessary references once.
	GetReferences();

	// Initial update for each part.
	UpdatePlayerName();
	UpdateHealth();
	UpdateTechniquePoints();

	// Subscribe to specific stat update events.
	if (PlayerStatComp)
	{
		PlayerStatComp->OnHealthChanged.AddDynamic(this, &UPlayerStatsWidget::UpdateHealth);
		PlayerStatComp->OnTechniquePointsChanged.AddDynamic(this, &UPlayerStatsWidget::UpdateTechniquePoints);
		// If the player's name may change during gameplay, we can also subscribe to an appropriate event.
	}
}

void UPlayerStatsWidget::GetReferences()
{
	// Get the player controller.
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	// Get the player's pawn.
	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return;
	}

	// Cache the reference to the player's stat component.
	PlayerStatComp = Pawn->FindComponentByClass<UStatComponent>();
}

void UPlayerStatsWidget::UpdatePlayerName()
{
	if (PlayerNameText && PlayerStatComp)
	{
		PlayerNameText->SetText(PlayerStatComp->EntityName);
	}
}

void UPlayerStatsWidget::UpdateHealth()
{
	if (!PlayerStatComp)
	{
		return;
	}

	// Update health text ("Current/Max").
	if (HealthText)
	{
		FString HealthStr = FString::Printf(TEXT("%.0f/%.0f"), PlayerStatComp->Health, PlayerStatComp->MaxHealth);
		HealthText->SetText(FText::FromString(HealthStr));
	}

	// Update the health progress bar.
	if (HealthBar)
	{
		float HealthPercent = (PlayerStatComp->MaxHealth > 0.f) ? PlayerStatComp->Health / PlayerStatComp->MaxHealth : 0.f;
		HealthBar->SetPercent(HealthPercent);
	}
}

void UPlayerStatsWidget::UpdateTechniquePoints()
{
	if (!PlayerStatComp)
	{
		return;
	}

	// Update technique text ("Current/Max").
	if (TechniqueText)
	{
		FString TechniqueStr = FString::Printf(TEXT("%.0f/%.0f"), PlayerStatComp->TechniquePoints, PlayerStatComp->MaxTechniquePoints);
		TechniqueText->SetText(FText::FromString(TechniqueStr));
	}

	// Update the technique progress bar.
	if (TechniqueBar)
	{
		float TechniquePercent = (PlayerStatComp->MaxTechniquePoints > 0.f) ? PlayerStatComp->TechniquePoints / PlayerStatComp->MaxTechniquePoints : 0.f;
		TechniqueBar->SetPercent(TechniquePercent);
	}
}
