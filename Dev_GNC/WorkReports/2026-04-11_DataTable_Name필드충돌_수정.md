# DataTable Row 필드 `Name`/`Desc` 충돌 수정

- **일시**: 2026-04-11
- **분류**: 버그 수정 (데이터 파이프라인)
- **범위**: CardGameRowTypes.h(자동 재생성), DataSubsystem.cpp, BattleCardWidget.cpp, BattleResultPopupWidget.cpp, HUDActorWidget.cpp, Excel 원본 테이블(사용자 담당)

---

## 배경

`DT_NamePresetData.uasset`의 `Name` 컬럼 값이 원본 Excel과 다르게 오염되는 현상이 발견되었다. 일부 행은 실제 텍스트 키(`text_name_preset_alpha` 등)가 들어가 있지만, 다른 행은 RowName 값(`Row_1`, `Row_2` 등)이 들어가 있었다. 동일 구조의 다른 DataTable들(CardData, PawnData, MonsterData 등)도 같은 증상일 가능성이 있었다.

## 원인 분석

### 핵심: UE5 DataTable 예약 키와 UPROPERTY 이름 충돌

UE5의 DataTable JSON 임포트는 JSON 객체의 `"Name"` 키를 **RowName**(행 키)로 예약한다. 이를 맞추기 위해 `Tools/json_exporter.py:193-194`가 모든 행에 `d["Name"] = row_name`을 삽입하고 있었다.

```python
row_name = d.get("idAlias") or f"Row_{d.get('id', 0)}"
d["Name"] = row_name
```

동시에 proto 직렬화는 `preserving_proto_field_name=True` 옵션으로 원본 `name` 컬럼을 JSON에 `"name"`(소문자)로 내보내고 있었다. 결과적으로 단일 JSON 행에 **`"name"`과 `"Name"` 두 키가 공존**하게 된다:

```json
{ "id": 1, "name": "text_name_preset_alpha", "Name": "Row_1" }
```

이제 UE5가 `FNamePresetRow::Name`(UPROPERTY)을 채울 때 `FJsonObjectConverter::JsonObjectToUStruct`가 동작한다. 이 변환기는 프로퍼티명을 **case-insensitive**로 JSON 키와 매칭한다. `"name"`과 `"Name"`은 대소문자만 다르므로 둘 다 `Name` 프로퍼티에 매칭되며, `TMap` 해시 순회 순서에 따라 어느 쪽 값이 최종 적용될지 비결정적으로 결정된다. 이것이 오염의 실체였다.

### 영향 범위

`CardGameRowTypes.h` 내 `FString Name` UPROPERTY를 가진 7개 struct 전부가 같은 충돌에 노출:

| Row Struct | `Name` | `Desc` |
|---|---|---|
| FCardDataRow | ✓ | ✓ |
| FCardEffectRow | ✓ | - |
| FEquipmentDataRow | ✓ | - |
| FMonsterDataRow | ✓ | - |
| FNamePresetRow | ✓ | - |
| FPawnDataRow | ✓ | - |
| FTileEntityRow | ✓ | - |

NamePresetData가 가장 먼저 발견된 것은 필드가 단 2개뿐이라 오염이 즉시 눈에 띄었기 때문이며, 실제로는 다른 DataTable들도 같은 메커니즘으로 오염되어 있었다.

## 해결 방향

두 가지 선택지가 있었다:

1. 생성기 측에서 UPROPERTY명을 `DisplayName` 등으로 강제 override
2. Excel 원본 컬럼명 자체를 `nameAlias` / `descAlias`로 변경해서 충돌을 뿌리째 제거

**후자를 선택**했다. 이유는:

- 이 필드들의 실제 값은 `text_card_name_shoot`, `text_pawn_name_none` 같은 **로컬라이즈 키(alias)**다. `Name` / `Desc`라는 이름은 실제 의미와 부합하지 않았다.
- Excel을 직접 수정하면 파이프라인 전체(proto → JSON → C++ struct)가 스키마 기반으로 자동 전파되어 생성기 수정이 불필요하다.
- `Tools/` 디렉터리 내에 `"name"` / `"desc"` 하드코딩 참조가 전혀 없음을 grep으로 확인했다.

## 변경 사항

### 1. Excel 원본 테이블 (사용자 담당)

- 모든 테이블의 `name` 컬럼 → `nameAlias`
- `FCardDataRow`의 `desc` 컬럼 → `descAlias`

### 2. 파이프라인 자동 전파

Excel 수정 후 `Tools/RunTableTool.bat` 실행 시:

- `proto_generator.py` → 새 필드명으로 `.proto` 재생성
- `data_serializer.py` → `.bytes` 재생성
- `json_exporter.py` → `"nameAlias"` / `"descAlias"` 키로 JSON export
- `ue_codegen.py`의 `_to_pascal()` → `nameAlias` → `NameAlias` (별도 override 불필요)
- `CardGameRowTypes.h` 재생성: `FString Name` → `FString NameAlias`, `FString Desc` → `FString DescAlias`
- `ImportDataTables.py` → DT 에셋 재임포트

JSON에는 `"Name"`(RowName)과 `"nameAlias"`(실제 값)만 존재하므로 case-insensitive 매칭 충돌이 소멸한다.

### 3. C++ 호출부 업데이트 (본 작업)

Row struct 포인터를 통해 `Name` / `Desc`를 읽던 5개 호출부를 새 필드명으로 교체:

| 파일 | 라인 | 변경 |
|---|---|---|
| [Subsystem/DataSubsystem.cpp](../Source/Dev_GNC/CardGame/Subsystem/DataSubsystem.cpp#L222) | 222 | `NamePresetList[Index]->Name` → `NamePresetList[Index]->NameAlias` |
| [UI/BattleResultPopupWidget.cpp](../Source/Dev_GNC/CardGame/UI/BattleResultPopupWidget.cpp#L136) | 136 | `CardRow->Name` → `CardRow->NameAlias` |
| [UI/BattleCardWidget.cpp](../Source/Dev_GNC/CardGame/UI/BattleCardWidget.cpp#L28) | 28 | `Data->Name` → `Data->NameAlias` |
| [UI/BattleCardWidget.cpp](../Source/Dev_GNC/CardGame/UI/BattleCardWidget.cpp#L29) | 29 | `Data->Desc` → `Data->DescAlias` |
| [UI/HUDActorWidget.cpp](../Source/Dev_GNC/CardGame/UI/HUDActorWidget.cpp#L68) | 68 | `Monster->Data->Name` → `Monster->Data->NameAlias` |

`rg "->Name|->Desc"`로 잔여 참조가 없음을 확인했다.

### 4. D-class는 수정 불필요

UDCard / UDPawn / UDMonster / UDEquipment는 자체 `Name` / `Desc` 멤버를 가지지 않고 `const F*Row* Data` 포인터만 보관한다 ([DCard.h:28](../Source/Dev_GNC/CardGame/Data/DCard.h#L28), [DPawn.h:37](../Source/Dev_GNC/CardGame/Data/DPawn.h#L37)). Row 접근은 모두 `Data->필드`를 통해 이루어지므로 D-class 헤더 및 `InitFromId` 구현은 건드릴 필요가 없었다.

## 검증 절차

1. **컴파일**: Dev_GNC 프로젝트 빌드. `Name` / `Desc` 잔여 참조가 있으면 컴파일 오류로 잡힌다.
2. **DataTable 값 확인**: 에디터에서 `DT_NamePresetData.uasset`을 열어 각 행의 `NameAlias` 컬럼이 `text_name_preset_alpha` 등 원본 Excel 값과 일치하는지 spot-check.
3. **런타임 스모크 테스트**:
   - 로비 진입 → Pawn 이름 정상 (`DataSubsystem::GetRandomName` 경로)
   - 전투 진입 → 카드 이름/설명이 `TextSubsystem->Get(Data->NameAlias)` 경유로 정상 표시
   - 몬스터 HUD → 이름 정상 표시
4. **추가 확인 대상**: DT_CardData, DT_PawnData, DT_MonsterData, DT_EquipmentData, DT_CardEffectData, DT_TileEntityData 역시 같은 충돌의 희생자였으므로 재임포트 후 값 복원 확인.

## 교훈

- UE5 DataTable JSON 임포트에서 `"Name"`은 RowName 전용 예약 키다. 구조체에 `Name` UPROPERTY를 두면 case-insensitive 매칭으로 인해 값이 비결정적으로 오염된다.
- 파이프라인이 스키마 기반으로 잘 설계되어 있으면, 데이터 소스(Excel) 수정만으로 생성기/C++ 스키마까지 자동 전파된다. 단, 데이터 소비자(C++ 호출부)는 직접 갱신이 필요하다.
- 로컬라이즈 키를 담는 필드는 명시적으로 `*Alias` / `*Key` 접미사를 붙이는 편이 의미 전달 및 예약어 충돌 회피 양쪽에서 유리하다.
