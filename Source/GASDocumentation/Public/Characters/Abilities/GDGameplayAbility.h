// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GASDocumentation/GASDocumentation.h"
#include "GDGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class GASDOCUMENTATION_API UGDGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGDGameplayAbility();
	
	// 按下输入时，具有此组的技能将自动激活
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGDAbilityInputID AbilityInputID = EGDAbilityInputID::None;
	
	// 用于将技能与某个槽（slot）关联，而不需要依赖于输入事件来自动激活技能。
	// 这种设计非常适用于被动技能，因为被动技能通常不依赖于玩家的输入来激活，而是自动生效。通过 AbilityID，你可以为这些技能分配一个唯一标识符，以便在技能槽中进行管理，而无需输入事件的触发。
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGDAbilityInputID AbilityID = EGDAbilityInputID::None;
	
	// 该能力是否应在被授予角色时立即激活。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool ActivateAbilityOnGranted = false;

	// 如果 ActivateAbilityOnGranted 被设置为 true，能力将在赋予角色时立即激活。此时，OnAvatarSet 方法将会被执行，处理一些初始化操作。
	// Epic's comment：在这个时机执行一些类似 BeginPlay 的逻辑，特别是对于被动技能或其他自动激活的技能。
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
