class TickMarks
	SHARED_NODES_LOADER = AS::LazyLoader.new{
		ticks_texture_map = AS::ResourceCache.load('gfx/tickmarks.png'){|key|
			Photon::Texture::MIP.new(key)
		}
		
		state_hash = {:texture => ticks_texture_map, :blend => Photon::Blend::PremultAlpha}
		
		w = 80
		h = w/2.0
		
		# generate a vertex array with geometry for each ball in the texture
		var = Photon::VertexArray.new(2, 0, 2)
		5.times{|row|
			var.texcoord(0, (row + 0)*0.2); var.vertex(0, 0)
			var.texcoord(1, (row + 0)*0.2); var.vertex(w, 0)
			var.texcoord(1, (row + 1)*0.2); var.vertex(w, h)
			var.texcoord(0, (row + 1)*0.2); var.vertex(0, h)
		}
		
		# generate the final array of nodes for drawing
		Array.new(5){|i|
			Photon::Node::RenderState.construct(state_hash){
				Photon::Node::VAR.new(var, GL::QUADS, i*4, 4)
			}
		}
	}
	
	def self.offset_node(i, num, child)
		col, row = *i.divmod(2)
		
		matrix = Photon::Matrix.new.translate!(60*col + 110, 50 - 40*row)
		return Photon::Node::Matrix.new(GL::MODELVIEW, matrix, child)
	end
	
	def self.make_nodes(count)
		num, remain = *count.divmod(5)
		shared_nodes = SHARED_NODES_LOADER.load
		
		nodes = Array.new(num){|i| offset_node(i, num, shared_nodes.last)}
		nodes << offset_node(num, num, shared_nodes[remain - 1]) if remain > 0
		
		return nodes
	end
	
	attr_reader :view_node
	
	def initialize
		@count = 0
		@view_node = Photon::Node::Array.new([])
	end
	
	def count=(count)
		return if count == @count
		
		@count = count
		@view_node.reconstruct(self.class.make_nodes(count))
		
		AS::ResourceCache.load('sfx/stroke.ogg').play if count%5 == 0 and count != 0
	end
	
	def count
		@count
	end
	
end

class DeathBar
	
	attr_reader :view_node
	attr_reader :count
	
	UNDERDRAW = 50
	HEIGHT = 374
	FULL = 120
	MAX_COLOR = 1.0 / 3.0 # Hue of color, 0..1 all around the circle.
	
	def initialize
		@count = 0
		@value = 0
		@color = Photon::Color.new_hsv(MAX_COLOR, 1.0, 1.0, 1.0)
		bar_tex = AS::ResourceCache.load('gfx/bar.png')
		
		state_hash = {:texture => bar_tex, :color => @color, :blend => Photon::Blend::PremultAlpha}
			
		basic_matrix = Photon::Matrix.new.translate!(20,188- UNDERDRAW)
		matrix = Photon::Matrix.new
		@view_node = Photon::Node::Matrix.construct(GL::MODELVIEW, matrix){
			Photon::Node::Matrix.construct(GL::MODELVIEW, basic_matrix){
				Photon::Node::RenderState.construct(state_hash){
					Photon::Node::CopyRect.new(bar_tex, 0, 0, bar_tex.w, bar_tex.h, 0, 0, 70, HEIGHT + UNDERDRAW)
					#new(tex, sx, sy, sw, sh, dx, dy, dw, dh)
				}
			}
		}
		
	end
	
	def bar=(count)
		return if count == @count
		@count = count
	end
	
	def update
		percentage = (count.to_f / FULL.to_f)
		@value+= 0.15 * ((percentage * HEIGHT) - @value)	
		@view_node.matrix.reset!.translate!(0,-HEIGHT + @value)
		
		color = MAX_COLOR * (1.0 - percentage)
		@color.color = Photon::Color.new_hsv(color, 1.0, 1.0, 1.0)
	end
end

class AbstractGameBoard
	BoardWidth = 100
	BoardHeight = 75
	MaxPieces = 120
	
	attr_reader :view_node, :score, :balls, :effects
	
	BOTTOM_SEGMENT_TYPE = 9857
	
	def init_space
		@space = CP::Space.new
		
		@space.gravity = vec2(0, -100)
		@space.iterations = 5
		@effects = []
		
		init_collision_funcs
	end
	
	def create_border
		
		static_body = CP::StaticBody.new
		
		border = 20
		b = border
		# set up the border
		seg = CP::StaticShape::Segment.new(static_body, vec2(-b, 2*BoardHeight + b), vec2(-b, -b), border)
		seg.u = 1.0
		@space.add_object(seg)
		
		seg = CP::StaticShape::Segment.new(static_body, vec2(-b, -b), vec2(BoardWidth + b, -b), border)
		seg.u = 1.0
		seg.collision_type = BOTTOM_SEGMENT_TYPE
		@space.add_object(seg)
		
		seg = CP::StaticShape::Segment.new(static_body, vec2(-b, 2*BoardHeight+ b), vec2(BoardWidth+ b, 2*BoardHeight+ b), border)
		seg.u = 1.0
		@space.add_object(seg)
		
		seg = CP::StaticShape::Segment.new(static_body, vec2(BoardWidth+ b, -b), vec2(BoardWidth+ b, 2*BoardHeight+ b), border)
		seg.u = 1.0
		@space.add_object(seg)
		
	end
	
	SHAKE_AMOUNT_LAND = 0.3
	
	def init_collision_funcs
		sound_and_mutate = lambda{|a, b|
			AS::ResourceCache.load('sfx/thump.ogg').play(2.0**(6.0*(rand*2 - 1)/8.0))
			a.collision_type = a.ball.color
			@shake += SHAKE_AMOUNT_LAND
		}
		
		uncollided = AbstractBall::UNCOLLIDED_COLLISION_TYPE
		wildcard = AbstractBall::WILDCARD_COLOR
		# sound and mutate with levels
		@space.add_collision_func(uncollided, BOTTOM_SEGMENT_TYPE, &sound_and_mutate)
		
		# sound and mutate with wildcards
		@space.add_collision_func(uncollided, wildcard, &sound_and_mutate)

		# sound and mutate with other balls
		AbstractBall::Colors.each do|color|
			@space.add_collision_func(uncollided, color, &sound_and_mutate)
		end
		
		AbstractBall::Colors.each do|color|
			@space.add_collision_func(color, color){|a, b| @pairs << [a.ball, b.ball]}
		end
		
		AbstractBall::Colors.each do|color|
			@space.add_collision_func(color, wildcard){|a, b| @wildcard_pairs << [a.ball, b.ball]} #TODO finish me!
		end
	end
	
	LAZY_LINE_NODE = AS::LazyLoader.new{
		Photon::Node::VAR.ad_hoc(GL::QUADS, 2, 0, 2){|var|
			var.texcoord(0, 0.00); var.vertex(-0.125,-0.125)
			var.texcoord(1, 0.00); var.vertex( 1.125,-0.125)
			var.texcoord(1, 0.25); var.vertex( 1.125, 0.125)
			var.texcoord(0, 0.25); var.vertex(-0.125, 0.125)
		}
	}
	
	def make_view_node
		# solid normal lines
		lines_texture = AS::ResourceCache.load('gfx/line.png'){|key|
			Photon::Texture::MIP.new(key, :mip_smooth, :rgba)
		}
		
		shared_line_node = Photon::Node::Matrix.new(GL::MODELVIEW, Photon::Matrix.new, LAZY_LINE_NODE.load)
		line_state_hash = {:texture => lines_texture, :blend => Photon::Blend::Multiply}
		pairs_node = Photon::Node::RenderState.construct(line_state_hash){
			Photon::Node::Map.new(@pairs){|balla, ballb|
				a = balla.body.pos
				b = ballb.body.pos
				
				shared_line_node.matrix.reset!.bone_scale!(a.x, a.y, b.x, b.y)
				shared_line_node
			}
		}
		
		# dashed wildcard lines
		dashes_texture = AS::ResourceCache.load('gfx/dashes.png'){|key|
			Photon::Texture::MIP.new(key, :mip_smooth, :rgba, true)
		}
		
		line_state_hash = {:texture => dashes_texture, :blend => Photon::Blend::Multiply}
		wild_pairs_node = Photon::Node::RenderState.construct(line_state_hash){
			Photon::Node::Map.new(@wildcard_pairs){|balla, ballb|
				a = balla.body.pos
				b = ballb.body.pos
				
				shared_line_node.matrix.reset!.bone_scale!(a.x, a.y, b.x, b.y)
				shared_line_node
			}
		}
		
		# everything else
		return Photon::Node::Array.new([
			Photon::Node::Array.new(@ball_view_array = []),
			pairs_node,
			wild_pairs_node,
			Photon::Node::Map.new(@effects){|effect| effect.view_node},
		])
	end
	
	def make_score_node
		@tickmarks = TickMarks.new
		
		outline = AS::ResourceCache.load(:font, 'gfx/font_outline')
		font = AS::ResourceCache.load(:font, 'gfx/font')
		
		white = Photon::Color::White
		black = Photon::Color::Black
		
		scale = 0.2
		outline_scale = 2.69*scale
		
		@combo_node = Photon::Node::Array.new([
			AS::QuickTextNode.new(outline, @score.combo_string, 500, 50, outline_scale, white),
			AS::QuickTextNode.new(font, @score.combo_string, 500, 50, scale, black),
			
			@tickmarks.view_node,
		])
		
		return Photon::Node::Array.new([
			AS::QuickTextNode.new(outline, @score.score_string, 125, 568, outline_scale, white),
			AS::QuickTextNode.new(font, @score.score_string, 125, 568, scale, black),
			@combo_node,
		])
	end
	
	attr_reader :ticks, :shake
	
	def initialize(gamestate_delegate)
		@gamestate_delegate = gamestate_delegate
		
		@ticks = 0
		@pause_ticks = 0
		@bar = DeathBar.new
		
		@score = Score.new(self, gamestate_delegate)
		
		@balls = []
		@pairs = []
		@wildcard_pairs = []
		
		@ball_queue = []
		@notification_queue = []
		
		@next_notification = 0	
		
		init_space
		create_border
		@view_node = make_view_node
		
		@shake = 0.0
	end
	
	def ball_count
		@balls.length
	end
	
	def make_bar_node
		@bar.view_node
	end
	
	def reset_pairs
		@pairs.clear
		@wildcard_pairs.clear
	end
	
	def random_ball(ball_class)
		radius = ball_class.max_radius
 		pos = vec2(radius + rand(BoardWidth - 2*radius), 1.5*BoardHeight + radius)
		
		ball_class.new(pos)
	end
	
	def add_ball(ball)
		@balls << ball
		
		@space.add_object(ball)
		@ball_view_array << ball.view.view_node
	end
	
	def queue_ball(clazz)
		@ball_queue << random_ball(clazz)
	end
	
	def remove_ball(ball)
		ball = @balls.delete(ball)
		
		if ball
			@space.remove_object(ball)
			@ball_view_array.delete(ball.view.view_node)
			
			@pairs.delete_if{|a, b| a==ball or b==ball}
			@wildcard_pairs.delete_if{|a, b| a==ball or b==ball}
			
			@effects.unshift ParticleEffect.new(ball)
		end
	end
	
	def ball_at(pos)
		@balls.find{|ball| ball.selected?(pos) }
	end
	
	def balls_near(pos, radius)
		@balls.find_all{|ball| ball.selected?(pos, radius)}
	end
	
	SHAKE_DECAY = 0.80
	
	def update
		@effects.send_each(:tick)
		@effects.reject!{|effect| effect.done?}
		@effects.send_each(:update)
	
		@next_notification -= 1 if @next_notification > 0
		if @next_notification <= 0 && @notification_queue.length > 0
			add_effect(@notification_queue.shift)
			@next_notification = 110
		end
	
		# early exit during match pauses
		@pause_ticks -= 1
		return true if @pause_ticks > 0

		reset_pairs
		if @ticks%3==0 and !@ball_queue.empty?
			add_ball(@ball_queue.shift)
		end
			
		@space.step(1.0/60.0)
		@balls.each{|ball| ball.body.reset_forces}		

		graph = Graph.new(@balls, @pairs)
		graph.islands.each{|group| match_group(group) if group.length >= 4}
		
		@score.update_strings
		@ticks += 1

		update_game_extras
		return false
	end
	
	def update_game_extras
		@shake *= SHAKE_DECAY

		@bar.update if @bar

		#if @combo_node.enabled && !@score.combo?
		#	add_notification("Combo: " + @score.multiplier.to_s) if @score.multiplier > 3
		#end

		@combo_node.enabled = @score.combo?
		@tickmarks.count = @score.multiplier - 1
	end
	
	def match_wildcard(wild_ball)
		#find all pairs where any ball is touching this wildcard ball.
		matched_pairs = @wildcard_pairs.select{|a, b| a == wild_ball or b == wild_ball}
		
		# Expand the graph with these pairs.
		graph = Graph.new(@objects, matched_pairs + @pairs)
		island = graph.flood(wild_ball)
		match_group(island)
	end
	
	# pitches used for match sound playback
	Pitches = [0, 2, 4, 5, 7, 9, 11, 12, 14, 16].map{|note| 2**(note/12.0)}
	
	# Found a island of 4 or more touching balls. Called though normal update matching, 
	# or when a wildcard matches a group.
	def match_group(group)
		group.map{|ball| ball.special_sound}.compact.uniq.each{|sound| sound.play}
		
		expanded_group = group.inject(group){ |list, ball|
			list + ball.expand_group(self)
		}
		
		group_cleared(expanded_group.uniq)
	end
		
	def new_pause_ticks
		20
	end
	
	SHAKE_AMOUNT_BALL = 0.5
	
	# This group has been identified for removal, either for a normal match, or some
	# kind of special ball match. (A group might be everything hit by a bomb, for example)
	def group_cleared(group)
		group.each{|ball| match_out_ball(ball)}
		
		group_score = @score.group_cleared(group.length)
		
		# Find the average position and add a score floaty
		sum_positions = group.inject(vec2(0,0)){|sum, ball| sum += ball.body.p}
		add_effect(ScoreFloaty.new(sum_positions/group.length, "#{group_score}"))
		
		@shake += SHAKE_AMOUNT_BALL*group.length
		
		combo = @score.combo_multiplier
		AS::ResourceCache.load('sfx/ploop.ogg').play(Pitches[combo <= 9 ? combo : 9])
		
		@pause_ticks = new_pause_ticks
	end
	
	# This ball should be removed, as it was in a group that was cleared (matched or special).
	def match_out_ball(ball)
		remove_ball(ball)
		
		Stats[:ballsUnlocked]+=1 if ball.class == LockBall
		Stats[:totalBallsPopped]+=1
	end
	
	def click_out_ball(ball)
		remove_ball(ball)

		# Notify score and badge tracker of the click
		@score.ball_clicked(ball)
		
		Stats[:totalBallsPopped]+=1
		Stats[:totalBallsClicked]+=1
	end
	
	def rate(i)
		(MaxPieces*(1.15**-i)).to_i
	end
	
	def add_effect(effect)
		@effects << effect
	end
		
	# any text to display on screen, in the game
	def add_notification(text)
		@notification_queue << Notification.new(text.to_s)
	end

	def clamp_mouse(x, y, radius)
		maxx = BoardWidth - radius
		x = (x < radius) ? radius : ((x > maxx) ? maxx : x)
		
		y = (y < radius) ? radius : y
		
		return x, y
	end
	
end

class AbstractGameStateDelegate

	attr_accessor :action_listener
	attr_reader :ticks
	attr_reader :board
	
	BOARD_BOUNDS = [100, 108, 600, 450] # l, b, w, h
	
	def initialize(board)
		@board = board
		
		@ticks = 0

		@action_listener = AS::Event::ActionListener.new()
				
		if DEBUG_MODE
			@action_listener.action_for_key(?l){
				game_over
			}
			@action_listener.action_for_key(?1){
					@board.queue_ball(NormalBall)
			}
			@action_listener.action_for_key(?2){
					@board.queue_ball(LockBall)
			}
			@action_listener.action_for_key(?3){
					@board.queue_ball(PushBall)
			}
			@action_listener.action_for_key(?4){
					@board.queue_ball(BombBall)
			}
			@action_listener.action_for_key(?5){
					@board.queue_ball(StormBall)
			}
			@action_listener.action_for_key(?6){
					@board.queue_ball(WildcardBall)
			}			
		end
		
		board_matrix = Photon::Matrix.new.translate!(175.0/1.75, 188.0/1.75).scale!(6, 6)
		scaled_board_node = Photon::Node::Matrix.construct(GL::MODELVIEW, board_matrix){
			@board.view_node
		}
		
		bg_tex, fg_tex = ThemeManager.border_textures
		bg_node = Photon::Node::CopyRect.new(bg_tex, 0, 0, bg_tex.w, bg_tex.h, -80, 0, 960, 600)
		fg_node = Photon::Node::RenderState.construct({:blend => Photon::Blend::PremultAlpha}){
			Photon::Node::CopyRect.new(fg_tex, 0, 0, fg_tex.w, fg_tex.h, -80, 0, 960, 600)
		}
		
		@shake_matricies = Array.new(3){Photon::Matrix.new}
		
		@view_node = Photon::Node::Array.new([
			Photon::Node::Matrix.new(GL::MODELVIEW, @shake_matricies[0], bg_node),
			Photon::Node::Matrix.construct(GL::MODELVIEW, @shake_matricies[1], scaled_board_node),
			Photon::Node::Matrix.construct(GL::MODELVIEW, @shake_matricies[2]){
				Photon::Node::Array.new([
					@board.make_bar_node,
					fg_node,
					@board.make_score_node,
				])
			}
		])
	end
	
	def enter_state
		# preload crap
		[
			'sfx/stroke.ogg',
			'sfx/boom.ogg',
			'sfx/clank.ogg',
			'sfx/clunk.ogg',
			'sfx/lightning.ogg',
			'sfx/loop.ogg',
			'sfx/ploop.ogg',
			'sfx/pop.ogg',
			'sfx/spring.ogg',
			'sfx/stroke.ogg',
			'sfx/thump.ogg',
			'sfx/windstorm.ogg',
			'gfx/bar.png',
			'gfx/lightning.png',
		].each{|path|
			AS::ResourceCache.load(path)
		}
		
		[
			'gfx/bombpowerup.png',
			'gfx/boom.png',
			'gfx/dashes.png',
			'gfx/jumppowerup.png',
			'gfx/line.png',
			'gfx/lockpowerup.png',
			'gfx/particle.png',
			'gfx/stormpowerup.png',
			'gfx/tickmarks.png',
			'gfx/explosion.png',
		].each{|path|
			AS::ResourceCache.load(path){|key| Photon::Texture::MIP.new(key)}
		}
	end
	
	def mouse_to_board(x, y)
		l, b, w, h = *BOARD_BOUNDS
		vec2(100*(x - l)/w, 75*(y - b)/h)
	end
	
	IGNORE_KEYS = AS::Event::KeyboardEvent::Keysyms.values_at(
		:Key_RSHIFT,
		:Key_LSHIFT,
		:Key_RCTRL,
		:Key_LCTRL,
		:Key_RALT,
		:Key_LALT,
		:Key_RMETA,
		:Key_LMETA,
		:Key_CAPSLOCK
	)
	
	def input(event)
		return if @action_listener.call(event)

		pause if event.is_a?(AS::Event::KeyboardEvent) && event.state && !IGNORE_KEYS.include?(event.key)
	end
	
	def pause
		AS::GameState.stop(BaseState.new(PauseStateDelegate.new(self)))
	end
	
	def update
		@ticks += 1
		@board.update
		@board.score.game_state_changed(self)
	end
	
	def ball_count
		@board.ball_count
	end
	
	def draw
		AS.clear_screen
		
		shake = @board.shake
		@shake_matricies.each{|mat|
			mat.reset!.translate!(shake*2.0*-rand, shake*2.0*-rand).scale!(1.0 + shake/440.0, 1.0 + shake/300.0)
		}
		
		@view_node.render
	end
	
	def game_over
		@board.score.end_game(self)
		
		Stats[:totalGamesPlayed]+=1
		Stats[:totalTimePlayed]+=@ticks
		if(self.class == ClassicGameStateDelegate)
			Stats[:gamesPlayedClassic]+=1
			Stats[:timePlayedClassic]+=@ticks		
		elsif (self.class == GrabGameStateDelegate)
			Stats[:gamesPlayedGrab]+=1
			Stats[:timePlayedGrab]+=@ticks		
		elsif (self.class == DropGameStateDelegate)
			Stats[:gamesPlayedDrop]+=1
			Stats[:timePlayedDrop]+=@ticks		
		else
			Stats[:gamesPlayedOverflow]+=1
			Stats[:timePlayedOverflow]+=@ticks		
		end
		Stats[:longestSingleGame]=@ticks if @ticks > Stats[:longestSingleGame]
		
		Stats.save
	
		AS::GameState.stop(BaseState.new(GameOverStateDelegate.new(self)))
	end
	
end
