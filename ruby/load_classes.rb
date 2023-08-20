VersionString = '2.3.1'
DEBUG_MODE = true unless defined?(DEBUG_MODE)

if DEBUG_MODE
	#puts "registered: #{$registered}"
	#require 'rubygems'; require 'ruby-debug'
end

AS.show_mouse = false

CP.collision_slop = 0.2

# These ruby files load various info from the disk.
require 'ruby/gameprefs.rb'

# Loads and plays the music.
require 'ruby/music.rb'

# Load and init the sound mixer
require 'ruby/sound_mixer.rb'

require 'ruby/gamestates/basestates.rb'

ReloadableFileList = [
	'ruby/menu.rb',
	'ruby/misc.rb',
	'ruby/as_extras.rb',
	'ruby/thememanager.rb',
	'ruby/balls.rb',
	'ruby/mouse.rb',
	'ruby/gamestates/menustate.rb',
	'ruby/gamestates/newgamestate.rb',
	'ruby/gamestates/highscoregamestate.rb',
	'ruby/gamestates/statsgamestate.rb',
	'ruby/gamestates/optionsstate.rb',
	'ruby/gamestates/shared_gamestate.rb',
	'ruby/gamestates/classic_gamestate.rb',
	'ruby/gamestates/overflow_gamestate.rb',
	'ruby/gamestates/drop_gamestate.rb',
	'ruby/gamestates/grab_gamestate.rb',
	'ruby/gamestates/pause_gamestate.rb',
	'ruby/gamestates/how_to_play_state.rb',
	'ruby/gamestates/gameoverstate.rb',
	'ruby/gamestates/gameover_nag_state.rb',
	'ruby/gamestates/theme_picker_state.rb',
	'ruby/graph.rb',
	'ruby/badge.rb',
	'ruby/score.rb',
	'ruby/effects.rb',
	'ruby/menubackground.rb',
]

ReloadableFileList.each{|file| require(file)}

def ReloadableFileList.reload
	self.each{|file| load(file)}
end

# These ruby files load various info from the disk.
require 'ruby/stats.rb'
require 'ruby/highscores.rb'
