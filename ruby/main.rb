require 'ruby/load_classes.rb'

GamePrefs.open_screen

require 'ruby/load_screen.rb'

# Load and start music
Intro = MusicPlayer.new('sfx/intro.ogg', false)
Music = MusicPlayer.new('sfx/loop.ogg', true) #formerly loop.ogg

Intro.volume = GamePrefs.music_volume
Music.volume = GamePrefs.music_volume
Mixer.volume = GamePrefs.effects_volume

Thread.new {
	Intro.play
	sleep 4.711
	Music.play
}

# Show the splash screen
AS.clear_color(1.0, 1.0, 1.0, 1.0)
LoadScreen.show
sleep 1.5

AS.clear_color(0.0, 0.0, 0.0, 0.0)
AS::GameState.state_machine(MenuStateDelegate.get_state)
