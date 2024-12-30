// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GDAT_PlayMontageAndWaitForEvent.generated.h"

class UGDAbilitySystemComponent;

/** Delegate type used, EventTag and Payload may be empty if it came from the montage callbacks */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGDPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

/**
 * This task combines PlayMontageAndWait and WaitForEvent into one task, so you can wait for multiple types of activations such as from a melee combo
 * Much of this code is copied from one of those two ability tasks
 * This is a good task to look at as an example when creating game-specific tasks
 * It is expected that each game will have a set of game-specific tasks to do what they want
 */

/**
 * 此任务将 PlayMontageAndWait 和 WaitForEvent 结合为一个任务，因此您可以等待多种类型的激活，例如近战连击。
 * 这是一个很好的示例任务，可用于创建特定于游戏的任务。每个游戏都预计会有一组特定于游戏的任务来实现他们的需求
 */
UCLASS()
class GASDOCUMENTATION_API UGDAT_PlayMontageAndWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	UGDAT_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer);

	/**
	* The Blueprint node for this task, PlayMontageAndWaitForEvent, has some black magic from the plugin that automagically calls Activate()
	* inside of K2Node_LatentAbilityCall as stated in the AbilityTask.h. Ability logic written in C++ probably needs to call Activate() itself manually.
	*/
	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;
	virtual void OnDestroy(bool AbilityEnded) override;

	/** The montage completely finished playing */
	UPROPERTY(BlueprintAssignable)
	FGDPlayMontageAndWaitForEventDelegate OnCompleted;

	/** The montage started blending out */
	UPROPERTY(BlueprintAssignable)
	FGDPlayMontageAndWaitForEventDelegate OnBlendOut;

	/** The montage was interrupted */
	UPROPERTY(BlueprintAssignable)
	FGDPlayMontageAndWaitForEventDelegate OnInterrupted;

	/** The ability task was explicitly cancelled by another ability */
	UPROPERTY(BlueprintAssignable)
	FGDPlayMontageAndWaitForEventDelegate OnCancelled;

	/** One of the triggering gameplay events happened */
	UPROPERTY(BlueprintAssignable)
	FGDPlayMontageAndWaitForEventDelegate EventReceived;

	/**
	 * Play a montage and wait for it end. If a gameplay event happens that matches EventTags (or EventTags is empty), the EventReceived delegate will fire with a tag and event data.
	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
	 * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing
	 * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled
	 * 
	 * 播放蒙太奇并等待其结束。如果发生匹配EventTags（或者EventTags为空）的游戏事件，EventReceived委托将会触发，并携带标签和事件数据。
	 * 如果StopWhenAbilityEnds为真，当能力正常结束时，此蒙太奇将会被中止。能力被显式取消时，它总是会被停止。
	 * 
	 * OnBlendOut     会在在正常执行过程中蒙太奇正在混合结束时被调用
	 * OnCompleted    会在蒙太奇完全播放完毕时被调用。
	 * OnInterrupted  会在被另一个蒙太奇覆盖了当前蒙太奇时被调用
	 * OnCancelled    会在如果能力或任务被取消被调用。
	 *
	 * @param TaskInstanceName Set to override the name of this task, for later querying
	 * @param MontageToPlay The montage to play on the character
	 * @param EventTags Any gameplay events matching this tag will activate the EventReceived callback. If empty, all events will trigger callback
	 * @param Rate Change to play the montage faster or slower
	 * @param bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
	 * @param AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGDAT_PlayMontageAndWaitForEvent* PlayMontageAndWaitForEvent(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		UAnimMontage* MontageToPlay,
		FGameplayTagContainer EventTags,
		float Rate = 1.f,
		FName StartSection = NAME_None,
		bool bStopWhenAbilityEnds = true,
		float AnimRootMotionTranslationScale = 1.f);

private:
	/** Montage that is playing */
	UPROPERTY()
	UAnimMontage* MontageToPlay;

	/** List of tags to match against gameplay events */
	UPROPERTY()
	FGameplayTagContainer EventTags;

	/** Playback rate */
	UPROPERTY()
	float Rate;

	/** Section to start montage from */
	UPROPERTY()
	FName StartSection;

	/** Modifies how root motion movement to apply */
	UPROPERTY()
	float AnimRootMotionTranslationScale;

	/** Rather montage should be aborted if ability ends */
	UPROPERTY()
	bool bStopWhenAbilityEnds;

	/** Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not. */
	bool StopPlayingMontage();

	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnAbilityCancelled();
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	/**  */
	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);

	/** 会触发FGDPlayMontageAndWaitForEventDelegate，绑定至AnimInstance */
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle CancelledHandle;
	FDelegateHandle EventHandle;
};
