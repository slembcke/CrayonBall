class HowToPlayStateDelegate < AbstractGameStateDelegate

	def initialize(prevState)
		font = AS::ResourceCache.load(:font, 'gfx/font')
		@actions = AS::Event::ActionListener.new(MenuStateDelegate.sharedActions)

		@board = HelpGameBoard.new(self)
		
		@ticks = 0

		@actions.action_for_key(:Key_1){
			#puts "reloading state"
			ReloadableFileList.reload
			AS::GameState.stop(self.class.reload_state)
		} if DEBUG_MODE
		
		@mc = AS::MenuController.new
		@mc.add_item("Back", 700, 40, 0.25, true){
			AS::GameState.stop(BaseState.new(prevState))
		}
		@mc.add_actions(@actions)
		
		@shake_matricies = []
		
		bg_tex = AS::ResourceCache.load('gfx/helpBack.png')
		bg_node = Photon::Node::CopyRect.new(bg_tex, 0, 0, bg_tex.w, bg_tex.h, -80, 0, 960, 600)

		create_text_triggers

#
		board_matrix = Photon::Matrix.new.scale!(6, 6)
		scaled_board_node = Photon::Node::Matrix.construct(GL::MODELVIEW, board_matrix){
			@board.view_node
		}

		@view_node = Photon::Node::Array.new([
			#AS::QuickTextNode.new(font, "How to Play", 400, 500, 0.3, Photon::Color::Black, true),
			bg_node,
			@mc,
			@click_specials,
			scaled_board_node,
		])
	end
	
	def self.reload_state
		return @@state = BaseState.new(HowToPlayStateDelegate.new(nil))
	end

	def create_text_triggers
		@click_specials = AS::MenuController.new
		top = 326
		@click_specials.add_item("Storm", 30+25, top, 0.21, true){
				@board.ball_factory = StormToPopBallFactory.new
		}
		@click_specials.add_item("Bomb", 183+15, top, 0.21, true){
				@board.ball_factory = BombToPopBallFactory.new
		}
		@click_specials.add_item("Wildcard", 340+10, top, 0.21, true){
				@board.ball_factory = WildCardPopBallFactory.new
		}

		top = 171
		@click_specials.add_item("Bomb", 100+30, top, 0.21, true){
			@board.ball_factory = BombBallFactory.new
		}
		@click_specials.add_item("Storm", 240+20, top, 0.21, true){
			@board.ball_factory = StormBallFactory.new 
		}
		
		@click_specials.add_actions(@actions)

	end

	def update
		super
		@mc.update
		@click_specials.update
	end
		
	def input(event)
		@actions.call(event)
	end
	
end

class HelpGameBoard < AbstractGameBoard

	attr_reader :ball_factory

	def initialize(*args)
		super
		@ball_factory = NormalBallFactory.new
	end
	
	def ball_factory=(bf)
		clear_board
		@ball_factory = bf
	end
	
	def update
		super
		
			
		#pop balls that we might have added for that purpose.
		if (popMe = @ball_factory.last_to_pop) != nil && @ball_factory.last_add_time <= 0
				
			popMe.set_magnitude(100) if popMe.respond_to?(:set_magnitude)
			
			popMe.clicked_on(self)
			remove_ball(popMe)
			@ball_factory.last_to_pop = nil
		end
		
		@ball_factory.last_add_time-=1
		
		add_pieces_on_timer
	end

	# Add pieces to the gameboard occasionally. We pace on the elapsed time.
	def add_pieces_on_timer
		if @gamestate_delegate.ticks%50 == 0 
			add_ball(@ball_factory.random_ball)
		end
	end
	
	def clear_board
		mah_balls = @balls.clone
		
		mah_balls.each{|ball|
			remove_ball(ball)
		}
	end
	
	def create_border
		
		static_body = CP::StaticBody.new
		
		border = 1.5+3
		offset = 24
		# set up the border
		seg = CP::StaticShape::Segment.new(static_body, vec2(482-offset/2, 600-256-offset) / 6.0, vec2(548-offset/2, 600-374-offset) / 6.0, border)
		seg.u = 1.0
		@space.add_object(seg)
		
		seg = CP::StaticShape::Segment.new(static_body, vec2(548-offset/2, 600-374-offset) / 6.0, vec2(705+offset/2, 600-374-offset) / 6.0, border)
		seg.u = 1.0
		seg.collision_type = BOTTOM_SEGMENT_TYPE
		@space.add_object(seg)
		
		seg = CP::StaticShape::Segment.new(static_body, vec2(705+offset/2, 600-374-offset) / 6.0, vec2(774+offset/2, 600-256-offset) / 6.0, border)
		seg.u = 1.0
		@space.add_object(seg)
		
	end

	def update_game_extras
		#empty
	end
	
	def new_pause_ticks
		0
	end
	
	def add_notification(text)
	 #boom!
	end
	
end

class NormalBallFactory

	attr_accessor :last_add_time

	def initialize
		@added_balls = 0
		@last_add_time = 0
	end

	def random_ball
		make_ball(NormalBall)
	end
	
	def last_to_pop
		nil
	end
	
	def make_ball(ball_class, color = 0)
		radius = ball_class.max_radius
	 	pos = vec2(2 * radius + rand((705-482) - 2*radius) + 482, 500) / 6.0
		
		instance = ball_class.new(pos, color)
		instance.body.vel = vec2(5*(rand*2 - 1), 0)
		
		@added_balls+=1
		
		return instance
	end
	
end

class BombBallFactory < NormalBallFactory

	def random_ball
		return make_ball(BombBall) if @added_balls % 6 == 0 
		return make_ball(NormalBall, 1) if @added_balls % 6 < 2 
		return make_ball(NormalBall, 0)
	end
	
end

class StormBallFactory < NormalBallFactory

	def random_ball
		return make_ball(StormBall) if @added_balls % 4 == 0 
		return make_ball(NormalBall)
	end
	
end


class StormToPopBallFactory < NormalBallFactory

	attr_accessor :last_to_pop

	def random_ball
		if @added_balls % 6 == 0 
			@last_to_pop = make_ball(StormBall)
			@last_add_time = 130
			return @last_to_pop
		end	

		return make_ball(NormalBall, rand(6))
	end
	
end

class BombToPopBallFactory < NormalBallFactory

	attr_accessor :last_to_pop

	def random_ball
		if @added_balls % 2 == 1 
			@last_to_pop = make_ball(BombBall)
			@last_add_time = 90
			return @last_to_pop
		end		
		return make_ball(NormalBall, rand(6))
	end
	
end

class WildCardPopBallFactory < NormalBallFactory

	attr_accessor :last_to_pop

	def random_ball
		if @added_balls % 5 == 1 || @added_balls % 5 == 4 
			@last_to_pop = make_ball(WildcardBall)
			@last_add_time = 90
			return @last_to_pop
		end		
		return make_ball(NormalBall, rand(6))
	end
	
end