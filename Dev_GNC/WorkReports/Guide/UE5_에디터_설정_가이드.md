# UE5 에디터 설정 가이드

> **목적**: C++ 코드 포팅 완료 후, UE5 에디터에서 수행해야 할 설정/에셋 생성 작업을 전체 Phase 기준으로 정리한 체크리스트.
> **참조**: `UE5_포팅_가이드.md` (아키텍처 매핑), 각 Phase별 WorkReport

---

## 1. 프로젝트 세팅

### 1-1. 기본 맵
- [ ] **게임 기본 맵**: `/CardGame/Maps/Title` (게임 시작 시 타이틀 화면)
- [ ] **에디터 시작 맵**: 개발 편의상 Stage 또는 Lobby 맵 지정 가능

### 1-2. GameInstance
- [ ] 기본 `UGameInstance` 사용 (커스텀 불필요 — 6개 GameInstanceSubsystem이 자동 등록)

### 1-3. 입력
- [ ] **기본 입력 모드**: 기본값 유지 (ACardGamePlayerController에서 BeginPlay 시 설정)

---

## 2. 맵 생성 (3개)

### 2-1. Title.umap
- [x] `Content/CardGame/Maps/Title` 에 빈 레벨 생성
- [ ] 월드 세팅 → 게임모드 오버라이드 = `BP_CardTitleGameMode`
- [ ] 배경 요소 배치 (선택사항: 스카이박스, 라이트, 배경 메시)

### 2-2. Lobby.umap
- [x] `Content/CardGame/Maps/Lobby` 에 빈 레벨 생성
- [ ] 월드 세팅 → 게임모드 오버라이드 = `BP_CardLobbyGameMode`
- [ ] 배경 요소 배치 (선택사항)

### 2-3. Stage.umap
- [x] `Content/CardGame/Maps/Stage` 에 빈 레벨 생성
- [ ] 월드 세팅 → 게임모드 오버라이드 = `BP_CardStageGameMode`
- [ ] **디렉셔널 라이트** + **스카이 애트머스피어** 추가 (3D 타일맵 조명)
- [ ] **플레이어 스타트** 액터 배치 (CameraPawn 스폰 위치 — StageGameMode가 이후 SetPivot으로 이동)

---

## 3. 게임모드 블루프린트 (4개 — 생성 완료)

### 3-1. BP_CardGameMode (베이스)
- [x] `Content/CardGame/Blueprints/GameMode/` 에 생성
- [ ] 부모 클래스: `ACardGameMode` (확인 필요)
- [ ] **기본 폰 클래스**: `BP_CardGameCameraPawn` (Title/Lobby에서는 카메라 불필요 시 None)
- [ ] **HUD 클래스**: `ACardGameHUD` (이미 C++ 기본값으로 설정됨, 확인만)

### 3-2. BP_CardTitleGameMode (Title 전용)
- [x] `Content/CardGame/Blueprints/GameMode/` 에 생성
- [ ] HUD Class 내 **InitialPageClass** = `WBP_TitlePage`

### 3-3. BP_CardLobbyGameMode (Lobby 전용)
- [x] `Content/CardGame/Blueprints/GameMode/` 에 생성
- [ ] HUD Class 내 **InitialPageClass** = `WBP_LobbyPage`

### 3-4. BP_CardStageGameMode (Stage 전용)
- [x] `Content/CardGame/Blueprints/GameMode/` 에 생성
- [ ] 부모 클래스: `ACardStageGameMode` (확인 필요)
- [ ] InitialPageClass = `WBP_StagePage` (HUD 설정에서)

> **핵심**: 각 맵마다 HUD의 `InitialPageClass`가 다르게 설정되어야 함.
> 방법 1) 게임모드 BP를 맵별로 만들어 HUD 설정 분리
> 방법 2) HUD BP를 맵별로 만들어 InitialPageClass 분리
> 방법 3) 하나의 게임모드에서 BeginPlay 시 InitialPageClass를 오버라이드
> → **추천: 방법 1** (게임모드 BP를 3개 만들어 각 맵에 할당)

---

## 4. 플레이어 컨트롤러 / 카메라 폰 블루프린트

### 4-1. BP_CardGameCameraPawn
- [ ] `Content/CardGame/Blueprints/Camera/` 에 생성
- [ ] 부모 클래스: `ACardGameCameraPawn`
- [ ] 디테일 패널에서 설정:
  - **CameraMappingContext**: `IMC_CardGame` (아래 5번에서 생성)
  - **PanAction**: `IA_CameraPan`
  - **RotateAction**: `IA_CameraRotate`
  - **ZoomAction**: `IA_CameraZoom`
- [ ] 카메라 기본값 조정 (필요 시):
  - PanSpeed, MinZoomDistance, MaxZoomDistance, DefaultZoomDistance, CameraElevation
- [ ] Title/Lobby에서 카메라 폰이 불필요하면 기본 폰 클래스를 None으로 변경 가능

---

## 5. 향상된 입력 에셋 (4개)

경로: `Content/CardGame/Input/`

### 5-1. 입력 액션 3종 — 생성 완료
| 에셋 이름 | 값 유형 | 설명 | 상태 |
|-----------|---------|------|------|
| `IA_CameraPan` | Axis2D (Vector2D) | 우클릭 드래그 팬 이동 | ✅ 생성됨 |
| `IA_CameraRotate` | Axis1D (Float) | Q/E 키 회전 | ✅ 생성됨 |
| `IA_CameraZoom` | Axis1D (Float) | 마우스 스크롤 줌 | ✅ 생성됨 |

### 5-2. 입력 매핑 컨텍스트 (IMC_CardGame) — 생성 완료
- [ ] 매핑이 올바르게 설정되었는지 확인:

| 액션 | 키 | 모디파이어 | 트리거 |
|------|-----|-----------|--------|
| IA_CameraPan | Mouse XY 2D-Axis | — | 우클릭 홀드 중에만 활성 |
| IA_CameraRotate | Q | 부정 | 눌림 |
| IA_CameraRotate | E | — | 눌림 |
| IA_CameraZoom | Mouse Wheel Axis | — | — |

> **팬 입력 설정 팁**: IA_CameraPan은 Mouse XY를 바인딩하되, **코디드 액션**으로 우클릭 홀드를 조건으로 설정하면 우클릭 드래그 시에만 팬이 동작함.

---

## 6. UI 위젯 블루프린트 (Phase 5 전체)

경로: `Content/CardGame/Blueprints/UI/`

> 모든 위젯 BP는 C++ 추상 클래스를 부모로 하여 생성.
> BindWidget 프로퍼티에 맞는 UMG 위젯을 배치해야 함.

### 6-1. 기반 위젯
| BP 이름 | 부모 C++ 클래스 | 필수 바인드위젯 |
|---------|------------------|-----------------|
| `WBP_FloatingText` | UFloatingTextWidget | TextContent |

### 6-2. HUD
| BP 이름 | 부모 C++ 클래스 | 필수 세팅 |
|---------|------------------|-----------|
| `WBP_CardGameHUD` | — | ACardGameHUD에서 직접 관리. HUD 자체는 AHUD이므로 BP 불필요. 단, **InitialPageClass**와 **FloatingTextClass**, **PopupLayer/PageLayer** 위젯 레이아웃이 필요하면 HUD BP 생성 |

### 6-3. 페이지 위젯 (7종)
| BP 이름 | 부모 C++ 클래스 | 주요 바인드위젯 |
|---------|------------------|-----------------|
| `WBP_TitlePage` | UTitlePageWidget | ButtonNewGame, ButtonLoadGame, ButtonOption, ButtonExit |
| `WBP_LobbyPage` | ULobbyPageWidget | ButtonStartBattle, ButtonDeckSetting, ButtonPawnManage, ButtonShop, ButtonRecruit, ButtonExit, TextGold(opt), TextPawnCount(opt) |
| `WBP_StagePage` | UStagePageWidget | (전투 UI 요소: 카드 핸드 패널, 턴/에너지 표시, 일시정지 버튼 등) |
| `WBP_DeckSettingPage` | UDeckSettingPageWidget | ButtonPawnTab, ButtonCardTab, ButtonClose, PanelPawnTab, PanelCardTab, ButtonSlot0~4, TextSlot0~4, ScrollOwnedPawns, ScrollDeckCards |
| `WBP_PawnManagePage` | UPawnManagePageWidget | (폰 선택 리스트, 스탯 표시, 장비 슬롯 4개) |
| `WBP_ShopPage` | UShopPageWidget | (카드 구매 리스트, 레벨업 버튼, 골드 표시) |
| `WBP_RecruitPage` | URecruitPageWidget | (폰 3개 슬롯, 리롤 버튼, 비용 표시) |

### 6-4. 팝업 위젯 (3종)
| BP 이름 | 부모 C++ 클래스 | 주요 바인드위젯 |
|---------|------------------|-----------------|
| `WBP_PausePopup` | UPausePopupWidget | ButtonContinue, ButtonOption, ButtonExit |
| `WBP_NoticePopup` | UNoticePopupWidget | TextTitle, TextBody, ButtonConfirm, ButtonCancel |
| `WBP_BattleResultPopup` | UBattleResultPopupWidget | TextTitle, TextRewardGold, TextRewardExp, ButtonContinue, ButtonExit, RewardCardPanel, ButtonRewardCard0~2, TextRewardCard0~2 |

### 6-5. 아이템/리스트 위젯 (4종)
| BP 이름 | 부모 C++ 클래스 | 주요 바인드위젯 |
|---------|------------------|-----------------|
| `WBP_BattleCard` | UBattleCardWidget | TextEnergyCost, TextCardName, ImageDimmed 등 |
| `WBP_PawnListItem` | UPawnListItemWidget | TextName, ButtonSelect |
| `WBP_EquipListItem` | UEquipListItemWidget | TextName, TextSlotType, ButtonSelect |
| `WBP_ShopCardItem` | UShopCardItemWidget | (카드 이름, 비용, 구매 버튼) |

### 6-6. HUD 액터 위젯
| BP 이름 | 부모 C++ 클래스 | 주요 바인드위젯 |
|---------|------------------|-----------------|
| `WBP_HUDActorWidget` | UHUDActorWidget | (HP바, 쉴드, 아머, 이름 텍스트) |

### 6-7. 위젯 간 참조 설정
위젯 BP 생성 후 아래 클래스 기본값 프로퍼티를 설정해야 함:

| 위치 | 프로퍼티 | 값 |
|------|---------|-----|
| ACardGameHUD (BP 또는 게임모드에서) | InitialPageClass | 맵별 다름 (Title/Lobby/Stage) |
| ACardGameHUD | FloatingTextClass | WBP_FloatingText |
| ULobbyPageWidget (WBP_LobbyPage) | DeckSettingPageClass | WBP_DeckSettingPage |
| ULobbyPageWidget | PawnManagePageClass | WBP_PawnManagePage |
| ULobbyPageWidget | ShopPageClass | WBP_ShopPage |
| ULobbyPageWidget | RecruitPageClass | WBP_RecruitPage |
| UDeckSettingPageWidget | PawnListItemClass | WBP_PawnListItem |
| UStagePageWidget | BattleCardClass | WBP_BattleCard |
| UStagePageWidget | PausePopupClass | WBP_PausePopup |
| UStagePageWidget | BattleResultPopupClass | WBP_BattleResultPopup |
| ACardGameActor (BP 또는 엔티티 카탈로그) | HUDActorWidgetClass | WBP_HUDActorWidget |

---

## 7. 데이터 테이블 에셋 (Phase 1)

경로: `Content/CardGame/DataTables/`

| 에셋 이름 | 행 구조체 | 상태 |
|-----------|-----------|------|
| DT_CardData | FCardDataRow | ✅ 생성됨 |
| DT_CardEffectData | FCardEffectRow | ✅ 생성됨 |
| DT_MonsterData | FMonsterDataRow | ✅ 생성됨 |
| DT_PawnData | FPawnDataRow | ✅ 생성됨 |
| DT_PawnGrowthData | FPawnGrowthRow | ✅ 생성됨 |
| DT_EquipmentData | FEquipmentDataRow | ✅ 생성됨 |
| DT_ShopData | FShopDataRow | ✅ 생성됨 |
| DT_StagePresetData | FStagePresetRow | ✅ 생성됨 |
| DT_StageRewardData | FStageRewardRow | ✅ 생성됨 |
| DT_NamePresetData | FNamePresetRow | ✅ 생성됨 |
| DT_TextData | FTextRow | ✅ 생성됨 |
| DT_TileEntityData | FTileEntityRow | ✅ 생성됨 |

- [ ] DataSubsystem의 하드코딩 경로와 실제 에셋 경로가 일치하는지 확인
- [ ] CSV 데이터가 올바르게 임포트되었는지 확인

---

## 8. 데이터 에셋 (Phase 2)

경로: `Content/CardGame/DataAssets/`

### 8-1. TileMapPreset (스테이지별 1개 이상)
- [ ] UTileMapPreset 파생 데이터 에셋 생성
- [ ] Width, Height, TileSize, TurnLimit, MaxPawnCount 설정
- [ ] Tiles 배열에 FTilePresetData 채우기 (타일 타입 + 스폰 포인트)
- [ ] Edges 배열에 FEdgePresetData 채우기 (벽 위치)
- [ ] DT_StagePresetData의 PresetPath 배열에 데이터 에셋 경로 등록

### 8-2. EntityCatalog
- [ ] UEntityCatalog 파생 데이터 에셋 생성
- [ ] 엔티티 타입별 TSoftClassPtr 등록 (Pawn 메시, Monster 메시, TileWall 등)

---

## 9. 3D 에셋 (Phase 6 에셋 범위)

### 9-1. 캐릭터 메시
- [ ] Unity FBX → UE5 재임포트
- [ ] 스켈레탈 메시 + 스켈레톤 설정
- [ ] 피직스 에셋 생성

### 9-2. 애니메이션
- [ ] 애님 몽타주 생성: Attack, Hit, Evade, Die
- [ ] 애니메이션 블루프린트 생성: Idle/Move 기본 스테이트 머신
- [ ] ACardGameActor의 애님 몽타주 프로퍼티에 연결

### 9-3. 타일 메시
- [ ] SquareTile 시각 메시 (스태틱 메시 또는 인스턴스드 스태틱 메시)
- [ ] 타일 상태별 머티리얼 (Normal, Selected, Highlighted, Disabled)

### 9-4. 머티리얼 / 텍스처
- [ ] Unity 머티리얼 → UE5 머티리얼 재생성
- [ ] 텍스처 임포트 (UI 아이콘, 배경 등)

---

## 10. 최소 실행 체크리스트

에디터 세팅의 **최소 필수 항목**만 정리. 이것만 완료하면 게임 루프가 동작함.

### 필수 (게임 루프 동작에 반드시 필요)
- [x] 맵 3개 생성 (Title, Lobby, Stage)
- [x] 게임모드 BP 4개 생성 (Base, Title, Lobby, Stage) — 맵별 할당은 미확인
- [x] 입력 에셋 4개 생성 (IA 3종 + IMC 1종)
- [ ] BP_CardGameCameraPawn 생성 + 입력 에셋 연결
- [ ] GameMode를 각 맵의 월드 세팅에 할당
- [ ] HUD InitialPageClass 맵별 설정
- [ ] 위젯 BP 최소 7종 생성 (TitlePage, LobbyPage, StagePage, BattleCard, PausePopup, BattleResultPopup, FloatingText)
- [ ] TileMapPreset 데이터 에셋 최소 1개 생성 + DT_StagePresetData에 경로 등록
- [ ] 프로젝트 세팅 → 기본 맵 = Title

### 권장 (기능 완성도)
- [ ] 나머지 위젯 BP 생성 (DeckSetting, PawnManage, Shop, Recruit, NoticePopup, 아이템 위젯 3종, HUDActorWidget)
- [ ] 위젯 간 참조 설정 (6-7절 참조)
- [ ] EntityCatalog 데이터 에셋 생성

### 선택 (비주얼)
- [ ] 캐릭터 메시 + 애니메이션 임포트
- [ ] 타일 메시 + 머티리얼 설정
- [ ] UI 텍스처/아이콘 임포트
- [ ] 맵 배경 (라이팅, 스카이박스, 배경 메시)
