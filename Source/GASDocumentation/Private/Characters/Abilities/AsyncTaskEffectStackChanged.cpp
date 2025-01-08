// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AsyncTaskEffectStackChanged.h"


/*
 * 先绑定GE激活/清除，在触发时绑定GameplayEffectStackChange
 * 
 */
UAsyncTaskEffectStackChanged * UAsyncTaskEffectStackChanged::ListenForGameplayEffectStackChange(UAbilitySystemComponent * AbilitySystemComponent, FGameplayTag InEffectGameplayTag)
{
	UAsyncTaskEffectStackChanged* ListenForGameplayEffectStackChange = NewObject<UAsyncTaskEffectStackChanged>();
	ListenForGameplayEffectStackChange->ASC = AbilitySystemComponent;
	ListenForGameplayEffectStackChange->EffectGameplayTag = InEffectGameplayTag;

	if (!IsValid(AbilitySystemComponent) || !InEffectGameplayTag.IsValid())
	{
		ListenForGameplayEffectStackChange->EndTask();
		return nullptr;
	}

	// 当GE被激活时会触发的委托
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(ListenForGameplayEffectStackChange, &UAsyncTaskEffectStackChanged::OnActiveGameplayEffectAddedCallback);
	// 当GE被清除时会触发的委托
	AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(ListenForGameplayEffectStackChange, &UAsyncTaskEffectStackChanged::OnRemoveGameplayEffectCallback);

	return ListenForGameplayEffectStackChange;
}

void UAsyncTaskEffectStackChanged::EndTask()
{
	if (IsValid(ASC))
	{
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
		ASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);
		
		if(ActiveEffectHandle.IsValid())
		{
			ASC->OnGameplayEffectStackChangeDelegate(ActiveEffectHandle)->RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UAsyncTaskEffectStackChanged::OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent * Target, const FGameplayEffectSpec & SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(EffectGameplayTag) || GrantedTags.HasTagExact(EffectGameplayTag))
	{
		// 绑定GameplayEffectStackChanged
		ASC->OnGameplayEffectStackChangeDelegate(ActiveHandle)->AddUObject(this, &UAsyncTaskEffectStackChanged::GameplayEffectStackChanged);
		// 触发OnGameplayEffectStackChange引脚
		OnGameplayEffectStackChange.Broadcast(EffectGameplayTag, ActiveHandle, 1, 0);
		ActiveEffectHandle = ActiveHandle;
	}
}

void UAsyncTaskEffectStackChanged::OnRemoveGameplayEffectCallback(const FActiveGameplayEffect & EffectRemoved)
{
	FGameplayTagContainer AssetTags;
	EffectRemoved.Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	EffectRemoved.Spec.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(EffectGameplayTag) || GrantedTags.HasTagExact(EffectGameplayTag))
	{
		// 触发OnGameplayEffectStackChange引脚，不过是移除
		OnGameplayEffectStackChange.Broadcast(EffectGameplayTag, EffectRemoved.Handle, 0, 1);
	}
}

void UAsyncTaskEffectStackChanged::GameplayEffectStackChanged(FActiveGameplayEffectHandle EffectHandle, int32 NewStackCount, int32 PreviousStackCount)
{
	// 触发OnGameplayEffectStackChange引脚
	OnGameplayEffectStackChange.Broadcast(EffectGameplayTag, EffectHandle, NewStackCount, PreviousStackCount);
}
