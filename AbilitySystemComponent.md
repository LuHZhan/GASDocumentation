## 13. Lu

### 13.1 AbilitySystemComponent.h

用于与游戏中的能力系统（包括能力、效果和属性）交互的核心组件。它的主要功能包括：

**GameplayAbilities**：提供能力的管理、分配和复制，允许玩家或AI使用能力，并确保在网络上正确同步。

**GameplayEffects**：用于处理和应用游戏效果，并提供查询和修改这些效果的功能。

**GameplayAttributes**：用于分配和管理角色的属性集，帮助存取和修改角色的属性。



#### 13.1 Delegates

```cpp
DECLARE_MULTICAST_DELEGATE_OneParam(FTargetingRejectedConfirmation, int32);
```

- **用途**：当一个目标演员拒绝目标确认时会调用该委托。通常用于射击或选择目标时，玩家或系统拒绝当前目标的确认。
- **参数**：`int32` 是一个参数，可能表示拒绝的原因或其他相关信息（例如，错误代码）。



```cpp
DECLARE_MULTICAST_DELEGATE_TwoParams(FAbilityFailedDelegate, const UGameplayAbility*, const FGameplayTagContainer&);
```

- **用途**：当能力激活失败时调用，传递失败的能力以及一个标签容器，标签容器说明了失败的原因（例如，能力无法执行的原因，如缺少资源或不满足条件）。

- **参数：**`const UGameplayAbility*`：表示无法激活的能力。`const FGameplayTagContainer&`：包含表示失败原因的标签集合。



```cpp
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilityEnded, UGameplayAbility*);
```

- **用途**：当能力结束时调用，用于通知所有订阅者该能力已结束。
- **参数**：`UGameplayAbility*`：表示已结束的能力。



```cpp
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitySpecDirtied, const FGameplayAbilitySpec&);
```

- **用途**：当能力规格（Ability Spec）被修改时通知监听者。

- **参数**：`const FGameplayAbilitySpec&`：表示已修改的能力规格。

  

```cpp
DECLARE_MULTICAST_DELEGATE_TwoParams(FImmunityBlockGE, const FGameplayEffectSpec& /*BlockedSpec*/, const FActiveGameplayEffect* /*ImmunityGameplayEffect*/);
```

- **用途**：当 `GameplayEffectSpec` 被一个 `ActiveGameplayEffect` 阻止时调用，通常因为免疫效果导致阻止某个 `GameplayEffect` 的应用。

- **参数：**

  `const FGameplayEffectSpec&`：被阻止的游戏效果规格。

  `const FActiveGameplayEffect*`：导致免疫的 `ActiveGameplayEffect`。

  

```cpp
DECLARE_DELEGATE_RetVal_TwoParams(bool, FGameplayEffectApplicationQuery, const FActiveGameplayEffectsContainer& /*ActiveGEContainer*/, const FGameplayEffectSpec& /*GESpecToConsider*/);
```

- **用途**：用于查询是否可以应用一个 `GameplayEffect`，返回一个布尔值决定是否允许应用。此委托允许使用多个委托来决定某个 `GameplayEffect` 是否可以被阻止（例如，通过免疫、抗性等）。

- **参数：**
  - `const FActiveGameplayEffectsContainer&`：当前激活的所有游戏效果容器。
  - `const FGameplayEffectSpec&`：要考虑的 `GameplayEffectSpec`，即要判断是否可以应用的效果。



####  13.2 Eunm

##### 13.2.1 EGameplayEffectReplicationMode

用于控制游戏效果（`GameplayEffect`）如何从服务器复制到客户端。它描述了不同的复制级别，从最小的信息到完整的信息，适应不同的网络需求和性能要求。

- `Minimal`：
  - 仅复制最少的游戏效果信息。
  - 这意味着只有必要的、最基本的信息会被同步到客户端，例如效果的启动时间、持续时间等，但不包含完整的效果数据。
  - **注意**：这个模式对于拥有 `AbilitySystemComponent` 的角色不起作用，通常在不涉及 `AbilitySystemComponent` 的情况下使用。如果涉及到 `AbilitySystemComponent`，应该使用 `Mixed` 模式。
- `Mixed`：
  - 对于模拟代理（即客户端上不控制的角色）仅复制最小的游戏效果信息，而对拥有者（即玩家自己控制的角色）和自动代理（如AI或网络中独立控制的对象）复制完整信息。
  - 这种模式适用于大多数情况，它在保持性能的同时保证了重要的信息同步。
- `Full`：
  - 完全复制所有游戏效果信息。
  - 包括效果的所有详细数据和状态，例如效果的持续时间、修改的属性、影响的对象等。适用于对效果状态有严格要求的情况，但可能会增加网络带宽的消耗。

**使用场景**：

- `Minimal` 适用于带宽有限或不需要完整游戏效果信息的情况。

- `Mixed` 是一种折衷方案，适合大多数多人游戏场景。

- `Full` 适用于需要高精度同步的场景，如关键性游戏效果或高度互动的状态。

  

##### 13.2.2 EConsiderPending 

用于在处理待定的操作（例如添加或移除能力）时，如何处理尚未应用的操作。它控制游戏对象在执行操作时是否考虑已排队的、待处理的项。

- `None`：
  - 不考虑任何待处理操作（例如，待处理的能力添加或移除）。
  - 适用于在执行某个操作时忽略待定状态，只关注当前的有效状态。
- `PendingAdd`：
  - 考虑待添加的项目（例如，待添加的能力）。
  - 如果某个能力或游戏效果正在排队等待添加，这个标志表示我们在执行操作时会考虑这些待添加的项目。
- `PendingRemove`：
  - 考虑待移除的项目（例如，待移除的能力）。
  - 如果某个能力或效果正在排队等待移除，这个标志表示我们在执行操作时会考虑这些待移除的项目。
- `All`：
  - 同时考虑待添加和待移除的项目。
  - 这意味着在执行操作时，既会考虑待添加的能力，也会考虑待移除的能力，确保在进行操作时不忽视任何待定的项目。

**使用场景**：

- `None` 适用于不关心待定状态的情况，可能用于在某些操作期间完全忽略那些尚未完成的项目。
- `PendingAdd` 和 `PendingRemove` 可以分别用于需要在执行操作时处理待添加或待移除项的场景。
- `All` 是最全面的选择，适用于需要完全同步待添加和待移除项的操作。



### 14.1 FGameplayEffectAttributeCaptureDefinition

在UE5.4中，`FGameplayEffectAttributeCaptureDefinition`结构体中的`bSnapshot`属性用于控制属性捕获的时间点。具体来说：

- **bSnapshot = true**：当`bSnapshot`为`true`时，属性值在效果应用时被捕获并固定下来。这意味着即使属性在效果持续期间发生变化，捕获的值也不会更新。这种方式适用于需要在效果应用时锁定属性值的情况，例如计算一次性伤害。
- **bSnapshot = false**：当`bSnapshot`为`false`时，属性值在效果持续期间会动态更新。这意味着每次效果计算时都会使用当前的属性值。这种方式适用于需要根据属性的实时变化来调整效果的情况，例如持续治疗或持续伤害效果。

[通过设置`bSnapshot`属性，你可以更灵活地控制Gameplay Effect在不同场景下的行为](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayAbilities/FGameplayEffectAttributeCaptureD-)[1](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayAbilities/FGameplayEffectAttributeCaptureD-)[2](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayAbilities/FGameplayEffectAttributeCaptureD-/__ctor/1)。

​              
