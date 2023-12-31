require 'src/load_classes.rb'

GamePrefs.open_screen

require 'src/load_screen.rb'

# Load and start music
Intro = MusicPlayer.new('sfx/intro.ogg', false)
Music = MusicPlayer.new('sfx/loop.ogg', true) #formerly loop.ogg

Intro.volume = GamePrefs.music_volume
Music.volume = GamePrefs.music_volume
Mixer.volume = GamePrefs.effects_volume

# Show the splash screen
AS.clear_color(1.0, 1.0, 1.0, 1.0)
LoadScreen.show
Intro.play
sleep 4.711
Music.play

AS.clear_color(0.0, 0.0, 0.0, 0.0)
AS::GameState.state_machine(MenuStateDelegate.get_state)
