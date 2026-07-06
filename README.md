# flashlighthead — налобный фонарь для CoD4 MW (2007) на базе iw3xo (без RTX)

Мод для клиента [iw3xo](https://github.com/xoxor4d/iw3xo-dev), добавляющий консольную команду `flashlighthead` — динамический прожектор (spot light) от головы игрока в **стоковом рендерере** (RTX-режим не требуется).

## Как это работает

Стоковый рендерер IW3 поддерживает динамические spot-света в рантайме — их использует FX-система (`FX_ELEM_TYPE_SPOT_LIGHT`), но игра почти нигде их не задействует:

- `GfxScene::addedLight[32]` — список динамических источников, заполняемый каждый кадр (указатель на сцену: `game::scene` = `0xCF10280`);
- рендерер отображает до 4 видимых динамических света, один из них — с shadow map (`visLightShadow`, техники `TECHNIQUE_LIT_SPOT_SHADOW`);
- текстура затухания берётся из дефолтного `GfxLightDef` движка (`rgp->dlightDef`).

Компонент `flashlight` каждый кадр (из `CG_CalcViewValues`, когда сцена заполняется) дописывает в `scene->addedLight` свой `GfxLight`:

- `type = GFX_LIGHT_TYPE_SPOT`, `canUseShadowMap = 1` — конус света + тени от геометрии;
- позиция = глаза игрока (`predictedPlayerState.origin + viewHeightCurrent`) + смещение «на лоб»;
- направление = вектор взгляда (`viewangles`) — луч следит за камерой каждый кадр;
- двойной конус (яркий hotspot + мягкий spill), как у реального налобного фонаря.

## Установка / сборка

1. `git clone https://github.com/xoxor4d/iw3xo-dev`
2. Применить патч: `git apply iw3xo-flashlight.patch` (или вручную скопировать `src/components/modules/flashlight.{cpp,hpp}` и внести правки в `loader.{cpp,hpp}` и `radiant_livelink.cpp` — см. патч).
3. `generate-buildfiles_vs22.bat` → собрать в Visual Studio 2022 (x86, Release).
4. Установить собранный клиент к CoD4 1.7 как обычный iw3xo (без флагов `-rtx`).

## Использование

```
flashlighthead 1      // включить
flashlighthead 0      // выключить
flashlighthead        // без аргумента — переключить
bind f "flashlighthead"
```

### Настройка (dvars, сохраняются в конфиг)

| Dvar | По умолчанию | Описание |
|---|---|---|
| `flashlight_intensity` | 3.0 | яркость |
| `flashlight_radius` | 2000 | дальность в юнитах (~40 юнитов = 1 м) |
| `flashlight_fov_inner` | 24 | полный угол яркого ядра луча, ° |
| `flashlight_fov_outer` | 70 | полный угол внешнего конуса, ° |
| `flashlight_color` | 1 0.92 0.78 | цвет (тёплый белый ~4000K) |
| `flashlight_offset` | 6 0 4 | смещение от глаз (вперёд, вправо, вверх) |

## Файлы

- `src/components/modules/flashlight.cpp / .hpp` — новый компонент (команда, dvars, инжект света в сцену);
- `iw3xo-flashlight.patch` — полный diff против `xoxor4d/iw3xo-dev` master: регистрация компонента в `loader.{cpp,hpp}` + вызов `flashlight::frame()` из существующего хука `CG_CalcViewValues` в `radiant_livelink.cpp`.

## Ограничения

- Работает в мультиплеере как клиентский мод — свет видите только вы (сервер не знает о фонаре). Для синхронизации между игроками нужен серверный мод.
- Лимит движка: до 4 видимых динамических света, тень только у одного.
- Если движок не отрисует свет без валидного lightdef на какой-то карте, компонент безопасно пропускает кадр (`rgp->dlightDef == null`).
