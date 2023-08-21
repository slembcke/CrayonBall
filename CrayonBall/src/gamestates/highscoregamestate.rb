class HighScoreStateDelegate
	
	SCORER_FONT_SCALE = 0.15
	
	# score parameter is an object of class Score, for the points, see score.score
	def initialize(prevState, score = :no_score, mode = :classic, fromPlay = false)
		@mode = mode
	
		@entered_name = "<Enter your name>"
		@entered_name = GamePrefs.prev_name.clone if GamePrefs.prev_name
		
		@actions = AS::Event::ActionListener.new(MenuStateDelegate.sharedActions)
		@background = MenuBackground.instance
		@input = AS::Event::StringInput.new(@entered_name, 25)
		@ticks = 0
		
		view_nodes = []
		
		font = AS::ResourceCache.load(:font, 'gfx/font')
		
		mode_picker = AS::MenuController.new
		mode_picker.add_item("Classic", 80, 550, 0.3, true){
			save_score_if_needed
			AS::GameState.stop(BaseState.new(HighScoreStateDelegate.new(prevState, :no_score, :classic)))
		}
		
		mode_picker.add_item("Overflow", 285, 550, 0.3, true){
			save_score_if_needed
			AS::GameState.stop(BaseState.new(HighScoreStateDelegate.new(prevState, :no_score, :overflow)))
		}
		
		mode_picker.add_item("Injection", 490, 550, 0.3, true){
			save_score_if_needed
			AS::GameState.stop(BaseState.new(HighScoreStateDelegate.new(prevState, :no_score, :drop)))
		}
		
		mode_picker.add_item("Shuffle", 700, 550, 0.3, true){
			save_score_if_needed
			AS::GameState.stop(BaseState.new(HighScoreStateDelegate.new(prevState, :no_score, :grab)))
		}
		
		mode_picker.make_active(HighScoreList::Modes.index(@mode))
		mode_picker.add_actions(@actions)

		
		mc = AS::MenuController.new
		mc.add_item("Back", 700, 15, 0.25){
			save_score_if_needed
			AS::GameState.stop(BaseState.new(MenuStateDelegate.get_state))
		}
		mc.add_item("Clear High Scores", 15, 15, 0.25){
			HighScores[@mode].clear
			AS::GameState.stop(BaseState.new(HighScoreStateDelegate.new(self, :no_score, @mode)))
		}
		
		
		mc.add_item("Replay", 520, 15, 0.25){
			save_score_if_needed
			if @mode == :classic
				AS::GameState.stop(BaseState.new(ClassicGameStateDelegate.new))
			elsif @mode == :grab
				AS::GameState.stop(BaseState.new(GrabGameStateDelegate.new))
			elsif @mode == :overflow
				AS::GameState.stop(BaseState.new(OverflowGameStateDelegate.new))
			else
				AS::GameState.stop(BaseState.new(DropGameStateDelegate.new))
			end
		}	if fromPlay
	
	
		mc.add_actions(@actions)
	
		if score != :no_score && HighScores[@mode].highscore?(score)
			@new_high_score = HighScores[@mode].add("", score)
		end
		
		HighScores[@mode].scores.each_with_index{|hs, i|
		  new_score = (hs == @new_high_score)
			color = (new_score) ? Photon::Color::Blue : Photon::Color::Black
			y = 485 - 66 * i
			view_nodes <<  AS::QuickTextNode.new(font, (i+1).to_s + ")", 25, y, SCORER_FONT_SCALE, color)
			view_nodes <<  AS::QuickTextNode.new(font, (new_score) ? @entered_name : hs.name, 70, y, SCORER_FONT_SCALE, color)
			view_nodes <<  AS::QuickTextNode.new(font, hs.score_string, 630, y, SCORER_FONT_SCALE, color)
	
			hs.badges.each_with_index{|b, i|
				x = i * 90+ 85
				tex = AS::ResourceCache.load('gfx/badgesTiny/' + b.class.texture_name)
				
				view_nodes << AS::QuickTextNode.new(font, b.count.to_s + "x ", x, y - 25, SCORER_FONT_SCALE, color)
				view_nodes << get_badge(tex, x+42, y - 15)
			}	
		}
		
		@menu_controllers = [mode_picker, mc]
		
		@scene = Photon::Node::Array.new(view_nodes + [
			mc,
			mode_picker,
		])
				
	end
	
	def save_score_if_needed
		if @new_high_score
			@input.finalize_string
			save_score(@input.final_string)
		end
	end
	
	def get_badge(tex, x, y)
		matrix = Photon::Matrix.new.translate!(x, y)
		
		return Photon::Node::Matrix.construct(GL::MODELVIEW, matrix){
				Photon::Node::RenderState.construct({:blend => Photon::Blend::PremultAlpha}) {
					Photon::Node::SpriteTemplate.new(tex, tex.w/2, tex.h/2)
				}
			}
	end
	
	def update
		@background.update
		@menu_controllers.send_each(:update)
		
		@ticks += 1
		
    if @new_high_score
			@input.toggle_caret if @ticks % 30 == 0 

			final_input = @input.final_string  
      if final_input
        save_score(final_input)
     		AS::GameState.stop(BaseState.new(MenuStateDelegate.get_state))
      end
    end
	end
	
	def save_score(final_name)
	  @new_high_score.name = final_name
	  GamePrefs.prev_name = final_name
	  GamePrefs.save
		HighScores[@mode].save
	end
	
	def input(event)
		@actions.call(event)
		@input.call(event)
	end
	
	def draw
		AS.clear_screen
		@background.view_root.render    
		@scene.render
	end
end
