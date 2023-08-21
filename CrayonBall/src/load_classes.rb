VersionString = '2.3.2'
DEBUG_MODE = false unless defined?(DEBUG_MODE)

if DEBUG_MODE
	#puts "registered: #{$registered}"
	#require 'rubygems'; require 'ruby-debug'
end

AS.show_mouse = false

CP.collision_slop = 0.2

# These ruby files load various info from the disk.
require 'src/gameprefs.rb'

# Loads and plays the music.
require 'src/music.rb'

# Load and init the sound mixer
require 'src/sound_mixer.rb'

require 'src/gamestates/basestates.rb'

ReloadableFileList = [
	'src/menu.rb',
	'src/misc.rb',
	'src/as_extras.rb',
	'src/thememanager.rb',
	'src/balls.rb',
	'src/mouse.rb',
	'src/gamestates/menustate.rb',
	'src/gamestates/newgamestate.rb',
	'src/gamestates/highscoregamestate.rb',
	'src/gamestates/statsgamestate.rb',
	'src/gamestates/optionsstate.rb',
	'src/gamestates/shared_gamestate.rb',
	'src/gamestates/classic_gamestate.rb',
	'src/gamestates/overflow_gamestate.rb',
	'src/gamestates/drop_gamestate.rb',
	'src/gamestates/grab_gamestate.rb',
	'src/gamestates/pause_gamestate.rb',
	'src/gamestates/how_to_play_state.rb',
	'src/gamestates/gameoverstate.rb',
	'src/gamestates/gameover_nag_state.rb',
	'src/gamestates/theme_picker_state.rb',
	'src/graph.rb',
	'src/badge.rb',
	'src/score.rb',
	'src/effects.rb',
	'src/menubackground.rb',
]

ReloadableFileList.each{|file| require(file)}

def ReloadableFileList.reload
	self.each{|file| load(file)}
end

# These ruby files load various info from the disk.
require 'src/stats.rb'
require 'src/highscores.rb'
