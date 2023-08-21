	
class MenuStateDelegate

	@@state = nil

	def self.sharedActions
		action_listener = AS::Event::ActionListener.new
		
		action_listener.action_for_key(:Key_ESCAPE){
			AS::GameState.stop(MenuStateDelegate.get_state)
		}
		return action_listener
	end

	def initialize
		top = 340
		side = 220
		spacing = 80
		
		font = AS::ResourceCache.load(:font, 'gfx/font')

		title_tex = AS::ResourceCache.load('gfx/titlelogo.png')
		title = Photon::Node::RenderState.construct({:blend => Photon::Blend::PremultAlpha}){
			w = title_tex.w/1.75
			h = title_tex.h/1.75
			Photon::Node::CopyRect.new(title_tex, 0, 0, title_tex.w, title_tex.h, 400 - w/2, 490 - h/2, w, h)
		}
		
		mc = AS::MenuController.new
		@menu_controller = mc
		@new_game = mc.add_item("Play", side, top, 0.35, true){
			AS::GameState.stop(BaseState.new(NewGameStateDelegate.new(self)))
		}
		mc.add_item("Options", side, top - spacing, 0.35, true){
			AS::GameState.stop(BaseState.new(OptionsStateDelegate.new(self)))
		}
		mc.add_item("High Scores", side, top - spacing * 2, 0.35, true){
			AS::GameState.stop(BaseState.new(HighScoreStateDelegate.new(self)))
		}
		
		side = 580
		mc.add_item("Stats", side, top, 0.35, true){
			AS::GameState.stop(BaseState.new(StatsStateDelegate.new(self)))
		}
		mc.add_item("Themes", side, top - spacing, 0.35, true){
			AS::GameState.stop(BaseState.new(ThemePickerStateDelegate.new))
		}
		mc.add_item("How to Play", side, top - spacing * 2, 0.35, true){
			AS::GameState.stop(BaseState.new(HowToPlayStateDelegate.new(self)))
		}
		
		unless $registered
			mc.add_item("Buy Now!", 400, top - spacing * 3, 0.35, true){
				Misc.web_page
			}
		end
		
		mc.add_item("Quit", 700, 40, 0.35, true){
			AS::GameState.stop()
		}
				
		@actions = AS::Event::ActionListener.new
		@actions.action_for_key(:Key_1){
			#puts "reloading state"
			ReloadableFileList.reload
			AS::GameState.stop(self.class.reload_state)
		} if DEBUG_MODE
		
		mc.add_actions(@actions)
		@background = MenuBackground.instance

		@scene = Photon::Node::Array.new([
			title,
			mc,
			AS::QuickTextNode.new(font, "v#{VersionString}", 6, 32, 0.15, Photon::Color::Black),
			AS::QuickTextNode.new(font, @registered_str="", 6, 12, 0.15, Photon::Color::Black),
		])
		
		@scene.children << AS::QuickTextNode.new(font, "DEBUG MODE ON", 50, 50, 0.5, Photon::Color::Red) if DEBUG_MODE
	end
	
	def enter_state
		minutes = Misc.remaining_trial
		
		if $registered
			@registered_str.replace("")
		elsif minutes > 0
				@registered_str.replace("#{minutes} minutes left in the trial version.")
		else
			AS::GameState.stop(BaseState.new(GameOverNagStateDelegate.new))
		end
	end
	
	def self.get_state
		return (@@state or reload_state)
	end
	
	def self.reload_state
			return @@state = BaseState.new(MenuStateDelegate.new)
	end
	
	def update
		@background.update
		@menu_controller.update
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
