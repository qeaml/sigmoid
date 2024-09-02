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

The `GAME.INFO` file contains the following fields:

* `title`: The title of the game.
* `author`: The author of the game.
* `description`: A description of the game.
* `version`: The version of the game.
* `start_scene`: The name of the scene to start the game in.
* `logo`: The logo of the game.
* `menu_background`: The menu background of the game.

## Scene file

A scene file is a JSON object that contains the following fields:

* `title`: A title for this scene.
* `background`: The background image for this scene.
* `music`: The music to play for this scene. If missing, no music is played.
* `next`: The name of the scene to go to next. If missing, the game will
  end.
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

A story scene's `actors` field is an object containing `Actor` definitions for
use with `Command`s.

A story scene's `commands` field is a list of `Command`s to execute.

#### `Actor`

An `Actor` is a JSON object containing the following fields:

* `name`: The name of the actor shown to the player.
* `sheet`: The sprite sheet for the actor's portrait.
* `sprites`: Defines the amount of sprites for the actor in columns and rows.

#### `Command`

A `Command` is a JSON object consisting of one field:

* The field name is a command.
* The field value is the data for the command.

The following are the types of commands and their data:

* `speak`: Causes an actor to speak.
  - `text`: The text to speak.
  - `actor`: The actor to speak. If missing, assume previously spoken actor.
  - `portrait`: The portrait to use. If missing, assume previously used
    portrait, only if `actor` is also missing.
* `background`: Changes the background.
  - `image`: The image to use. Empty string to remove the background. If
    missing, does not change the background.
  - `music`: The music to play. Empty string to stop the music. If missing, does
    not change the music.
* `sprite`: Changes a sprite.
  - `id`: The ID of the sprite.
  - `actor`: The actor whose sprite sheet to use. If missing, do not change the
    sprite's actor.
  - `portrait`: Which portrait from the actor's sprite sheet to use. If missing,
    do not change the sprite's portrait.
  - `hide`: Whether to hide the sprite. A `sprite` command with `hide: false`
    must be issued at least once to show the sprite. If a `hide: false` command
    is issues and `actor` or `portrait` are missing and the sprite has not been
    assigned an actor & portrait, the sprite will be hidden. In such scenario, a
    warning is issues to the engine console.
  - `pos`: The position to place the sprite. If missing, do not change the
    sprite's position. Sprites default to the center of the screen.
  - `size`: The size to scale the sprite to. If missing, do not change the
    sprite's size. Sprites must have a size set before they can be shown.
* `wait`: Waits for a certain amount of time.
  - `time`: The amount of time to wait in seconds.

[bundle file]: https://qeaml.github.io/nwge-docs/BUNDLE