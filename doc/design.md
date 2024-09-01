# Sigmoid engine design

## Overview

A Sigmoid engine game is contained within it's own [bundle file]. The bundle
file contains a file called `INFO.JSON`, which contains information about
the game. The game consists of **scenes**, which are defined in their own file
each. For example, the scene `FIELD` is contained in the file `FIELD.SCN`. The
scene file contains a JSON object that describes the scene. There are two types
of scenes. A **field scene** is a scene that contains tiles and entities, where
one entity is the player. A **story scene** is a scene that contains text &
other visuals, similar to a visual novel.

## Game info

The `INFO.JSON` file contains the following fields:

* `title`: The title of the game.
* `author`: The author of the game.
* `description`: A description of the game.
* `version`: The version of the game.
* `start_scene`: The name of the scene to start the game in.

## Scene file

A scene file is a JSON object that contains the following fields:

* `title`: A title for this scene.
* `background`: The background image for this scene.
* `music`: The music to play for this scene.
* `next`: The name of the scene to go to next.
* `type`: The type of scene. Can be either `field` or `story`.
* Additional fields for each scene type.

### Field scene

A field scene contains the following fields:

* `tiles`: A list of `TileSpan`s to place on the field.
* `entities`: A list of `Entity`s to place on the field.
* `player`: The player entity.

#### `TileSpan`

A `TileSpan` is a JSON array consisting of four elements:

* The first element is the X coordinate to start at.
* The second element is the Y coordinate to start at.
* The third element is the tile ID.
* The fourth element is the number of tiles to place.

### Story scene

A story scene's `commands` field is a list of `Command`s to execute.

#### `Command`

A `Command` is a JSON array consisting of two elements:

* The first element is the type of command.
* The second element is the data for the command.

The following are the types of commands and their data:

* `speak`: Causes a character to speak.
  - `text`: The text to speak.
  - `character`: The character to speak.
  - `portrait`: The portrait to use.
* `background`: Changes the background.
  - `image`: The image to use.
  - `music`: The music to play.
* `sprite`: Changes a sprite.
  - `id`: The ID of the sprite.
  - `image`: The image to use or `null` to remove the sprite.
* `wait`: Waits for a certain amount of time.
  - `time`: The amount of time to wait in seconds.

[bundle file]: https://qeaml.github.io/nwge-docs/BUNDLE