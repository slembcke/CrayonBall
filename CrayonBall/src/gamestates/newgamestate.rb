class NewGameStateDelegate
	
	def load_splash
		
		#splash
		outline = AS::ResourceCache.load(:font, 'gfx/font_outline')
		font = AS::ResourceCache.load(:font, 'gfx/font')
		
		white = Photon::Color.new(231.0/255.0, 144.0/255.0, 59.0/255.0)
		black = Photon::Color::Black
		
		scale = 0.5
		outline_scale = 2.69*scale
		string = "Loading!"

		Photon::Node::Array.new([
			Photon::Node::RenderState.construct(:color => Photon::Color.new(1,1,1,0.7), :blend => Photon::Blend::Alpha){
				Photon::Node::VAR.ad_hoc(GL::QUADS, 2, 0, 0){|var|
					var.vertex(-80, 600)
					var.vertex(-80,   0)
					var.vertex(880,   0)
					var.vertex(880, 600)
				}
			},
			AS::QuickTextNode.new(outline, string, 400, 300, outline_scale, white, true),
			AS::QuickTextNode.new(font, string, 400, 300, scale, black, true),
		]).render
		AS.swap_buffers
	end
	
	def initialize(prevState)
		font = AS::ResourceCache.load(:font, 'gfx/font')

		title = AS::QuickTextNode.new(font, "Start a New Game", 400, 540, 0.5, Photon::Color::Black, true)

		top = 450
		spacing = 65
		
		@notification = ""
	 	@notification_node = AS::QuickTextNode.new(font, @notification, 400, top - 4.7*spacing, 0.2, Photon::Color::Black, true)
		
		mc = AS::MenuController.new
		@menu_controller = mc
		item = mc.add_item("Classic Mode", 400, top, 0.30, true){
			load_splash
			AS::GameState.stop(BaseState.new(ClassicGameStateDelegate.new))
		}
		item.set_inside_callback{
			@notification_node.str = "Classic Mode:\n You lose when the bar fills up, which indicates\n 120 on-screen balls. Match groups of four\n colors to remove them!"
		}
		
		item = mc.add_item("Overflow Mode", 400, top - spacing, 0.30, true){
			load_splash
			AS::GameState.stop(BaseState.new(OverflowGameStateDelegate.new))
		}
		item.set_inside_callback{
			@notification_node.str = "Overflow Mode:\n The screen stays full of balls, so keep matching\n balls to keep the bar low. The bar fills up with each\n click and over time; you lose when it's full!"
		}
		
		item = mc.add_item("Injection Mode", 400, top - 2 * spacing, 0.30, true){
			load_splash
			AS::GameState.stop(BaseState.new(DropGameStateDelegate.new))
		}
		item.set_inside_callback{
			@notification_node.str = "Injection Mode:\n Click anywhere to add a ball. There's no\n penalty for clicking like the other\n modes. This is a fast action mode!"
		}
	
			item = mc.add_item("Shuffle Mode", 400, top - 3 * spacing, 0.30, true){
			load_splash
			AS::GameState.stop(BaseState.new(GrabGameStateDelegate.new))
		}
		item.set_inside_callback{
			@notification_node.str = "Shuffle Mode:\n Drag balls around to make matches. The screen\nstays full like in overflow mode. Match fast\nto keep the bar low!"
		}
	
	
		mc.add_item("Back", 700, 40, 0.25, true){
			AS::GameState.stop(BaseState.new(prevState))
		}
		
		@actions = AS::Event::ActionListener.new(MenuStateDelegate.sharedActions)

		mc.add_actions(@actions)
		@background = MenuBackground.instance

		@scene = Photon::Node::Array.new([
			title,
			@notification_node,
			mc,
		])
	end
	
	def update
		@background.update
		@menu_controller.update
	end
		
	def input(event)
		@actions.call(event) unless @guilt_ticks
	end
	
	def draw
		AS.clear_screen
		@background.view_root.render    
		@scene.render
	end
end
