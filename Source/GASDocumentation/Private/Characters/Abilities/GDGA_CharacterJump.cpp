// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GDGA_CharacterJump.h"
#include "Characters/GDCharacterBase.h"
#include "GASDocumentation/GASDocumentation.h"

UGDGA_CharacterJump::UGDGA_CharacterJump()
{
	AbilityInputID = EGDAbilityInputID::Jump;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Jump")));

}

void UGDGA_CharacterJump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData * TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}

		ACharacter * Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
		Character->Jump();
	}
}

bool UGDGA_CharacterJump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayTagContainer * SourceTags, const FGameplayTagContainer * TargetTags, OUT FGameplayTagContainer * OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AGDCharacterBase* Character = CastChecked<AGDCharacterBase>(ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);
	return Character && Character->CanJump();
}

void UGDGA_CharacterJump::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo != NULL && ActorInfo->AvatarActor != NULL)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

// Epic's comment
/**
 *	Canceling an non instanced ability is tricky. Right now this works for Jump since there is nothing that can go wrong by calling
 *	StopJumping() if you aren't already jumping. If we had a montage playing non instanced ability, it would need to make sure the
 *	Montage that *it* played was still playing, and if so, to cancel it. If this is something we need to support, we may need some
 *	light weight data structure to represent 'non intanced abilities in action' with a way to cancel/end them.
 *	
 *	取消一个非实例化的能力会比较棘手。当前这种方式适用于 Jump（跳跃），因为即使调用 StopJumping() 时你并未跳跃，也不会出现问题。
 *	如果我们有一个播放蒙太奇的非实例化能力，它需要确保是 *它自身* 播放的蒙太奇还在播放，并在这种情况下将其取消。
 *	如果我们需要支持这种功能，可能需要某种轻量级的数据结构来表示“正在执行的非实例化能力”，并提供一种取消/结束它们的方法。
 */
void UGDGA_CharacterJump::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UGDGA_CharacterJump::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	ACharacter * Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	Character->StopJumping();
}