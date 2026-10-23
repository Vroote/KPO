# Отчёт по проекту ВышКат

Прудыус Владислав Романович, БПИ 254

## Ход работы

Перед написанием проекта было изучено задание и от руки нарисована примерная схема взаимодействия между модулями ([`docs/architecture_arm_v1.pdf`](docs/architecture_arm_v1.pdf)).

Также была создана базовая структура проекта: `src`, `include`, `docs`. В папки были добавлены соответственно `.cpp` и `.h` будущих модулей.

В `docs` были добавлены схема архитектуры от руки и черновик отчёта (чистовик строится по черновику при помощи ИИ).

В контекст ИИ-агента были добавлены требования, чтобы он придерживался их на протяжении всего проекта ([`docs/instructions.txt`](docs/instructions.txt)):

```text
Инструкции к проекту:
Стек:
С++20
SOLID
Boost.DI
GoogleTest

Инструкции к работе:
Папки: include/, src/, tests/
Стиль: поля с суффиксом `_`, методы camelCase, константы kName, хелперы в анонимном namespace, тексты исключений пиши по-русски.
После каждого запроса пиши только указанный модуль, ничего не подключай заранее.
Параллельно ведём схему архитектуры (docs/)
```

Важно отметить, что язык C++20, Boost.DI и GoogleTest — моё решение, SOLID указан в задании. Я добавил SOLID в инструкции, чтобы агент понимал, что мы делаем, и при моей ошибке мог вовремя поправить.

**Промпт ИИ:**

```text
Прочти инструкции в docs/instructions.txt, запомни, больше ничего не делай.
```

**Промпт ИИ:**

```text
Перерисуй аккуратно мою схему (docs/architecture_arm_v1.pdf). Ничего не добавляй и не убирай, только сделай читаемо.
```

Я разрешил агенту исправить орфографические опечатки.

**Промпт ИИ:**

```text
Перерисуй схему в более современном дизайне, только оставь всю структуру.
```

Далее начинаем делать задание.

**Промпт ИИ:**

```text
Сделай include/equipment.h и src/equipment.cpp, CMakeLists.txt с библиотекой vyshkat_core.
Интерфейсы: IInventoriable (name(), inventoryNumber()) и IEnergyConsumer (dailyEnergyKwh()).
Vehicle: Scooter, Bicycle и ElectricVehicle -> ElectricScooter, ElectricBicycle.
Item: Helmet и ElectricItem -> DockStation, ChargingCabinet.

Поля Vehicle: kindTitle, nickname, simplicity (1..10), wearPercent (0..100).
У каждого вида есть kTitle. name() возвращает kindTitle + «nickname».

Конструкторы проверяют параметры и бросают invalid_argument, в том числе при NaN и dailyEnergyKwh < 0.
assignInventoryNumber вызывается один раз, повторный вызов бросает logic_error.
```

**Правки:** агент написал всё в `namespace vyshkat`, хотя в текущем проекте это не требуется. Это не является ошибкой, но уменьшает читаемость, так что я решил это убрать.

**Промпт ИИ:**

```text
Сделай VehicleFactory (Equipment_factory.h/.cpp).

Типы: VehicleParams {name, simplicity, wear, energy}, VehicleType {id, title, needEnergy}, Creator = std::function.
Встроенные id: scooter, bicycle, e-scooter, e-bicycle.
Методы create(kind, params), types(), contains(), registerType().
Неизвестный kind, пустой id или creator, или дублирование id -> invalid_argument.
```

Далее я решил дополнить схему:

```text
Схема: Отметь, что конструктор проверяет параметры, а inventoryNumber пустой, пока объект не на балансе.
Подпиши блок «VehicleFactory - реестр видов».
```

**Промпт ИИ:**

```text
Сделай Fleet (Fleet.h/.cpp): vehicles_ и items_ на unique_ptr.
addVehicle и addItem пусть берут номер из одного общего lastNumber_ (формат «ВК-[:06]») и возвращают его. nullptr -> invalid_argument.
vehicles() и items() возвращают vector<const T*>.
```

**Промпт ИИ:**

```text
Сделай Inspection_service_interface.h.
Структура InspectionResult {accepted, reason}.
Интерфейс IInspectionService с методом inspect(const Vehicle&) const.
```

Обновим схему.

**Промпт ИИ:**

```text
Схема: Подпиши Fleet «баланс, inventoryNumber». Отметь, что это Fleet и что lastNumber_ общий для Vehicle и Item.
```

**Промпт ИИ:**

```text
Сделай ServiceCenter : IInspectionService (ServiceCenter_service.h/.cpp).
Константы kMaxWearPercent = 40 и kMaxDailyEnergyKwh = 2.
wearPercent > kMaxWearPercent -> reason «износ 55 % > 40 %».
Иначе, если приведение типа к IEnergyConsumer удалось и dailyEnergyKwh > kMaxDailyEnergyKwh -> reason «расход 2.5 > 2 кВт·ч/сут».
Ровно на лимите - принимается.
```

**Промпт ИИ:**

```text
Сделай IntakeService(const IInspectionService&, Fleet&) (Intake_service.h/.cpp).
accept(unique_ptr<Vehicle>) возвращает IntakeResult {accepted, inventoryNumber, reason}. Если accepted == false, номер не тратится, иначе Fleet::addVehicle.
registerItem(unique_ptr<Item>) ставит вещь сразу, без inspect, и возвращает inventoryNumber.
```

**Ошибка ИИ:** после ревью было обнаружено, что `accept(nullptr)` не бросает исключение, а возвращает `{false, "", "пустой транспорт"}`.

- Если `creator` вернул пустой указатель или, например, передали перемещённый `unique_ptr`, то транспорт не принимается непонятно почему для пользователя, хотя это баг в программе.
- Также нет соответствия с `registerItem` и `addVehicle` при `nullptr`: они бросают исключения, а `accept` возвращает результат.
- `reason` больше не работает правильно.

Поэтому говорим ИИ переделать:

```text
Исправь IntakeService::accept: при vehicle == nullptr бросай std::invalid_argument("Нельзя принять пустой транспорт") до вызова inspect, а не возвращай IntakeResult.
Fleet при этом не трогать, номер не тратится. Остальную логику accept и registerItem не меняй, другие модули не трогай.
```

**Промпт ИИ:**

```text
Сделай BeginnerPolicy (Beginner_policy.h/.cpp): kMinSimplicity = 6
isSuitable(const Vehicle&) возвращает simplicity() >= kMinSimplicity.
```

**Промпт ИИ:**

```text
Сделай FleetReports(const Fleet&, const BeginnerPolicy&) (Fleet_reports.h/.cpp).
summary() возвращает FleetSummary {vehicleCount, itemCount, dailyEnergyKwh}, dailyEnergyKwh - сумма по vehicles() и items() через приведение типа к IEnergyConsumer.
beginnerFriendly() возвращает Vehicle, для которых isSuitable == true.
inventory() возвращает InventoryLine {inventoryNumber, name, isVehicle}, отсортированные по inventoryNumber.
```

После этого шага агент предложил вынести форматирование инвентаря из `Fleet` в открытую функцию, я согласился.

Обновление схемы.

**Промпт ИИ:**

```text
Схема: подпиши BeginnerPolicy «simplicity ≥ 6»
Добавь листы со сценариями summary(), beginnerFriendly() и inventory() (через IInventoriable). Отметь, что Fleet отдаётся только по const&
По общей схеме не видно, в каком порядке идут inspect и выдача номера. Добавь второй лист «Сценарий»:
- диаграмма последовательности: Оператор -> App -> VehicleFactory -> IntakeService -> ServiceCenter -> Fleet с ветками accepted / !accepted.
- жизненный цикл: данные -> объект без inventoryNumber -> inspect -> в Fleet или уничтожен.
- короткий блок «почему так»: номер выдаётся только после inspect, IntakeService знает только IInspectionService, unique_ptr передаёт владение.
```

**Уточнение ИИ:**

```text
Отдельный лист «Отчеты» для каждого сценария.
```

**Промпт ИИ:**

```text
Сделай ConsoleApp(factory, intake, reports, istream&, ostream&) (app.h/.cpp), метод run().
Меню:
1 types
2 acceptVehicle
3 registerItem
4 summary
5 beginnerFriendly
6 inventory
0 выход.

Вывод:
Принят: ВК-… <name>
Отклонён: <reason>
Поставлен на баланс: ВК-… <name>
Транспорт: 3, вещей: 3, суточный расход: 5.8 кВт·ч.
invalid_argument печатается как «Ошибка: <what>», затем возврат в меню.
На EOF run() тихо выходит.
Пусть будет padRight, который выравнивает текст.
```

**Ошибка ИИ:** агент реализовал добавление оборудования прямо в `app`, без фабрики, что противоречит принципам SOLID.

**Промпт ИИ:**

```text
Добавь в Equipment_factory.h/.cpp фабрику вещей ItemFactory по образцу VehicleFactory.
ItemParams {energy}, ItemType {id, title, needEnergy}.
Методы: create(kind, params) -> unique_ptr<Item>, types(), contains(), registerType(type, creator).
В конструкторе зарегистрируй helmet (Шлем, без энергии), dock-station (Док-станция) и charging-cabinet - последние два с расходом.
Ошибки как в VehicleFactory: пустой id, пустой creator, повторный id, неизвестный вид -> invalid_argument.
ConsoleApp и остальные модули не трогай.
```

**Промпт ИИ** (в дополнение к предыдущему):

```text
Переведи ConsoleApp на ItemFactory: добавь const ItemFactory& в конструктор после VehicleFactory.
registerItem() выводит список видов из itemFactory.types() в том же формате, что и пункт 1, спрашивает «Вид (id): », и если needEnergy, то спрашивает расход. Вещь создаёт через itemFactory.create.
Из app.cpp убери прямые упоминания Helmet, DockStation, ChargingCabinet.
Остальные пункты меню не меняй, другие модули не трогай.
```

Далее остаются DI и тесты.

**Промпт ИИ:**

```text
Сделай Di_container.h: inline auto makeInjector(istream&, ostream&) на Boost.DI
Привязки: IInspectionService -> ServiceCenter, все сервисы в di::singleton, чтобы Fleet был общим, istream и ostream - по ссылке.
```

**Промпт ИИ:**

```text
Сделай main.cpp: makeInjector(cin, cout) -> create<ConsoleApp>() -> run().
Добавь цель KPO в CMake с линковкой vyshkat_core и Boost.DI.
```

Далее были проблемы с Boost.DI, поэтому он ставится через `FetchContent`.

## Тесты

Важно, чтобы агент не знал, что он писал, чтобы не подгонял тесты под своё решение. Для этого я попросил его написать подробную документацию по всем тестируемым функциям и передать другому агенту, который не знает контекста.

**Промпт ИИ:**

```text
Напиши четкую документацию по каждой функции для покрытия юнит тестами.
```

**Промпт ИИ** (другой агент):

```text
Подключи Google тесты и напиши их по спецификации
```

Также ИИ последним шагом сделал красивый интерфейс консоли.

Тесты были написаны и пройдены успешно. Также были введены намеренно неверные тесты, которые по плану «упали».

## Итог

Для сборки нужны CMake 3.20 или новее и компилятор с поддержкой `std::format`: GCC 13+, Clang 17+, Apple Clang 15+ или MSVC 2019 16.10+. При первой сборке нужен интернет, потому что Boost.DI и GoogleTest скачиваются автоматически.

### Запуск

```bash
cmake -S . -B build
cmake --build build
./build/KPO
ctest --test-dir build
```

### Архитектура

- `IInventoriable` (наименование, инвентарный номер) реализуют `Vehicle` и `Item`. `IEnergyConsumer` (суточный расход) реализуют только `ElectricVehicle` и `ElectricItem`, поэтому у шлема и обычного самоката нет «пустого» расхода.
- `VehicleFactory` и `ItemFactory` — реестры видов, новый вид добавляется через `registerType`.
- `ServiceCenter : IInspectionService` проверяет износ (≤ 40 %) и расход (≤ 2 кВт·ч/сут).
- `IntakeService` сначала вызывает `inspect`, и только при положительном решении передаёт объект в `Fleet`. Поэтому номер не тратится на отклонённую технику. Вещи ставятся на баланс без техосмотра.
- `Fleet` хранит объекты (`unique_ptr`) и выдаёт номера `ВК-000001` из одного счётчика для техники и вещей.
- `FleetReports` строит сводку, список для новичков (через `BeginnerPolicy`, простота ≥ 6) и инвентарь.
- `ConsoleApp` отвечает только за ввод и вывод. Всё собирается в `Di_container.h` (Boost.DI).

### SOLID

- **S** — `Fleet` хранит, `IntakeService` принимает, `FleetReports` считает отчёты, `ConsoleApp` работает с консолью.
- **O** — новый вид транспорта или вещи добавляется через `registerType` в фабрике, остальной код не меняется.
- **L** — любой наследник `Vehicle` / `Item` одинаково обрабатывается в `Fleet` и `FleetReports`.
- **I** — интерфейсы `IInventoriable` и `IEnergyConsumer` разделены.
- **D** — `IntakeService` зависит от `IInspectionService`, а конкретный `ServiceCenter` подставляет DI-контейнер.

### Обязательные вопросы

**1. Наиболее явный принцип** — DIP: `IntakeService` знает только интерфейс `IInspectionService`, поэтому техосмотр можно подменить и в DI-контейнере, и в тестах.

**2. Специально (с пониманием) ограниченный принцип.** `ServiceCenter` и `FleetReports` используют `dynamic_cast` к `IEnergyConsumer` вместо виртуального метода в `Vehicle`. Это небольшой отход от O и L в SOLID, но иначе у неэлектрической техники был бы фиктивный расход 0.

**3. Расширение.**

- Новый вид транспорта: нужен новый класс и вызов `registerType` в `VehicleFactory`, остальной код не меняется.
- Сотрудник не наследует `IInventoriable` и не попадает в `Fleet`, поэтому ничего не ломается.
- Склад запчастей — отдельный модуль. Если запчасти нужно учитывать на балансе, это новый наследник `Item`.

**4. Тесты.**

- Решение техосмотра фиксируют тесты SC02–SC08 ([`tests/test_service_center.cpp`](tests/test_service_center.cpp)).
- Правило «для новичков» фиксируют тесты BP02 и BP03 ([`tests/test_beginner_policy.cpp`](tests/test_beginner_policy.cpp)).
- Техосмотр подменяется двумя способами:
  - через конструктор: `FakeInspection` передаётся в `IntakeService` (тесты IN01–IN04, [`tests/test_intake_service.cpp`](tests/test_intake_service.cpp));
  - через DI-контейнер: поверх `makeInjector` привязка `IInspectionService` заменяется на фейк с помощью `di::override` (тесты DI06 и DI07, [`tests/test_di_container.cpp`](tests/test_di_container.cpp)). В DI07 самокат с износом 90 %, который `ServiceCenter` отклонил бы, принимается через меню приложения.

Всего 121 тест, все проходят, покрытие 100 % функций.

После правки пункта 2 меню (вывод списка видов транспорта перед «Вид (id)») агент, писавший приложение, добавил тест AP27: он проверяет, что таблица видов выводится раньше вопроса «Вид (id)». Этот тест написан не по спецификации, а вместе с правкой.

Итого 122 теста проходят.

### Роль ИИ

**Моё:** выбор стека (C++20, Boost.DI, GoogleTest), исходная схема архитектуры от руки, разбиение проекта на модули и спецификация каждого промпта, ревью кода после каждого шага. Тесты писал отдельный агент, который не знал, как устроен код, а работал только по спецификации.

**ИИ:** писал код модулей по моим спецификациям, перерисовывал схемы архитектуры, составил документацию функций для тестов и написал сами тесты. Этот README переписан ИИ из моего черновика отчёта ([`docs/Отчет_черновик.pages`](docs/Отчет_черновик.pages)).

**Исправлено мной после ревью:**

- `IntakeService::accept(nullptr)` теперь бросает `invalid_argument`, а не возвращает отказ (тесты IN05 и IN06, [`tests/test_intake_service.cpp`](tests/test_intake_service.cpp): исключение бросается до `inspect`, номер не тратится);
- создание вещей перенесено из `ConsoleApp` в отдельную фабрику `ItemFactory`;
- убран лишний `namespace vyshkat`;
- добавлены тесты подмены техосмотра через DI-контейнер (DI06, DI07);
- пункт 2 меню показывает список видов транспорта так же, как пункт 3 (тест AP27).

### Пример использования

```text
1 — виды транспорта
2 — принять транспорт
3 — поставить вещь на баланс
4 — сводка
5 — для новичков
6 — инвентарь
0 — выход
> 2
scooter     Самокат
bicycle     Велосипед
e-scooter   Электросамокат      нужен расход энергии
e-bicycle   Электровелосипед    нужен расход энергии
Вид (id): e-scooter
Кличка: Крутой
Простота (1–10): 1
Износ, %: 70
Расход, кВт·ч/сут: 1
Отклонён: износ 70 % > 40 %

1 — виды транспорта
2 — принять транспорт
3 — поставить вещь на баланс
4 — сводка
5 — для новичков
6 — инвентарь
0 — выход
> 3
helmet           Шлем
dock-station     Док-станция         нужен расход энергии
charging-cabinet Зарядный шкаф       нужен расход энергии
Вид (id): helmet
Поставлен на баланс: ВК-000001 Шлем

1 — виды транспорта
2 — принять транспорт
3 — поставить вещь на баланс
4 — сводка
5 — для новичков
6 — инвентарь
0 — выход
> 1
scooter     Самокат
bicycle     Велосипед
e-scooter   Электросамокат      нужен расход энергии
e-bicycle   Электровелосипед    нужен расход энергии

1 — виды транспорта
2 — принять транспорт
3 — поставить вещь на баланс
4 — сводка
5 — для новичков
6 — инвентарь
0 — выход
> 2
scooter     Самокат
bicycle     Велосипед
e-scooter   Электросамокат      нужен расход энергии
e-bicycle   Электровелосипед    нужен расход энергии
Вид (id): bicycle
Кличка: Крутой
Простота (1–10): 1
Износ, %: 1
Принят: ВК-000002 Велосипед «Крутой»

1 — виды транспорта
2 — принять транспорт
3 — поставить вещь на баланс
4 — сводка
5 — для новичков
6 — инвентарь
0 — выход
> 4
Транспорт: 1, вещей: 1, суточный расход: 0 кВт·ч.

1 — виды транспорта
2 — принять транспорт
3 — поставить вещь на баланс
4 — сводка
5 — для новичков
6 — инвентарь
0 — выход
> 6
ВК-000001   Шлем                            вещь
ВК-000002   Велосипед «Крутой»              транспорт

1 — виды транспорта
2 — принять транспорт
3 — поставить вещь на баланс
4 — сводка
5 — для новичков
6 — инвентарь
0 — выход
> 0
```
