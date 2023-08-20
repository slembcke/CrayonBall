
GAME_STORAGE_PATH = case RUBY_PLATFORM
	when /linux/: './'
	when /darwin/: File.expand_path('~/Library/Application Support/CrayonBall/')
	when /mingw/:  ENV['APPDATA'] ? ENV['APPDATA'] + '/Scribball/' : 'prefs'
end

prefsClass = Class.new do
	attr_reader :effects_value, :music_value, :screen_value
	attr_accessor :prev_name

	SCREEN_VALUES = [[800, 600], [1280, 800], [1440, 900], [0, 0]]
	SOUND_VALUES = [0.0, 0.25**2, 0.5**2, 0.75**2, 1.0]
	
	def find_screen_fit
		index = 0
		
		for i in (1...(SCREEN_VALUES.length - 1)) do
			w, h = SCREEN_VALUES[i]
			break if(w >= AS.native_w or h >= AS.native_h)
			
			index = i
		end
		
		return index
	end

	def initialize
		@screen_value = find_screen_fit
		@effects_value = 4
		@music_value = 3
		@prev_name = ""
		
		load
	end
	
	def open_screen
		prefw, prefh = *GamePrefs.screen_resolution
		AS.open_screen(prefw, prefh, :vbl_sync => true, :fullscreen => GamePrefs.fullscreen?, :name => "Crayon Ball")
		
		AS.clear_screen
		BaseState.set_screen_size(AS.screen_w, AS.screen_h)
	end
	
	def screen_value=(value)
		@screen_value = value
		save
		
		#dump textures, change the screen, and reload textures
		ObjectSpace.each_object(Photon::Texture){|tex| tex.release}
		open_screen
		AS.clear_screen
		ObjectSpace.each_object(Photon::Texture){|tex| tex.load}
	end

	def screen_resolution
		SCREEN_VALUES[@screen_value]
	end
	
	def fullscreen?
		screen_resolution == [0,0]
	end
	
	def effects_value=(value)
		@effects_value = value
		Mixer.set_vol_with_preview(effects_volume)
		save
	end
	
	def effects_volume
		SOUND_VALUES[@effects_value]
	end
	
	def music_value=(value)
		@music_value = value
		Music.volume = music_volume
		save
	end
	
	def music_volume
		SOUND_VALUES[@music_value]
	end
	
	PrefsPath = File.join(GAME_STORAGE_PATH, 'prefs.ser')
	
	def load
		# create the folder the first time.
		Dir.mkdir(GAME_STORAGE_PATH) unless File.exist?(GAME_STORAGE_PATH)
		# create the prefs file the first time
		save unless File.exist?(PrefsPath)
	
		open(PrefsPath, 'r'){|f|
			hash = Marshal.load(f)
			@screen_value = hash[:screen_value]
			@effects_value = hash[:effects_value]
			@music_value = hash[:music_value]
			@prev_name = hash[:prev_name]
		}
	end
	
	def save
		hash = {
			:screen_value => @screen_value,
			:effects_value => @effects_value,
			:music_value => @music_value,
			:prev_name => @prev_name,
		}
	
		open(PrefsPath, 'w'){|f| Marshal.dump(hash, f)}
		
		update_permissions
	end
	
	def update_permissions
		File.chmod(0777, GAME_STORAGE_PATH)

		Dir.foreach(GAME_STORAGE_PATH) {|f|
			next if f == ".."
			File.chmod(0777, File.join(GAME_STORAGE_PATH.to_s, f.to_s))
		}

	end
	
end

GamePrefs = prefsClass.new
