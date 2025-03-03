#include "Widget/PlayerStatsWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/StatComponent.h"

void UPlayerStatsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Get the player controller.
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        return;
    }

    // Get the player pawn.
    APawn* Pawn = PC->GetPawn();
    if (!Pawn)
    {
        return;
    }

    // Get the player's StatComponent.
    UStatComponent* StatComp = Pawn->FindComponentByClass<UStatComponent>();
    if (!StatComp)
    {
        return;
    }

    // Update the player's name.
    if (PlayerNameText)
    {
        PlayerNameText->SetText(StatComp->EntityName);
    }

    // Update health text ("Current/Max").
    if (HealthText)
    {
        FString HealthStr = FString::Printf(TEXT("%.0f/%.0f"), StatComp->Health, StatComp->MaxHealth);
        HealthText->SetText(FText::FromString(HealthStr));
    }

    // Update the health progress bar.
    if (HealthBar)
    {
        float HealthPercent = (StatComp->MaxHealth > 0.f) ? StatComp->Health / StatComp->MaxHealth : 0.f;
        HealthBar->SetPercent(HealthPercent);
    }

    // Update mana text ("Current/Max").
    if (ManaText)
    {
        FString ManaStr = FString::Printf(TEXT("%.0f/%.0f"), StatComp->TechniquePoints, StatComp->MaxTechniquePoints);
        ManaText->SetText(FText::FromString(ManaStr));
    }

    // Update the mana progress bar.
    if (ManaBar)
    {
        float ManaPercent = (StatComp->MaxTechniquePoints > 0.f) ? StatComp->TechniquePoints / StatComp->MaxTechniquePoints : 0.f;
        ManaBar->SetPercent(ManaPercent);
    }
}
