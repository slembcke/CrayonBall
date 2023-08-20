class GameOverNagStateDelegate
	
	def initialize(*args)
		@args = args
		@ticks = 0
		xoff = 680
		
		font = AS::ResourceCache.load(:font, 'gfx/font')

		mc = AS::MenuController.new
		@menu_controller = mc
		mc.add_item("Join us on Facebook", 190, 47, 0.25, true){
			Misc.web_page("http://www.facebook.com/pages/ScribBall/38619059480?ref=hms_scribball_page")
			goto_next_state
		}
		mc.add_item("Now for iPhone!", xoff, 130, 0.25, true){
			Misc.web_page("http://howlingmoonsoftware.com/fromDesktopToiPhone.php")
			goto_next_state
		}
		
		facebook = AS::QuickTextNode.new(font, "and save $5 or more!", 190, 17, 0.15, Photon::Color::Black, true)
    mc.add_item("Buy Now!", xoff, 70, 0.25, true){
			Misc.web_page
			goto_next_state
		}
		mc.add_item(@args.empty? ? "Quit" : "Continue", xoff, 30, 0.25, true){
			goto_next_state
		}
		
		@actions = AS::Event::ActionListener.new(MenuStateDelegate.sharedActions)
		@actions.action_for_key(:Key_1){
			#puts "reloading state"
			ReloadableFileList.reload
			AS::GameState.stop(self.class.reload_state)
		} if DEBUG_MODE

		mc.add_actions(@actions)
		@background = MenuBackground.instance

		tex = AS::ResourceCache.load('gfx/gameover.png')
		
		title = @args.empty? ? "Your Trial Has Expired!" : "Thanks for Playing!"
		
		@scene = Photon::Node::Array.new([
			Photon::Node::CopyRect.new(tex, 0, 0, tex.w, tex.h, -80, 0, 960, 600),
			AS::QuickTextNode.new(font, title, 400, 550, 0.4, Photon::Color::Black, true),
			facebook,
			mc,
		])
	end
	
	def goto_next_state
		if @args.empty?
			AS::GameState.stop
		else
			AS::GameState.stop(BaseState.new(HighScoreStateDelegate.new(*@args)))
		end
		
	end
	
	def self.reload_state
			return @@state = BaseState.new(GameOverNagStateDelegate.new)
	end
	
	def input(event)
		@actions.call(event)
	end
	
	def update
		@menu_controller.update
	end
	
	def draw
		AS.clear_screen
		@scene.render
	end
	
end
