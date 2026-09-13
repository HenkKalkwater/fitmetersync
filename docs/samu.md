# SAMU (Application layer)

Builds on top of [ircu](ircu.md). Handles the application logic.

## Command overview

| Direction | CMD Number | Name               | Payload                               | Argument                    |
|-----------|------------|--------------------|---------------------------------------|-----------------------------|
| Receive   | `0x00`     | ReceiveIdentData   | [IdentData](#identdata)               | Always `0x0`                |
| Receive   | `0x01`     | ReceiveTimeData    | [TimeData](#timedata)                 | Always `0x0`                |
| Receive   | `0x02`     | ReceiveMets        | [MetsData](#metsdata)                 | Amount of bytes to retrieve |
| Receive   | `0x03`     | ReceiveAltitude    | [AltitudeData](#altitudedata)         | Amount of bytes to retrieve |
| Receive   | `0x04`     | ReceiveActivityTag | [ActivityTagData](#activitytagdata)   | Amount of bytes to retrieve |
| Receive   | `0x05`     | ReceiveCalories    | [CaloriesData](#caloriesdata)         | Amount of bytes to retrieve |
| Receive   | `0x06`     | ReceiveSteps       | [StepsData](#stepsdata)               | Amount of bytes to retrieve |
| Transmit  | `0x80`     | SetIdentData       | [IdentData](#identdata)               | Always `0x0`                |
| Transmit  | `0x81`     | SetTimeData        | [TimeData](#timedata)                 | Always `0x0`                |
| Transmit  | `0x82`     | SetLocalizeData    | [LocalizeData](#localizedata)         | Data bank                   |
| Transmit  | `0x83`     | SetMiiIconData     | [MiiIconData](#miiicondata)           | Data bank                   |
| Transmit  | `0x84`     | SetMiiInfoData     | Unknown (Code refers to FFLStoreData) | Always `0x0`                |
| Transmit  | `0x85`     | SetPersonalData    | [PersonalData](#personaldata)         | Alwasy `0x0`                |

## Payload segmentation

The [MetsData](#metsdata), [AltitudeData](#altitudedata), [ActivityTagData](#activitytagdata), 
[CaloriesData](#caloriesdata) and [StepsData](#stepsdata) are divided into segments, divided by the time markers below:

Details are not yet fully known.

| u8 marker | u16 marker | Description                   |
|-----------|------------|-------------------------------|
| `0xFC`    | `0xFFFC`   | Follows relative date marker? |
| `0xFD`    | `0xFFFD`   | Follows absolute date marker? |
| `0xFE`    | `0xFFFE`   | End of stream                 |
| `0xFF`    | `0xFFFF`   | Padding, should be ignored    |

### Time stamp format

| Offset | Type  | Description |
|--------|-------|-------------|
| 0x00   | u8    | Unknown     |
| 0x01   | u8    | Minute      |
| 0x02   | u8    | Hour        |
| 0x03   | u8    | Day         |
| 0x04   | u8    | Month       |
| 0x05   | u8    | Year - 2000 |

## Payloads
Here's an overview of all payload types

### IdentData

| Name      | Type  | Description             |
|-----------|-------|-------------------------|
| `user_id` | `u32` | User ID (save data ID?) |
| `samu_id` | `u32` | ID of the Fit Meter     |
| `flags`   | `u8`  | Flags (unknown so far)  |

### TimeData

| Name    | Type | Description |
|---------|------|-------------|
| `unk`   | `u8` | Unknown     |
| `year`  | `u8` | Year - 2000 |
| `month` | `u8` | Month       |
| `day`   | `u8` | Day         |
| `hour`  | `u8` | Hour        |
| `min`   | `u8` | Minute      |
| `sec`   | `u8` | Second      |

### MetsData
METs per minute. [Compressed u8 data](#payload-segmentation).

### AltitudeData
Altitude in meters per minute. [Compressed u8 data](#payload-segmentation).
Each value is a delta value from the previous one. 

| Value            | Description                                  |
|------------------|----------------------------------------------|
| `0x00`           | Repeat 0x00 (no change) previous value times |
| `0x01` to `0x7F` | 1 to 127 meters                              |
| `0x80` to `0xF9` | -1 to -120 meters                            | 
| `0xFA`           | add 128 meters to the next value.            | 
| `0xFB`           | substract 120 meters to the next value.      |

### ActivityTagData
Activity tag per minute. [Compressed u8 data](#payload-segmentation). Details not yet known.

### CaloriesData
Calories burned per day. [Compressed u16 data](#payload-segmentation).

### StepsData
Steps per day. [Compressed u16 data](#payload-segmentation).

### LocalizeData

These are located in `content/Samu` and contain various formats in the subfolders. These are sent to the Fit Meter based 
on the set language, region and whether the user is a dog.

The resources are stored in compressed .jarc files. They are extractable using 
[QuickBMS](https://aluigi.altervista.org/quickbms.htm) and the `JARC` script found on the 
same page.

After decompression these archives, there will be a lot of files in the form of `82_XX.dat`. `XX` is a number in 
hexadecimal that will be sent as the argument to `SetLocalizeData`.

#### Data formats

The localize data contains texture and sound data. They are described below.

##### Texture
1 bit per pixel black and white. Stored in rows of 8 pixels. Some textures span more than 8 rows, these are stored
right after each other. You'll need to manually split them up.

##### Sound
The exact format is unknown, but they seem to be stored in tone height + duration pairs of 3 bytes.
`00 80 0C` is silence.

#### Data lists

| Number | Type    | Description                                                |
|--------|---------|------------------------------------------------------------|
| `00`   | Texture | arrows, battery, flag, goal                                |
| `01`   | Texture | colon, decorator, temperature, time                        |
| `02`   | Texture | graph left, graph left arrow                               |
| `03`   | Texture | graph right, graph right arrow                             |
| `04`   | Texture | arrow down, left, right, ???                               |
| `05`   | Texture | contrast button, sound high button                         |
| `06`   | Texture | sound low, sound off button                                |
| `07`   | Texture | battery low, hearth percent, sound icon, kcal text         |
| `08`   | Texture | ???, altitude graph header                                 |
| `09`   | Texture | mets graph header                                          |
| `0A`   | Texture | weekly kcal grap header                                    |
| `0B`   | Texture | settings header                                            |
| `0C`   | Texture | sound settings header                                      |
| `0D`   | Texture | contrast settings header                                   |
| `0E`   | Texture | "Send data" header                                         |
| `0F`   | Texture | "Data sent!" header                                        |
| `10`   | Texture | "wks ago" text                                             |
| `11`   | Texture | "Plays SFX based on activity" text                         |
| `12`   | Texture | "Connecting..." text                                       |
| `13`   | Texture | "No available connections." text                           |
| `14`   | Texture | "Connection failed." text                                  |
| `15`   | Texture | "Connection not possible." text                            |
| `16`   | Texture | "Take off when using Wii Fit U" text                       |
| `17`   | Texture | "Registered." text                                         |
| `18`   | Texture | "Comparing activity..." text                               |
| `19`   | Texture | "Training Twins" text                                      |
| `1A`   | Texture | "Same League" text                                         |
| `1B`   | Texture | "Different Lanes" text                                     |
| `1C`   | Texture | "Poles Apart" text                                         |
| `1D`   | Texture | big digits 0..4                                            |
| `1E`   | Texture | big digits 5..9                                            |
| `1F`   | Texture | medium digits 0..9                                         |
| `20`   | Texture | small digits 0..9                                          |
| `21`   | Texture | overline big digits 0..5                                   |
| `22`   | Texture | overline big digits 6..9                                   |
| `23`   | Sound   |                                                            |
| `24`   | Sound   |                                                            |
| `25`   | Sound   |                                                            |
| `26`   | Sound   |                                                            |
| `27`   | Sound   |                                                            |
| `28`   | Sound   |                                                            |
| `29`   | Sound   |                                                            |
| `30`   | Sound   |                                                            |
| `2A`   | Sound   | sync success jingle?                                       |
| `2B`   | Sound   |                                                            |
| `2C`   | Sound   |                                                            |
| `2D`   | Sound   |                                                            |
| `2E`   | Sound   |                                                            |
| `2F`   | Sound   |                                                            |
| `31`   | Texture | "Me" and "Partner" button                                  |
| `32`   | Texture | Mountain, gate                                             |
| `33`   | Texture | "Battery changed. Sync with the Wii U to update time" text |

### MiiIconData

The exact payload is unknown, but it seems to work similar to texture data in [LocalizeData](#localizedata).
Updated if the hash of the Mii changes or on setup.

The game data contains several dog icons under `content/Samu/dog_icon`. Mii picture data seems to be generated on the 
fly.

### PersonalData

Sent when initialising the Fit Meter, on locale change or on calorie goal change

| Offset | Type  | Description                                      |
|--------|-------|--------------------------------------------------|
| `0x0`  | u8    | Metabolism (calculated in-game based on profile) |
| `0x1`  | u16   | Calorie goal                                     |
| `0x3`  | u8    | Day change (corresponds to night owl setting)    |
| `0x4`  | u8    | Celsius/Fahrenheit flag                          |
| `0x5`  | u8    | 24-hour clock flag                               |
| `0x6`  | u8    | Metric flag                                      |
| `0x7`  | u8[6] | Unknown                                          |
| `0xD`  | u16   | Average calories per day                         |
