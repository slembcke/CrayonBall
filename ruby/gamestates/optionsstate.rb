
class VolumeController < AS::MenuController
	def initialize(x, y, scale)
		super()
		@offset = x
		@y = y
		@scale = scale
		@padding = 20
	end
	
	def add_item(name, &block)
		super(name, @offset, @y, @scale, false, &block)
		@offset += @font.string_width(name) * @scale + @padding
	end
end

class OptionsStateDelegate
	
	def initialize(prevState)
		font = AS::ResourceCache.load(:font, 'gfx/font')

		@actions = AS::Event::ActionListener.new(MenuStateDelegate.sharedActions)

		@actions.action_for_key(:Key_ESCAPE){
			puts 'crap?'
			AS::GameState.stop(BaseState.new(prevState))
		}

		music_title, music = make_volume("Music: ", 400, 350, 0.27, font, :music_value=)
		sound_title, sound = make_volume("Sound Effects: ", 400, 280, 0.27, font, :effects_value=)
	
		music.make_active(GamePrefs.music_value)
		sound.make_active(GamePrefs.effects_value)
		
		screen_title, screen = make_screen_picker(400, 200, 0.27, font)
	
		back = AS::MenuController.new
		back.add_item("Done", 700, 40, 0.25, true){
			AS::GameState.stop(BaseState.new(prevState))
		}
		back.add_actions(@actions)
		
		@menu_controllers = [music, sound, screen, back]
		@background = MenuBackground.instance

		@scene = Photon::Node::Array.new([
			AS::QuickTextNode.new(font, "Crayon Ball Options", 400, 500, 0.5, Photon::Color::Black, true),
			music_title, music,
			sound_title, sound,
			screen_title, screen,

			back,
	#		screen_title, screen,
		])
	end
	
	def make_screen_picker(x, y, scale, font)
		title_string = "Screen Resolution:"
		
		back_offset = x - (font.string_width(title_string)*scale + 20)
		title = AS::QuickTextNode.new(font, title_string, back_offset, y, scale, Photon::Color::Black, false)
		
		picker = AS::MenuController.new

		picker.add_item("800x600", x, y, 0.27, false){
			GamePrefs.screen_value = 0
		}
		
		picker.add_item("1280x800", x, y - 50, 0.27, false){
			GamePrefs.screen_value = 1
		}
		
		picker.add_item("1440x900", x, y - 100, 0.27, false){
			GamePrefs.screen_value = 2
		}
		
		picker.add_item("fullscreen", x, y - 150, 0.27, false){
			GamePrefs.screen_value = 3
		}
		
		picker.make_active(GamePrefs.screen_value)
		picker.add_actions(@actions)
		
		return title, picker
	end
	
	def make_volume(title_string, x, y, scale, font, setter)

		back_offset = x - (font.string_width(title_string)*scale + 20)
		title = AS::QuickTextNode.new(font, title_string, back_offset, y, scale, Photon::Color::Black, false)
		option = VolumeController.new(x, y, scale)

		option.add_item("Off"){
			GamePrefs.method(setter).call(0)
		}

		option.add_item("1"){
			GamePrefs.method(setter).call(1)
		}

		option.add_item("2"){
			GamePrefs.method(setter).call(2)
		}

		option.add_item("3"){
			GamePrefs.method(setter).call(3)
		}

		option.add_item("4"){
			GamePrefs.method(setter).call(4)
		}

		option.add_actions(@actions)
		
		return title, option
	end
		
	def update
		@background.update
		@menu_controllers.send_each(:update)
	end
		
	def input(event)
		@actions.call(event)
	end
	
	def draw
		AS.clear_screen
		@background.view_root.render    
		@scene.render
	end
end
