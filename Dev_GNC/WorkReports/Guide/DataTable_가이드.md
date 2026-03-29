# DataTable 가이드

## 개요

Excel에서 게임 데이터를 작성하면 자동화 파이프라인이 UE5 DataTable용 JSON으로 변환한다.
변환된 JSON은 UE5 에디터에서 DataTable 에셋으로 임포트하여 런타임에 사용한다.

```
Excel (.xlsx) → Proto → .bytes → JSON → UE5 DataTable
```

---

## 폴더 구조

```
Dev_GNC/
├── TableData/                  ← Excel 파일 (직접 편집)
│   ├── CardDataTable.xlsx
│   ├── MonsterDataTable.xlsx
│   ├── ...                     (데이터 12개)
│   └── Proto/                  ← 스키마 정의
│       ├── Enum.xlsx
│       ├── Proto_CardDataTable.xlsx
│       └── ...                 (스키마 12개)
├── Tools/
│   ├── RunTableTool.bat        ← 변환 실행 (더블클릭)
│   ├── ImportDataTables.py     ← UE5 에디터 내 임포트 스크립트
│   ├── main.py                 ← 파이프라인 본체
│   ├── config.yaml             ← 설정
│   └── output/                 ← 생성 결과물 (자동 생성)
│       ├── json/DT_*.json
│       ├── bytes/*.bytes
│       ├── proto/GameData.proto
│       └── py/GameData_pb2.py
└── Source/.../CardGame/
    ├── Data/
    │   ├── CardGameTypes.h         ← UENUM 정의
    │   └── CardGameRowTypes.h/cpp  ← FTableRowBase 구조체
    └── Subsystem/
        ├── DataSubsystem.h/cpp     ← 데이터 조회 API
        └── TextSubsystem.h/cpp     ← 텍스트/로컬라이제이션
```

---

## 일상 작업: 데이터 수정

Excel 값만 바꾸는 경우 (필드 추가/삭제 없음).

### 1. Excel 수정
`TableData/` 폴더의 xlsx 파일을 열어 값을 수정하고 저장한다.

### 2. 변환 실행
아래 방법 중 하나로 실행한다.

- **VSCode 단축키**: `Ctrl+Shift+B` (기본 빌드 태스크로 등록됨)
- **VSCode 명령 팔레트**: `Ctrl+Shift+P` → `Tasks: Run Task` → `Run Table Tool`
- **직접 실행**: `Tools/RunTableTool.bat` 더블클릭

```
처리한 데이터 시트 수  : 12
생성된 JSON 파일 수    : 12
```
이 출력이 나오면 성공.

### 3. UE5 에디터에서 임포트
Output Log 하단 `Cmd` 입력창에서:
```
py "C:/UnrealProjects/ProjectGNC/Dev_GNC/Tools/ImportDataTables.py"
```
기존 DataTable 에셋의 데이터가 갱신된다.

---

## 새 테이블 추가

### 1. Excel 스키마 작성
`TableData/Proto/`에 `Proto_XxxDataTable.xlsx` 생성.
기존 파일을 복사해서 필드명/타입을 수정하면 편하다.

### 2. Excel 데이터 작성
`TableData/`에 `XxxDataTable.xlsx` 생성.
스키마에 정의한 필드 순서대로 컬럼을 맞춘다.

### 3. 변환 실행
`Tools/RunTableTool.bat` 더블클릭.
`output/json/DT_XxxData.json`이 생성되는지 확인.

### 4. C++ 코드 등록
Claude에서 `/add-table Xxx` 커맨드를 실행하면 아래 파일이 자동 수정된다:
- `CardGameRowTypes.h` — USTRUCT 추가
- `DataSubsystem.h/cpp` — 로드/조회 메서드 추가
- `ImportDataTables.py` — JSON 임포트 매핑 추가
- `json_exporter.py` — enum 필드 매핑 (해당 시)

### 5. 빌드 및 임포트
프로젝트 빌드 후 UE5 에디터에서 ImportDataTables.py 실행.

---

## 기존 테이블에 필드 추가/삭제

### 1. 스키마 수정
`TableData/Proto/Proto_XxxDataTable.xlsx`에서 필드 행을 추가/삭제한다.
**주의**: 기존 필드의 번호(Number 열)는 변경하지 않는다. 새 필드는 기존 최대 번호 + 1로 설정.

### 2. 데이터 수정
`TableData/XxxDataTable.xlsx`에 해당 컬럼을 추가/삭제한다.
**참고**: 스키마에 필드를 추가했지만 데이터 엑셀에 컬럼을 아직 만들지 않아도 에러 없이 진행된다.
누락된 컬럼은 proto3 기본값(int→0, string→"", bool→false, enum→0번 값)으로 자동 채워진다.

### 3. 변환 실행
`Tools/RunTableTool.bat` 더블클릭.
Step 4에서 필드 번호 변경 경고가 나오면 확인한다.

### 4. C++ 구조체 수정
`CardGameRowTypes.h`에서 해당 USTRUCT에 UPROPERTY를 추가/삭제한다.

### 5. 빌드 및 임포트
프로젝트 빌드 후 ImportDataTables.py 실행.

---

## C++ 데이터 조회 API

```cpp
// DataSubsystem 가져오기
auto* DataSub = GetGameInstance()->GetSubsystem<UDataSubsystem>();

// ID로 조회
const FPawnDataRow* Pawn = DataSub->GetPawnData(1);
const FCardDataRow* Card = DataSub->GetCardData(1001);

// 특수 조회
const FShopDataRow* Shop = DataSub->GetShopDataByLevel(3);
const FStagePresetRow* Stage = DataSub->GetStagePresetByLevel(1);
const FPawnGrowthRow* Growth = DataSub->GetPawnGrowthByClassLevel(EClassType::Scout, 2);
FString Name = DataSub->GetRandomName();

// 전체 목록
const TMap<int32, const FCardDataRow*>& AllCards = DataSub->GetAllCardData();

// 텍스트 조회
auto* TextSub = GetGameInstance()->GetSubsystem<UTextSubsystem>();
FString Text = TextSub->Get("text_pawn_name_scout");
TextSub->SetLanguage(ELanguage::Eng);
```

---

## 현재 등록된 테이블 (12종)

| DataTable | Row Struct | 행 수 | 비고 |
|---|---|---|---|
| DT_PawnData | FPawnDataRow | 5 | 플레이어 유닛 |
| DT_MonsterData | FMonsterDataRow | 2 | 적 유닛 |
| DT_CardData | FCardDataRow | 40 | 카드 |
| DT_CardEffectData | FCardEffectRow | 12 | 카드 효과 |
| DT_EquipmentData | FEquipmentDataRow | 16 | 장비 |
| DT_TileEntityData | FTileEntityRow | 2 | 타일 엔티티 |
| DT_StagePresetData | FStagePresetRow | 5 | 스테이지 구성 |
| DT_StageRewardData | FStageRewardRow | 5 | 스테이지 보상 |
| DT_ShopData | FShopDataRow | 5 | 상점 |
| DT_PawnGrowthData | FPawnGrowthRow | 40 | 유닛 성장 |
| DT_NamePresetData | FNamePresetRow | 26 | 이름 프리셋 |
| DT_TextData | FTextRow | 70 | 텍스트/로컬라이제이션 |

---

## 스키마 FieldType 참조

### 기본 타입

| FieldType | Proto 타입 | UE5 C++ 타입 | 비고 |
|---|---|---|---|
| `int32` | int32 | `int32` | |
| `int64` | int64 | `int64` | |
| `float` | float | `float` | |
| `double` | double | `double` | |
| `string` | string | `FString` | |
| `bool` | bool | `bool` | |

### 특수 타입

| FieldType | Proto 타입 | UE5 C++ 타입 | 비고 |
|---|---|---|---|
| `enum:EnumName` | EnumName (enum) | `EEnumName` | Enum.xlsx에 정의 필요 |
| `asset:ClassName` | string | `TSoftObjectPtr<UClassName>` | 데이터 에셋 참조 |
| `classref:ClassName` | string | `TSoftClassPtr<AClassName>` | BP 클래스 참조 |

### asset / classref 타입 사용법

에셋 참조 필드는 두 가지 타입으로 구분된다. 파이프라인 내부에서는 둘 다 `string`으로 처리되며, 차이는 C++ 코드 생성 시에만 발생한다.

| 타입 | 용도 | C++ 타입 | 에디터 UI |
|---|---|---|---|
| `asset:X` | 데이터 에셋 인스턴스 참조 | `TSoftObjectPtr<UX>` | 에셋 선택 드롭다운 |
| `classref:X` | 블루프린트 클래스 참조 | `TSoftClassPtr<AX>` | BP 클래스 선택 드롭다운 |

**에디터 설정값 보존**: asset/classref 타입 필드는 엑셀에 컬럼이 없어도 기존 JSON의 값이 자동 보존된다.
UE5 에디터에서 수동 설정한 에셋 경로가 파이프라인 재실행 시 소실되지 않는다.
엑셀에 해당 컬럼이 존재하면 엑셀 값이 우선 적용된다.

**asset 예시** — 데이터 에셋 참조 (예: TileMapPreset):

| MessageName | FieldName | FieldType | FieldNumber | Rule |
|---|---|---|---|---|
| StagePresetData | presetPath | asset:TileMapPreset | 4 | repeated |

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite)
TArray<TSoftObjectPtr<UTileMapPreset>> PresetPath;

// 로드
UTileMapPreset* Preset = PresetRef.LoadSynchronous();
```

**classref 예시** — BP 클래스 참조 (예: CardGameActor):

| MessageName | FieldName | FieldType | FieldNumber | Rule |
|---|---|---|---|---|
| PawnData | prefabPath | classref:CardGameActor | 14 | optional |

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite)
TSoftClassPtr<ACardGameActor> PrefabPath;

// 클래스 로드
UClass* ActorClass = PrefabPath.LoadSynchronous();
// 스폰
GetWorld()->SpawnActor<ACardGameActor>(ActorClass, SpawnTransform);
```

**Excel 데이터 입력값**: UE5 패키지 경로 문자열
```
/Game/CardGame/Presets/Stage1_Easy.Stage1_Easy
```

### repeated 필드 (배열)

Rule 열에 `repeated`를 지정하면 `TArray<T>`로 매핑된다.
데이터 입력은 쉼표 구분 문자열 또는 다중 컬럼 방식을 지원한다.

---

## 트러블슈팅

**RunTableTool.bat 실행 시 창이 바로 닫힘**
→ bat 파일이 CRLF 줄바꿈인지 확인. LF만 있으면 cmd.exe가 제대로 파싱하지 못한다.

**JSON 임포트 시 행이 0개**
→ JSON의 필드명과 USTRUCT의 UPROPERTY 이름이 일치하는지 확인. proto 필드명은 camelCase.

**필드 번호 변경 경고**
→ 기존 필드의 proto 번호를 바꾸면 이전 .bytes와 호환이 깨진다. 새 번호로만 추가할 것.

**enum 값이 숫자로 나옴**
→ `json_exporter.py`의 `FIELD_ENUM_TYPE`에 해당 필드가 매핑되어 있는지 확인.
