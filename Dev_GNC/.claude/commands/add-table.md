---
description: "DataTable을 프로젝트에 등록한다. 사용법: /add-table <TableName> (예: /add-table Buff)"
allowed-tools: Read, Write, Edit, Glob, Grep
---

인자로 전달된 이름을 기반으로 UE5 DataTable을 프로젝트에 추가 등록한다.

## 인자 파싱

사용자 입력에서 테이블 이름을 추출한다.
- 입력 예시: `Buff`, `PawnData`, `ItemData`
- `Data` 접미사가 있으면 제거 (예: `PawnData` → `Pawn`)
- 이하 변수를 정의한다:
  - `BaseName`: 접미사 제거 후 이름 (예: `Buff`)
  - `DataName`: `{BaseName}Data` (예: `BuffData`)
  - `RowStruct`: `F{BaseName}DataRow` 또는 `F{BaseName}Row` — proto 메시지명에 따라 결정
  - `TableVar`: `{BaseName}DataTable` (예: `BuffDataTable`)
  - `CacheVar`: `{BaseName}Cache` (예: `BuffCache`)
  - `AssetPath`: `/Game/CardGame/DataTables/DT_{DataName}.DT_{DataName}`

## 1단계: 사전 검증

다음 항목을 확인한다. 문제가 있으면 작업을 중단하고 사용자에게 알린다.

**Proto 메시지 확인**
- `Tools/output/proto/GameData.proto`에서 `message {DataName}` 이 존재하는지 확인한다.
- 없으면: "GameData.proto에 {DataName} 메시지가 없습니다. Excel 스키마 추가 후 RunTableTool.bat 실행하세요." 출력 후 중단.

**JSON 파일 확인**
- `Tools/output/json/DT_{DataName}.json` 파일이 존재하는지 확인한다.
- 없으면: "DT_{DataName}.json이 없습니다. RunTableTool.bat 실행 후 다시 시도하세요." 출력 후 중단.

**중복 확인**
- `Source/Dev_GNC/CardGame/Data/CardGameRowTypes.h`에서 `{RowStruct}` 가 이미 존재하면 "이미 등록된 테이블입니다." 출력 후 중단.

## 2단계: Proto 메시지 분석

`Tools/output/proto/GameData.proto`에서 `message {DataName}` 의 필드 목록을 읽는다.
**추가로** `Tools/output/schema_snapshot.json`에서 `{DataName}` 키의 원본 `field_type`을 참조한다.
schema_snapshot의 `field_type`이 proto 타입보다 우선한다 (asset/classref 정보가 보존됨).

각 필드에 대해 (schema_snapshot의 field_type 기준):
- 타입 매핑:
  - `int32` → `int32`
  - `string` → `FString`
  - `float` → `float`
  - `bool` → `bool`
  - `enum:EnumName` → 대응하는 UENUM `E{EnumName}` (CardGameTypes.h에서 확인)
  - `asset:ClassName` → `TSoftObjectPtr<U{ClassName}>` (forward declaration 추가)
  - `classref:ClassName` → `TSoftClassPtr<A{ClassName}>` (forward declaration 추가)
- repeated 접두사가 있으면 `TArray<T>`로 감싼다
  - 예: `repeated asset:X` → `TArray<TSoftObjectPtr<UX>>`
  - 예: `repeated classref:X` → `TArray<TSoftClassPtr<AX>>`

asset/classref 타입 필드가 있으면:
- CardGameRowTypes.h 상단에 해당 클래스의 forward declaration을 추가한다 (이미 있으면 스킵)
  - `asset:TileMapPreset` → `class UTileMapPreset;`
  - `classref:CardGameActor` → `class ACardGameActor;`

id 필드가 `int32`인지 `string`인지 확인한다 (기본값: `int32`).

## 3단계: CardGameRowTypes.h에 FTableRowBase 구조체 추가

`Source/Dev_GNC/CardGame/Data/CardGameRowTypes.h`의 마지막 구조체 뒤에 추가한다.

```cpp
USTRUCT(BlueprintType)
struct {RowStruct} : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString IdAlias;
	// ... proto 메시지의 나머지 필드들 (2단계에서 분석한 결과)
};
```

모든 UPROPERTY에 `EditAnywhere, BlueprintReadOnly` 지정. 기본값: 정수=0, 문자열=빈문자열, 배열=빈배열.

## 4단계: DataSubsystem.h 수정

**조회 메서드 추가** (public 섹션, 기존 마지막 조회 메서드 아래):
```cpp
const {RowStruct}* Get{DataName}(int32 Id) const;
```

**테이블 포인터 추가** (private 섹션, 기존 마지막 DataTable 아래):
```cpp
UPROPERTY() TObjectPtr<UDataTable> {TableVar};
```

**캐시 추가** (private 섹션, 기존 마지막 Cache 아래):
```cpp
TMap<int32, const {RowStruct}*> {CacheVar};
```

## 5단계: DataSubsystem.cpp 수정

**Initialize()에 LoadTable 호출 추가** (마지막 LoadTable 호출과 BuildSecondaryIndices() 사이):
```cpp
{TableVar} = LoadTable<{RowStruct}>(
	TEXT("{AssetPath}"), {CacheVar});
```

**조회 메서드 구현 추가** (ID 기반 조회 섹션 끝):
```cpp
const {RowStruct}* UDataSubsystem::Get{DataName}(int32 Id) const
{
	const auto* Found = {CacheVar}.Find(Id);
	return Found ? *Found : nullptr;
}
```

## 6단계: ImportDataTables.py 수정

`Tools/ImportDataTables.py`의 `TABLE_STRUCT_MAP` 딕셔너리에 추가:
```python
"{DataName}": "{RowStruct 에서 F prefix 제거한 이름}",
```

예시: `"BuffData": "BuffDataRow",`

## 7단계: json_exporter.py enum 매핑 확인 (선택)

새 테이블에 enum 타입 필드가 있다면:
- `Tools/json_exporter.py`의 `FIELD_ENUM_TYPE` 딕셔너리에 `필드명: enum타입명` 추가
- `ENUM_MAPS`에 해당 enum이 없으면 추가

이미 매핑된 enum 필드면 이 단계는 건너뛴다.

## 8단계: 완료 보고

다음 형식으로 결과를 출력한다:

```
완료: {DataName} 테이블 등록

수정된 파일:
- CardGameRowTypes.h : {RowStruct} 구조체 추가
- DataSubsystem.h    : Get{DataName}() 선언, 테이블/캐시 멤버 추가
- DataSubsystem.cpp  : LoadTable 호출, 조회 메서드 구현
- ImportDataTables.py: TABLE_STRUCT_MAP에 매핑 추가
- json_exporter.py   : (enum 매핑 추가 시에만)

사용법:
- C++: GetGameInstance()->GetSubsystem<UDataSubsystem>()->Get{DataName}(Id)
- JSON 임포트: UE5 에디터에서 py "Tools/ImportDataTables.py" 실행
```
