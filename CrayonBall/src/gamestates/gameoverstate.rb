# Fixes the fading problem from giving up from the menustate or pausestate
# Fade back to the gamestate, then fade to the gameover state
# works by wrapping the gamestate delegate so that only the draw method is called.
class GameOverFadeHackState < AS::GameState
	
	def self.fade_to_game_then_game_over(old_game_delegate)
		FadeState.new(new(old_game_delegate), 15).run
		old_game_delegate.game_over
	end
	
	def update
		# don't delegate update
	end
	
end

class Animation
	attr_reader :ticks
	
	def initialize
		@ticks = 0
	end
	
	def tick
		@ticks += 1
	end
	
	def update(controller)
		# do nothing
	end
	
	def done?
		false
	end
	
	# called when the animation is added to a controller
	def start(controller)
		# do nothing
	end
	
	# called when the animation is removed from the controller
	def finish(controller)
		# do nothing
	end
end

class DurationAnimation < Animation
	def initialize(duration)
		@duration = duration
		@tick_scale = 1.0/(duration - 1)
		super()
	end

	def value
		@ticks*@tick_scale
	end
	
	def done?
		@ticks == @duration
	end
end

class AnimationController
	attr_reader :ticks
	
	def initialize
		@ticks = 0
		@animations = []
	end
	
	def <<(anim)
		@animations << anim
		anim.start(self)
	end
	
	def filter_animations
		finished, @animations = *@animations.partition{|anim| anim.done?}
		finished.send_each(:finish, self)
	end
	
	def update
		filter_animations
		
		@animations.send_each(:update, self)
		@animations.send_each(:tick)
		
		@ticks += 1
	end
end

class BlockAnimation < DurationAnimation
	def initialize(duration, &block)
		super(duration)
		@update_block = block
	end
	
	def update(controller)
		@update_block.call(self, controller)
	end
	
	def finish_block(&block)
		@finish_block = block
	end
	
	def finish(controller)
		@finish_block.call(self, controller) if @finish_block
	end
end

class ScoreCountAnimation < DurationAnimation
	def initialize(score, str)
		super(60)
		@score, @str = score, str
	end
	
	def update(controller)
		@str.replace("Score: #{(@score*value).to_i} points!")
	end
	
	def	finish(controller) 
		#puts 'finish'
	end
end

class ChainedAnimation < Animation
	def self.chain(*anims)
		anims.reverse.inject{|next_anim, this_anim| new(this_anim, next_anim)}
	end
	
	def self.[](*anims)
		chain(*anims)
	end
	
	def initialize(delegate, successor)
		@delegate, @successor = delegate, successor
	end
	
	def tick
		@delegate.tick
	end
	
	def done?
		@delegate.done?
	end
	
	def update(controller)
		@delegate.update(controller)
	end
	
	def start(controller)
		@delegate.start(controller)
	end
	
	def finish(controller)
		@delegate.finish(controller)		
		controller << @successor
	end
end

class BadgeNode < Photon::Node::Matrix
	class Animation < DurationAnimation
		def initialize(node, matrix, color)
			@node, @matrix, @color = node, matrix, color
			super(30)
		end
		
		def start(controller)
			@node.enabled = true
		end
		
		def update(controller)
			val = value
			valcomp = 1.0 - val
			
			scale = 1.0 + valcomp
			@matrix.reset!.sprite!(0.0, 0.0, scale, scale, valcomp)
			
			@color.color = Photon::Color.new(val, val, val, val)
		end
		
		def finish(controller)
			AS::ResourceCache.load('sfx/stomp.ogg').play
			@node.label_node.enabled = true
		end
	end
	
	attr_reader :anim
	attr_reader :label_node
	
	def pos_for_index(index)
		per_row = 5
		j, i = *index.divmod(per_row)
		
		x = 400 + ((i.to_f/(per_row - 1))*2 - 1 + j*0.25)*300
		y = 220 - j*135
		
		return x, y
	end
	
	def initialize(badge, index)
		tex = AS::ResourceCache.load('gfx/badges/' + badge.class.texture_name)
		color = Photon::Color::Red.clone
		
		scale = 0.5
		
		matrix = Photon::Matrix.new
		badge_node = Photon::Node::Matrix.construct(GL::MODELVIEW, matrix){
			Photon::Node::RenderState.construct({:blend => Photon::Blend::PremultAlpha, :color => color}){
				Photon::Node::SpriteTemplate.new(tex, tex.w/2, tex.h/2)
			}
		}
		
		font = AS::ResourceCache.load(:font, 'gfx/font')
		label_text = badge.count > 1 ? "#{badge.text} #{badge.count}x" : badge.text
		@label_node = AS::QuickTextNode.new(font, label_text, 0, -140, 0.25, Photon::Color::Black, true)
		@label_node.enabled = false
		
		x, y = *pos_for_index(index)
		trans_matrix = Photon::Matrix.new.translate!(x, y).scale!(scale, scale)
		super(GL::MODELVIEW, trans_matrix, Photon::Node::Array.new([badge_node, @label_node]))
		self.enabled = false
		
		@anim = Animation.new(self, matrix, color)
	end
end

class GameOverStateDelegate
	
	def initialize(game_delegate)
		@animation_done = false
		@score = game_delegate.board.score
		@mode = game_delegate.mode
		
		font = AS::ResourceCache.load(:font, 'gfx/font')
		title = AS::QuickTextNode.new(font, "Game Over!", 400, 530, 0.5, Photon::Color::Black, true)
		
		mc = AS::MenuController.new
		mc.enabled = false
		@menu_controller = mc

		highScore = HighScores[@mode].highscore?(@score)

		if ($registered && !highScore) 
			@replay = mc.add_item("Replay", 740, 60, 0.20, true){
				AS::GameState.stop(BaseState.new(game_delegate.class.new))
			}			
		end
		@back = mc.add_item("Done", 740, 20, 0.20, true){
			done()
		}
		
		@actions = AS::Event::ActionListener.new()
		mc.add_actions(@actions)
		
		score = @score.score
		score_str = "sanity!"
		final_score = AS::QuickTextNode.new(font, score_str, 100, 360, 0.3, Photon::Color::Black, false)

		coin_str = ""
		final_coins = AS::QuickTextNode.new(font, coin_str, 100, 300, 0.3, Photon::Color::Black, false)
		
		badges = @score.badges_earned
		badge_nodes = Array.new(badges.length){|i| BadgeNode.new(badges[i], i)}
		
		@scene = Photon::Node::Array.new([
			title,
			final_score,
			final_coins,
			mc,
		] + badge_nodes)
		
		coins = score / 500 + badges.inject(0){|sum, badge| sum + badge.count} * 10
		Stats[:currentCash]  += coins
		Stats[:totalCashEver] += coins
		
		@animation_controller = AnimationController.new

		badge_chain = ChainedAnimation.chain()

		array = [
			BlockAnimation.new(60){|anim, controller|
				AS::ResourceCache.load('sfx/clank.ogg').play if anim.ticks%4 == 0
				score_str.replace("Score: #{Misc.commas((score*anim.value).to_i)} points!")
			}
		]
		array += badge_nodes.map{|badge| badge.anim}
		array << BlockAnimation.new(60){|anim, controller|
			AS::ResourceCache.load('sfx/clank.ogg').play if anim.ticks%4 == 0
			coin_str.replace("Coins Earned: #{Misc.commas((coins*anim.value).to_i)}")
		}
		
		# wait 30 ticks till after the animation is done before responding to keys
		anim = BlockAnimation.new(30) {}
		anim.finish_block{
			@animation_done = true
			@menu_controller.enabled = true
		}
		array << anim

		@animation_controller << ChainedAnimation.chain(*array)
	end
	
	def make_underlay
		@screen_shot_texture = ScreenCopyTexture.new(:smooth, :rgb)
		
		state = {
			:blend => Photon::Blend::Alpha,
			:color => Photon::Color.new(1.0, 1.0, 1.0, 0.5),
			:texture => @screen_shot_texture
		}
		
		return Photon::Node::Array.new([
			transparentLayer = Photon::Node::RenderState.construct({:color => Photon::Color::White, :blend => Photon::Blend::PremultAlpha}){
				Photon::Node::VAR.ad_hoc(GL::QUADS, 2, 0, 0) { | var|
					var.vertex(-80, 600)
					var.vertex(-80,   0)
					var.vertex(880,   0)
					var.vertex(880, 600)
				}
			},
			Photon::Node::RenderState.construct(state){
				w, h = @screen_shot_texture.w, @screen_shot_texture.h
				Photon::Node::CopyRect.new(@screen_shot_texture, 0, 0, w, h, *BaseState.screen_bounds)
			}
		])
	end
	
	def enter_state
		@underlay = make_underlay
	end
	
	def exit_state
		@screen_shot_texture.release
	end
	
	def input(event)
		@actions.call(event)

		return unless event.any_key? and @animation_done
		
		done
	end
	
	def done()
		next_delegate_class = $registered ? HighScoreStateDelegate : GameOverNagStateDelegate
		AS::GameState.stop(BaseState.new(next_delegate_class.new(self, @score, @mode, true)))
	end
	
	def update
		@menu_controller.update
		@animation_controller.update
	end
	
	def draw
		AS.clear_screen
		
		@underlay.render
		@scene.render
	end
	
end
