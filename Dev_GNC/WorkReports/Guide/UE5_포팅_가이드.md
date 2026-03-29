# Unity → UE5 포팅 가이드

> **부제**: Unity C# CardGame → UE5 C++ Dev_GNC 아키텍처 매핑 참조 문서

---

## 1. 매니저 → 서브시스템 매핑

| Unity MonoSingleton | UE5 클래스 | Subsystem 타입 | 근거 |
|---|---|---|---|
| DataManager (Singleton) | UDataSubsystem | GameInstance | 앱 수명 동일, 씬 전환에도 유지 |
| TextManager | UTextSubsystem | GameInstance | 로컬라이제이션, 앱 수명 동일 |
| PlayerManager | UPlayerDataSubsystem | GameInstance | 골드, 세이브 등 영속 데이터 |
| PawnManager | UPawnSubsystem | GameInstance | 보유 폰 목록 |
| ItemManager | UItemSubsystem | GameInstance | 장비 목록 |
| DeckManager | UDeckSubsystem | GameInstance | 덱 편성 |
| LobbyManager | ULobbySubsystem | GameInstance | 로비 상태 |
| StageManager | UStageSubsystem | World | 레벨(스테이지)에 종속 |
| TileManager | UGridSubsystem | World | 타일맵은 레벨에 종속 |
| TurnManager | UTurnSubsystem | World | 전투 턴은 레벨에 종속 |
| UIManager (프리팹) | ACardGameHUD (AHUD) | — | PlayerController 소유 |

### 핵심 차이점
- `UWorldSubsystem`은 레벨 전환 시 자동 생성/파괴 → `IResettable.ResetAll()` 불필요
- `UGameInstanceSubsystem`은 `DontDestroyOnLoad` + `MonoSingleton` 수명과 동일
- `AssetPathAttribute` + `Resources.Load` → `TSoftObjectPtr<>` + 비동기 에셋 로딩

---

## 2. 씬 → 레벨/GameMode 매핑

| Unity Scene | UE5 구성 |
|---|---|
| BootScene + BootScript | `UGameInstance::Init()` |
| TitleScene + TitleScript | `Title.umap` + `ACardTitleGameMode` |
| LobbyScene + LobbyScript | `Lobby.umap` + `ACardLobbyGameMode` |
| StageScene + StageScript | `Stage.umap` + `ACardStageGameMode` |
| SceneController (Singleton) | `UGameInstance::OpenLevel()` |
| SceneBase (abstract) | `AGameModeBase` 파생 클래스 |

### 라이프사이클 매핑
- `SceneBase.OnEnterScene()` → `AGameModeBase::InitGame()`
- `SceneBase.OnExitScene()` → `AGameModeBase::EndPlay()`
- `SceneController.ReturnToTitle()` → WorldSubsystem 자동 파괴 + `OpenLevel("Title")`

---

## 3. UI 시스템 매핑

| Unity | UE5 |
|---|---|
| ViewCore (abstract base) | `UCommonActivatableWidget` 또는 커스텀 `UViewBase` |
| PageView (스택 기반) | `UCommonActivatableWidgetStack` |
| PopupView | 별도 레이어의 `UCommonActivatableWidget` |
| UIManager (프리팹) | `ACardGameHUD` + 위젯 스택 2개 (Page, Popup) |
| PrefabLoader + AssetPath | `TSoftClassPtr<UUserWidget>` + 비동기 로드 |

### ViewCore 라이프사이클 매핑
- `PreOpen()` → `NativeOnActivated()`
- `OnOpened()` → `BP_OnActivated`
- `PreClose()` → `NativeOnDeactivated()`
- `OnClosed()` → `RemoveFromParent()` 또는 위젯 소멸

---

## 4. 데이터 시스템 매핑

| Unity | UE5 |
|---|---|
| Protobuf .bytes | `UDataTable` (CSV/JSON 임포트) |
| `Resources.Load<T>(path)` | `TSoftObjectPtr<T>` + `LoadSynchronous()` |
| `GameData.cs` (generated) | `FXxxRow : FTableRowBase` (수동 정의) |
| `EntityCatalog` (ScriptableObject) | `UDataAsset` 파생 |
| `TableLoader` + `Dictionary<int, TRow>` | `UDataTable::FindRow<FRow>(RowName)` |

### 13개 테이블 매핑

| Unity Table | UE5 DataTable | Row Struct |
|---|---|---|
| CardData | DT_Card | FCardDataRow |
| CardEffectData | DT_CardEffect | FCardEffectRow |
| MonsterData | DT_Monster | FMonsterDataRow |
| PawnData | DT_Pawn | FPawnDataRow |
| PawnGrowthData | DT_PawnGrowth | FPawnGrowthRow |
| EquipmentData | DT_Equipment | FEquipmentDataRow |
| ShopData | DT_Shop | FShopDataRow |
| StagePresetData | DT_StagePreset | FStagePresetRow |
| StageRewardData | DT_StageReward | FStageRewardRow |
| NamePresetData | DT_NamePreset | FNamePresetRow |
| TextTable | DT_Text (또는 UE5 StringTable) | FTextRow |
| TileEntityData | DT_TileEntity | FTileEntityRow |
| StagePreset | UDataAsset | — |

---

## 5. 핵심 패턴 변환 표

| Unity C# | UE5 C++ |
|---|---|
| `MonoSingleton<T>` | `UGameInstanceSubsystem` / `UWorldSubsystem` |
| `DontDestroyOnLoad` | GameInstanceSubsystem (자동 영속) |
| `IResettable` + `ResetAll()` | WorldSubsystem 자동 파괴 (불필요) |
| `Coroutine (IEnumerator)` | `FTimerHandle` + 콜백 / `FLatentAction` |
| `[SerializeField]` | `UPROPERTY(EditDefaultsOnly)` |
| `AssetPathAttribute` | `TSoftClassPtr<>` / `TSoftObjectPtr<>` |
| `Vector2Int` | `FIntPoint` |
| `Vector3` | `FVector` |
| `Dictionary<K,V>` | `TMap<K,V>` |
| `List<T>` | `TArray<T>` |
| `event Action<T>` | `DECLARE_DYNAMIC_MULTICAST_DELEGATE` |
| `[System.Serializable]` | `USTRUCT(BlueprintType)` |
| `enum` | `UENUM(BlueprintType)` |
| `abstract class` | `UCLASS(Abstract)` |
| `interface` | `UINTERFACE` + `IXxx` |
| `GetComponent<T>()` | `FindComponentByClass<T>()` |
| `Instantiate(prefab)` | `SpawnActor<T>()` / `CreateWidget<T>()` |
| `Destroy(gameObject)` | `Destroy()` (AActor) / `RemoveFromParent()` (UWidget) |

---

## 6. 타일맵 시스템 매핑

| Unity | UE5 |
|---|---|
| TileManager (MonoSingleton) | `UGridSubsystem` (UWorldSubsystem) |
| SquareTile (MonoBehaviour) | `ASquareTile` (AActor) |
| TileSlot | `UGridSlotComponent` (UActorComponent) |
| TileEntity (abstract) | `AGridEntity` (AActor) |
| EdgeTile | `AEdgeTile` (AActor) |
| Actor (이동/공격) | `ACardGameActor` (AGridEntity 상속) |
| EntityCatalog (ScriptableObject) | `UDataAsset` 파생 |

### 좌표/경로 탐색
- `Vector2Int` → `FIntPoint`
- `Dictionary<Vector2Int, SquareTile>` → `TMap<FIntPoint, ASquareTile*>`
- A* / BFS / Bresenham 알고리즘은 순수 로직이므로 C++로 직역 가능

---

## 7. 전투/턴 시스템 매핑

### 턴 시스템
- `TurnManager` Coroutine 기반 → `UTurnSubsystem` + `FTimerHandle` 콜백 체인
- `ETurnPhase` 상태 머신 (PawnPhase / MonsterPhase)
- Acting Power → 동일 구조 유지

### 카드 효과
- `CardEffectType` 12종 → `UENUM(BlueprintType) ECardEffectType`
- `StageManager.ApplyCardEffectsCoroutine` → `UStageSubsystem::ApplyCardEffects()`
- 버프/디버프 `BuffEntry` → `FBuffEntry` USTRUCT

### 명중률
- 만분율 기반 (10000 = 100%) → 동일 구조 유지

---

## 8. 포팅 순서

### Phase 1: 데이터 기반 ✅ 완료
1. `CardGameTypes.h` — 모든 enum 정의
2. `FTableRowBase` 구조체 13종 정의
3. Protobuf `.bytes` → CSV 변환 스크립트 (Python)
4. `UDataSubsystem` — 테이블 로드 + Get 메서드
5. `UTextSubsystem` — 로컬라이제이션
6. DObject, DPawn, DMonster, DCard, DEquipment UCLASS 정의

### Phase 2: 타일맵 + 그리드 ✅ 완료
1. `FEdgeTileKey` — 엣지 식별 정규화 구조체
2. `UTileMapPreset` — 맵 프리셋 UDataAsset (FTilePresetData, FEdgePresetData)
3. `UEntityCatalog` — 엔티티 카탈로그 UDataAsset (FEntityCatalogEntry, TSoftClassPtr)
4. `UGridSlotComponent` — 타일/엣지 엔티티 배치 슬롯 (UActorComponent)
5. `AGridEntity` — 타일 엔티티 추상 베이스 (AActor, Abstract)
6. `ASquareTile` — 사각 타일 액터 (상태/시각화/델리게이트)
7. `AEdgeTile` — 엣지 타일 액터
8. `ATileWall` — 벽 엔티티 (bIsPassable)
9. `UGridSubsystem` — 타일맵 관리 UWorldSubsystem
   - 맵 생성: GenerateTileMap, GenerateTileMapFromPreset
   - 타일/엣지 조회: GetTile, GetAdjacentTiles, GetTilesInRange, GetEdgeTile
   - 경로탐색: GetReachableTiles(BFS), GetPath(A*), SmoothPath(Greedy+Bresenham)
   - 좌표 변환: GridToWorld, WorldToGrid (XY 평면, Z=Up)

### Phase 3: 전투 + 캐릭터 ✅ 완료
1. `ETurnPhase` — 턴 페이즈 열거형 (CardGameTypes.h에 추가)
2. `ACardGameActor` — 전투 캐릭터 액터 (AGridEntity 상속)
   - Tick 기반 이동 보간 (MoveTo, FaceTarget)
   - AnimMontage 콜백 (PerformAttack, ReceiveHit, Evade, Die)
   - InitFromPawn/InitFromMonster, FloatingText 연결
3. `UTurnSubsystem` — 턴 관리 UWorldSubsystem
   - 턴 페이즈 (Pawn/Monster), SharedActingPower (5/턴)
   - 이동 추적 (OnActorMoved, GetRemainingMovement, AdjustRemainingMovement)
   - 몬스터 AI 콜백 체인 (Alert: 접근+공격, Wander: 랜덤이동)
   - ExecuteAttack (공격/피격 애니메이션 병렬 + 0.3초 타이머 피해 적용)
4. `UStageSubsystem` — 스테이지 관리 UWorldSubsystem
   - SelectTile 3단 분기 (재클릭 해제 / 이동 / 새 선택)
   - 이동 범위/카드 범위/반경 프리뷰 시각화
   - TryUseCard, UseCardSelf — 비용 체크, 탄약 소모
   - 카드 효과 12종 순차 적용 (ProcessNextEffect 콜백 체인)
   - 명중 판정 (만분율 Accuracy), 사망 체크, 전투 결과 (승리/패배)
   - LoadStage — 프리셋 기반 폰/몬스터 스폰, 보상 누적

### Phase 4: 카드/덱 시스템 ✅ 완료
1. `UPlayerDataSubsystem` — 플레이어 메타 데이터 UGameInstanceSubsystem
   - 골드 관리 (AddGold, SpendGold, HasEnoughGold)
   - 난이도 레벨 관리 (IncrementDifficulty)
2. `UPawnSubsystem` — 보유 폰 목록 UGameInstanceSubsystem
   - AddPawn, RemovePawn, GetPawns
3. `UItemSubsystem` — 보유 장비 목록 UGameInstanceSubsystem
   - AddEquip, RemoveEquip, GetEquips
4. `UDeckSubsystem` — 덱 편성 + 글로벌 카드 덱 UGameInstanceSubsystem
   - Pawn 편성 슬롯 (5개, 초기 2개 오픈, ExpandSlot)
   - 글로벌 카드 덱 (DeckCardList/CardPool/DiscardPool/HandCardList)
   - InitStage (셔플+5장 드로우), StartTurnDraw (턴당 1장)
   - DrawCards, UseCard (Supply/Consume 폐기, Stable→DiscardPool)
5. `UTurnSubsystem` 수정 — StartPawnTurn에서 DeckSubsystem 글로벌 드로우 연동
6. `UStageSubsystem` 수정
   - UseCardSelf/TryUseCard: CardOwner null 시 DeckSubsystem.UseCard 폴백
   - LoadStage: DeckSubsystem.InitStage 호출 추가
   - SpawnPawns: DeckSubsystem 편성 폰 기반 스폰 (ResetStats+BuildCardPool+InitHand)
   - ProcessNextEffect DrawCard: 글로벌 덱 드로우 추가

### Phase 5: UI

#### Phase 5-A: 핵심 인프라 ✅ 완료
1. `CardGameUITypes.h` — FViewParam 구조체 (뷰 파라미터 전달)
2. `UViewBase` — 추상 위젯 베이스, 라이프사이클 (NativePreOpen/OnOpened/PreClose/OnClosed)
3. `UPageWidget` — 페이지 베이스, bKeepPreviousVisible (Unity PageView 대응)
4. `UPopupWidget` — 팝업 베이스, PopupZOrder (Unity PopupView 대응)
5. `UFloatingTextWidget` — 플로팅 텍스트, 풀링 재사용, 월드→스크린 프로젝션
6. `ACardGameHUD` — 뷰 스택 관리자 (Unity UIManager+HudController 대응)
   - 페이지 스택 (OpenPage/ClosePage/CloseAllPages)
   - 팝업 관리 (OpenPopup/ClosePopup/CloseAllPopups, 다중 공존)
   - 플로팅 텍스트 풀 (10개 사전 생성, 라운드 로빈)

#### Phase 5-B: 전투 UI ✅ 완료
1. `UCardDragDropOp` — 카드 드래그 payload (UDragDropOperation, 헤더 온리)
2. `UBattleCardWidget` — 핸드 카드 1장 표시 + UE5 네이티브 드래그-드롭
3. `UHUDActorWidget` — 월드 스페이스 HP바/Shield/Armor/Name (UWidgetComponent Screen Space)
4. `UStagePageWidget` — 전투 HUD 페이지 (카드 핸드 동기화, 턴/에너지, 드래그 처리)
5. `ACardGameActor` 수정 — UWidgetComponent(HUDActorWidget) 추가 + HandleFloatingText→HUD 연결

#### Phase 5-C: 로비/타이틀 ✅ 완료
1. `ULobbySubsystem` — 상점/모집 데이터 생성 UGameInstanceSubsystem
   - 상점: GenerateShopList (ShopLevel→CardId배열), LevelUpShop, RemoveShopCard
   - 모집: GenerateRecruitList (랜덤 3폰), RemoveRecruitPawn
   - EnsureLobbyData (로비 진입 시 자동 생성), ResetForNextStage, ResetAll
2. `UTitlePageWidget` — 타이틀 페이지 (NewGame/LoadGame/Option/Exit)
   - NewGame: 전 서브시스템 ResetAll → OpenLevel("Lobby")
3. `ULobbyPageWidget` — 로비 페이지 (StartBattle/DeckSetting/PawnManage/Shop/Recruit/Exit)
   - EnsureTestData (Pawn 없으면 DeckSubsystem.InitTestData)
   - StartBattle: GetActivePawns 체크 → OpenLevel("Stage")
4. `ACardGameHUD` 수정 — InitialPageClass 프로퍼티 (레벨별 초기 페이지 자동 열기)
5. `UDeckSubsystem` 수정 — InitTestData() 추가 (폰2+카드6+장비11+편성)

#### Phase 5-D: 관리 페이지 (✅ 완료)
> DPawn Equip/Unequip 추가, 아이템 위젯 3종, 관리 페이지 4종, LobbyPageWidget 연결

0. `UDPawn` 수정 — Equip/Unequip 메서드 추가 (장비 착탈 + RecalculateStats)
1. 아이템 위젯 3종: `UPawnListItemWidget`, `UEquipListItemWidget`, `UShopCardItemWidget`
2. `UDeckSettingPageWidget` — 덱 편성 (탭 전환, 폰 슬롯 5개 + 카드 목록)
3. `UPawnManagePageWidget` — 폰 관리 (폰 선택 + 스탯 + 장비 슬롯 4개)
4. `UShopPageWidget` — 상점 (카드 구매 + 레벨업)
5. `URecruitPageWidget` — 모집 (폰 3개 슬롯 + 리롤)
6. `ULobbyPageWidget` 수정 — 4개 스텁 핸들러를 OpenPage로 교체
7. `UDeckSubsystem` 수정 — GetDeckCards() getter 추가

#### Phase 5-E: 팝업 ✅ 완료
1. `UPausePopupWidget` — 일시정지 (Continue/Option/Exit)
2. `UNoticePopupWidget` — 범용 확인/취소 (FOnNoticeConfirm/Cancel 델리게이트)
3. `UBattleResultPopupWidget` — 전투 결과 + 보상 카드 선택 (가중치 랜덤 3장, 토글 선택)

### Phase 6: 씬 흐름 + 카메라 ✅ 완료
> 에셋 작업 (3D 모델/애니메이션/머티리얼/텍스처)은 에디터 수동 작업으로 분리.
> 에디터 설정 가이드: `WorkReports/Guide/UE5_에디터_설정_가이드.md`

1. `ACardGameMode` — 공통 베이스 GameMode (HUD/PC/CameraPawn 클래스 설정)
2. `ACardStageGameMode` — 전투 전용 GameMode (난이도별 프리셋 로드 → LoadStage → 카메라 피벗)
3. `ACardGamePlayerController` — 마우스 커서 + GameAndUI 입력 모드
4. `ACardGameCameraPawn` — 탑다운 전략 카메라 (Pan/Rotate/Zoom, Enhanced Input, SpringArm)
5. UI 씬 전환 스텁 완성:
   - `UPausePopupWidget` Exit → ResetAll + OpenLevel("Title")
   - `UBattleResultPopupWidget` Continue → OpenLevel("Lobby"), Exit → ResetAll + OpenLevel("Title")
   - `ULobbyPageWidget` Exit → ResetAll + OpenLevel("Title")

---

## 9. Source 폴더 구조

```
Source/Dev_GNC/CardGame/
├── CardGameMode.h/cpp              // ✅ Phase 6 — 공통 베이스 GameMode
├── CardStageGameMode.h/cpp         // ✅ Phase 6 — 전투 전용 GameMode
├── CardGamePlayerController.h/cpp  // ✅ Phase 6 — 마우스 커서 + 입력 모드
├── CardGameCameraPawn.h/cpp        // ✅ Phase 6 — 탑다운 전략 카메라 (Enhanced Input)
├── Subsystem/
│   ├── DataSubsystem.h/cpp         // ✅ Phase 1 — UGameInstanceSubsystem
│   ├── TextSubsystem.h/cpp         // ✅ Phase 1 — UGameInstanceSubsystem
│   ├── GridSubsystem.h/cpp         // ✅ Phase 2 — UWorldSubsystem
│   ├── TurnSubsystem.h/cpp         // ✅ Phase 3 — UWorldSubsystem
│   ├── StageSubsystem.h/cpp        // ✅ Phase 3 — UWorldSubsystem
│   ├── PlayerDataSubsystem.h/cpp   // ✅ Phase 4 — UGameInstanceSubsystem
│   ├── PawnSubsystem.h/cpp         // ✅ Phase 4 — UGameInstanceSubsystem
│   ├── ItemSubsystem.h/cpp         // ✅ Phase 4 — UGameInstanceSubsystem
│   ├── DeckSubsystem.h/cpp         // ✅ Phase 4 — UGameInstanceSubsystem
│   └── LobbySubsystem.h/cpp       // ✅ Phase 5-C — UGameInstanceSubsystem 상점/모집
├── Data/
│   ├── CardGameTypes.h              // ✅ Phase 1+3 — Enum 정의 (ETurnPhase 추가)
│   ├── CardGameRowTypes.h/cpp       // ✅ Phase 1 — FTableRowBase 구조체 13종
│   ├── DObject.h/cpp                // ✅ Phase 1
│   ├── DPawn.h/cpp                  // ✅ Phase 1
│   ├── DMonster.h/cpp               // ✅ Phase 1
│   ├── DCard.h/cpp                  // ✅ Phase 1
│   ├── DEquipment.h/cpp             // ✅ Phase 1
│   ├── BuffEntry.h                  // ✅ Phase 1
│   ├── FPawnStats.h/cpp             // ✅ Phase 1
│   ├── TileMapPreset.h/cpp          // ✅ Phase 2 — UDataAsset
│   └── EntityCatalog.h/cpp          // ✅ Phase 2 — UDataAsset
├── Grid/
│   ├── EdgeTileKey.h                // ✅ Phase 2 — 헤더 온리
│   ├── GridSlotComponent.h/cpp      // ✅ Phase 2
│   ├── GridEntity.h/cpp             // ✅ Phase 2 — Abstract AActor
│   ├── SquareTile.h/cpp             // ✅ Phase 2
│   ├── EdgeTile.h/cpp               // ✅ Phase 2
│   └── CardGameActor.h/cpp          // ✅ Phase 3+5-B — 전투 캐릭터 액터 + UWidgetComponent + FloatingText
├── Entity/
│   ├── TileWall.h/cpp               // ✅ Phase 2
│   ├── TilePawn.h/cpp               // Phase 3
│   └── TileItem.h/cpp               // Phase 3+
└── UI/
    ├── CardGameUITypes.h            // ✅ Phase 5-A — FViewParam
    ├── CardGameHUD.h/cpp            // ✅ Phase 5-A — AHUD, 뷰 스택 관리자
    ├── ViewBase.h/cpp               // ✅ Phase 5-A — 추상 위젯 베이스
    ├── PageWidget.h/cpp             // ✅ Phase 5-A — 페이지 베이스
    ├── PopupWidget.h/cpp            // ✅ Phase 5-A — 팝업 베이스
    ├── FloatingTextWidget.h/cpp     // ✅ Phase 5-A — 플로팅 텍스트
    ├── CardDragDropOp.h             // ✅ Phase 5-B — 드래그 payload (헤더 온리)
    ├── StagePageWidget.h/cpp        // ✅ Phase 5-B — 전투 HUD 페이지
    ├── BattleCardWidget.h/cpp       // ✅ Phase 5-B — 핸드 카드 위젯 + 드래그-드롭
    ├── HUDActorWidget.h/cpp         // ✅ Phase 5-B — 월드 스페이스 HP바/이름
    ├── TitlePageWidget.h/cpp        // ✅ Phase 5-C — 타이틀 페이지
    ├── LobbyPageWidget.h/cpp        // ✅ Phase 5-C — 로비 페이지
    ├── PawnListItemWidget.h/cpp     // ✅ Phase 5-D — 폰 목록 아이템 위젯
    ├── EquipListItemWidget.h/cpp    // ✅ Phase 5-D — 장비 목록 아이템 위젯
    ├── ShopCardItemWidget.h/cpp     // ✅ Phase 5-D — 상점 카드 아이템 위젯
    ├── DeckSettingPageWidget.h/cpp  // ✅ Phase 5-D — 덱 편성 페이지
    ├── PawnManagePageWidget.h/cpp   // ✅ Phase 5-D — 폰 관리 페이지
    ├── ShopPageWidget.h/cpp         // ✅ Phase 5-D — 상점 페이지
    ├── RecruitPageWidget.h/cpp      // ✅ Phase 5-D — 모집 페이지
    ├── PausePopupWidget.h/cpp       // ✅ Phase 5-E — 일시정지 팝업
    ├── NoticePopupWidget.h/cpp      // ✅ Phase 5-E — 범용 확인/취소 팝업
    └── BattleResultPopupWidget.h/cpp // ✅ Phase 5-E — 전투 결과 + 보상 카드 선택
```

---

## 10. Content 폴더 구조

```
Content/CardGame/
├── DataTables/              // DT_Card, DT_Monster 등 12종
├── DataAssets/              // TileMapPreset, EntityCatalog
├── Blueprints/
│   ├── GameMode/            // BP_CardGameMode, BP_CardStageGameMode
│   ├── UI/                  // UMG 위젯 블루프린트 (Page/Popup/Item 위젯)
│   ├── Camera/              // BP_CardGameCameraPawn
│   └── Animation/           // AnimBP, AnimMontage
├── Input/                   // IA_Pan, IA_Rotate, IA_Zoom, IMC_CardGame
├── Maps/
│   ├── Title.umap           // ACardGameMode, InitialPage=TitlePage
│   ├── Lobby.umap           // ACardGameMode, InitialPage=LobbyPage
│   └── Stage.umap           // ACardStageGameMode, InitialPage=StagePage
├── Materials/
└── Textures/
```
