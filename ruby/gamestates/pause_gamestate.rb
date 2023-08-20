class PauseStateDelegate
	
	def initialize(prev_state)
		@prev_state = prev_state
		
		font = AS::ResourceCache.load(:font, 'gfx/font')
		title = AS::QuickTextNode.new(font, "Paused", 400, 450, 0.5, Photon::Color::Black, true)
		
		mc = AS::MenuController.new
		@menu_controller = mc
		mc.add_item("Resume", 400, 350, 0.25, true){
			AS::GameState.stop(BaseState.new(prev_state))
		}
		mc.add_item("Give Up", 400, 300, 0.25, true){
			GameOverFadeHackState.fade_to_game_then_game_over(@prev_state)
		}
		mc.add_item("Options", 400, 250, 0.25, true){
			AS::GameState.stop(BaseState.new(OptionsStateDelegate.new(self)))
		}
		mc.add_item("Quit Now", 400, 200, 0.25, true){
			AS::GameState.stop()
		}

		@actions = AS::Event::ActionListener.new

		mc.add_actions(@actions)

		@scene = Photon::Node::Array.new([
			title,
			mc,
		])
	end
	
	def enter_state
		@prev_state.draw
	
		color = Photon::Color.new(1.0, 1.0, 1.0, 0.5)
		tex = ScreenCopyTexture.new(:smooth, :rgb)

		@underlay = Photon::Node::Array.new([
			transparentLayer = Photon::Node::RenderState.construct({:color => Photon::Color::White, :blend => Photon::Blend::PremultAlpha}){
				Photon::Node::VAR.ad_hoc(GL::QUADS, 2, 0, 0) { | var|
					var.vertex(-80, 600)
					var.vertex(-80,   0)
					var.vertex(880,   0)
					var.vertex(880, 600)
				}
			},
			Photon::Node::RenderState.construct({:blend => Photon::Blend::Alpha, :color => color, :texture => tex}){
				w, h = tex.w, tex.h
				Photon::Node::CopyRect.new(@screen_shot_texture, 0, 0, w, h, *BaseState.screen_bounds)
			}
		])
	end
	
	def input(event)
		return if @actions.call(event)
		AS::GameState.stop(BaseState.new(@prev_state)) if event.any_key?
	end
	
	def update
		@menu_controller.update
	end
	
	def draw
		AS.clear_screen
		@underlay.render 
		@scene.render
	end
end
