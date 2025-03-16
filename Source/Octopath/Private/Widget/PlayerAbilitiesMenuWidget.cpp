#include "Widget/PlayerAbilitiesMenuWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Widget/MyCommonButtonText.h"
#include "Kismet/GameplayStatics.h"
#include "Character/AllyAbilityComponent.h"
#include "GameFramework/Character.h"

void UPlayerAbilitiesMenuWidget::PopulateAbilitiesMenu()
{
    // Clear any existing buttons and the map.
    if (AbilitiesListBox)
    {
        AbilitiesListBox->ClearChildren();
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
    // Get the player's AllyAbilityComponent (assurez-vous qu'il contient un TArray<USkillData*> Skills)
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

        // Bind the button's OnMyClicked delegate to our handler.
        AbilityButton->OnMyTextClicked.AddDynamic(this, &UPlayerAbilitiesMenuWidget::OnAbilityButtonClicked);

        // Add the button to the vertical box.
        if (AbilitiesListBox)
        {
            UVerticalBoxSlot* NewSlot = AbilitiesListBox->AddChildToVerticalBox(AbilityButton);
            if (NewSlot)
            {
                NewSlot->SetPadding(FMargin(5.f));
            }
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
