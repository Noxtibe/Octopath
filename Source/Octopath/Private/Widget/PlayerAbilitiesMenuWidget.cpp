#include "Widget/PlayerAbilitiesMenuWidget.h"
#include "Components/ScrollBox.h"
#include "Widget/MyCommonButtonText.h"
#include "Widget/MyCommonButton.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Character/AllyAbilityComponent.h"
#include "GameFramework/Character.h"

void UPlayerAbilitiesMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind the Back button event.
	if (BackButton)
	{
		BackButton->OnMyClicked.RemoveAll(this);
		BackButton->OnMyClicked.AddDynamic(this, &UPlayerAbilitiesMenuWidget::HandleBackButtonClicked);
	}
}

void UPlayerAbilitiesMenuWidget::PopulateAbilitiesMenu()
{
	// Clear any existing children in the ScrollBox and the map.
	if (AbilitiesScrollBox)
	{
		AbilitiesScrollBox->ClearChildren();
	}
	AbilityButtonMap.Empty();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get the player's character.
	ACharacter* PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
	if (!PlayerCharacter)
	{
		return;
	}

	// Get the player's AllyAbilityComponent.
	UAllyAbilityComponent* AllyAbilityComp = PlayerCharacter->FindComponentByClass<UAllyAbilityComponent>();
	if (!AllyAbilityComp)
	{
		return;
	}

	TArray<USkillData*> Skills = AllyAbilityComp->Skills;
	if (Skills.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PopulateAbilitiesMenu - No skills found"));
		return;
	}

	// For each skill, create a button.
	for (USkillData* Skill : Skills)
	{
		if (!Skill)
		{
			continue;
		}
		if (!AbilityButtonClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("PopulateAbilitiesMenu - AbilityButtonClass is not set"));
			continue;
		}
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		UMyCommonButtonText* AbilityButton = CreateWidget<UMyCommonButtonText>(PC, AbilityButtonClass);
		if (!AbilityButton)
		{
			continue;
		}
		// Set the button's display text to the skill's name.
		AbilityButton->SetButtonText(Skill->SkillName);

		// Bind the button's click delegate to our handler.
		AbilityButton->OnMyTextClicked.AddDynamic(this, &UPlayerAbilitiesMenuWidget::OnAbilityButtonClicked);

		// Add the button to the ScrollBox.
		if (AbilitiesScrollBox)
		{
			AbilitiesScrollBox->AddChild(AbilityButton);
		}

		// Map this button to the corresponding skill.
		AbilityButtonMap.Add(AbilityButton, Skill);
	}
}

void UPlayerAbilitiesMenuWidget::OnAbilityButtonClicked(UMyCommonButtonText* ClickedButton)
{
	if (!ClickedButton)
	{
		return;
	}
	if (AbilityButtonMap.Contains(ClickedButton))
	{
		USkillData* SelectedSkill = AbilityButtonMap[ClickedButton];
		UE_LOG(LogTemp, Log, TEXT("OnAbilityButtonClicked - Ability selected: %s"), *SelectedSkill->SkillName.ToString());
		// Broadcast the selected ability.
		OnAbilitySelected.Broadcast(SelectedSkill);
	}
}

void UPlayerAbilitiesMenuWidget::HandleBackButtonClicked(UMyCommonButton* Button)
{
	UE_LOG(LogTemp, Log, TEXT("HandleBackButtonClicked - Back button pressed"));
	// Broadcast the back event so that the parent component can handle it.
	OnBackPressed.Broadcast();
}
