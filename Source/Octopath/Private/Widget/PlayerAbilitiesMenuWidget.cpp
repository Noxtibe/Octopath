#include "Widget/PlayerAbilitiesMenuWidget.h"
#include "Components/ScrollBox.h"
#include "Widget/MyCommonButtonText.h"
#include "Widget/MyCommonButton.h"
#include "Kismet/GameplayStatics.h"
#include "Character/AllyAbilityComponent.h"
#include "GameFramework/Character.h"
#include "Widget/SkillDescriptionWidget.h"

// Define a custom log category for this widget.
DEFINE_LOG_CATEGORY_STATIC(LogPlayerAbilitiesMenu, Log, All);

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

	// Remove any existing description widget.
	if (CurrentSkillDescriptionWidget)
	{
		CurrentSkillDescriptionWidget->RemoveFromParent();
		CurrentSkillDescriptionWidget = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("PopulateAbilitiesMenu: World is null"));
		return;
	}

	// Get the player's character.
	ACharacter* PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
	if (!PlayerCharacter)
	{
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("PopulateAbilitiesMenu: PlayerCharacter is null"));
		return;
	}

	// Get the player's AllyAbilityComponent.
	UAllyAbilityComponent* AllyAbilityComp = PlayerCharacter->FindComponentByClass<UAllyAbilityComponent>();
	if (!AllyAbilityComp)
	{
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("PopulateAbilitiesMenu: AllyAbilityComponent is null"));
		return;
	}

	TArray<USkillData*> Skills = AllyAbilityComp->Skills;
	if (Skills.Num() == 0)
	{
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("PopulateAbilitiesMenu: No skills found"));
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
			UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("PopulateAbilitiesMenu: AbilityButtonClass is not set"));
			continue;
		}
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		UMyCommonButtonText* AbilityButton = CreateWidget<UMyCommonButtonText>(PC, AbilityButtonClass);
		if (!AbilityButton)
		{
			UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("PopulateAbilitiesMenu: Failed to create AbilityButton"));
			continue;
		}
		// Set the button's display text to the skill's name.
		AbilityButton->SetButtonText(Skill->SkillName);
		UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("PopulateAbilitiesMenu: Created button for skill: %s"), *Skill->SkillName.ToString());

		// Bind the button's click event.
		AbilityButton->OnMyTextClicked.AddDynamic(this, &UPlayerAbilitiesMenuWidget::OnAbilityButtonClicked);

		// Bind the hover events.
		AbilityButton->OnHovered.AddDynamic(this, &UPlayerAbilitiesMenuWidget::OnAbilityButtonHovered);
		AbilityButton->OnUnhovered.AddDynamic(this, &UPlayerAbilitiesMenuWidget::OnAbilityButtonUnhovered);

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
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("OnAbilityButtonClicked: ClickedButton is null"));
		return;
	}
	if (AbilityButtonMap.Contains(ClickedButton))
	{
		USkillData* SelectedSkill = AbilityButtonMap[ClickedButton];
		UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("OnAbilityButtonClicked: Ability selected: %s"), *SelectedSkill->SkillName.ToString());
		// Broadcast the selected ability.
		OnAbilitySelected.Broadcast(SelectedSkill);
	}
	else
	{
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("OnAbilityButtonClicked: Button not found in map"));
	}
}

void UPlayerAbilitiesMenuWidget::HandleBackButtonClicked(UMyCommonButton* Button)
{
	UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("HandleBackButtonClicked: Back button pressed"));
	// Broadcast the back event so that the parent component can handle it.
	OnBackPressed.Broadcast();
}

void UPlayerAbilitiesMenuWidget::OnAbilityButtonHovered(UMyCommonButtonText* HoveredButton)
{
	if (!HoveredButton)
	{
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("OnAbilityButtonHovered: HoveredButton is null"));
		return;
	}

	UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("OnAbilityButtonHovered: Called for button %s"), *HoveredButton->GetName());

	if (!AbilityButtonMap.Contains(HoveredButton))
	{
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("OnAbilityButtonHovered: Button %s not found in map"), *HoveredButton->GetName());
		return;
	}

	USkillData* Skill = AbilityButtonMap[HoveredButton];
	if (!Skill)
	{
		UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("OnAbilityButtonHovered: Mapped skill is null for button %s"), *HoveredButton->GetName());
		return;
	}

	// Create the description widget if it doesn't already exist.
	if (!CurrentSkillDescriptionWidget && SkillDescriptionWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("OnAbilityButtonHovered: Creating SkillDescriptionWidget"));
		CurrentSkillDescriptionWidget = CreateWidget<USkillDescriptionWidget>(PC, SkillDescriptionWidgetClass);
		if (CurrentSkillDescriptionWidget)
		{
			UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("OnAbilityButtonHovered: SkillDescriptionWidget created, adding to viewport"));
			CurrentSkillDescriptionWidget->AddToViewport();
			// Ensure the widget does not intercept clicks.
			CurrentSkillDescriptionWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			UE_LOG(LogPlayerAbilitiesMenu, Warning, TEXT("OnAbilityButtonHovered: Failed to create SkillDescriptionWidget"));
		}
	}

	if (CurrentSkillDescriptionWidget)
	{
		UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("OnAbilityButtonHovered: Setting description text: %s"), *Skill->Description.ToString());
		CurrentSkillDescriptionWidget->SetDescription(Skill->Description);

		// Use the fixed position defined in the editor.
		UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("OnAbilityButtonHovered: Using fixed position: (%f, %f)"),
			DescriptionWidgetFixedPosition.X, DescriptionWidgetFixedPosition.Y);
		CurrentSkillDescriptionWidget->SetAnchorsInViewport(FAnchors(0.5f, 0.f, 0.5f, 0.f));
		CurrentSkillDescriptionWidget->SetAlignmentInViewport(FVector2D(0.5f, 1.f));
		CurrentSkillDescriptionWidget->SetPositionInViewport(DescriptionWidgetFixedPosition, false);
	}
}

void UPlayerAbilitiesMenuWidget::OnAbilityButtonUnhovered(UMyCommonButtonText* UnhoveredButton)
{
	UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("OnAbilityButtonUnhovered: Called for button %s"), *UnhoveredButton->GetName());
	if (CurrentSkillDescriptionWidget)
	{
		UE_LOG(LogPlayerAbilitiesMenu, Log, TEXT("OnAbilityButtonUnhovered: Removing SkillDescriptionWidget from viewport"));
		CurrentSkillDescriptionWidget->RemoveFromParent();
		CurrentSkillDescriptionWidget = nullptr;
	}
}
