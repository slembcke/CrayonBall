class GrabGameBoard < AbstractGameBoard
	attr_accessor :mouse_point
	
	def initialize(*args)
		super
		
		@overflow = 0
		
		@mouse_point = vec2(0,0)
		@grabbed_ball = nil
	end
	
	def group_cleared(group)
		super
		
		@overflow -= group.length
	end
	
	def click_out_ball(ball)
		super
		@overflow += 3
	end
	
	def update
		if @grabbed_ball
			point = @mouse_point
			@grabbed_ball.body.p = vec2(*clamp_mouse(point.x, point.y, @grabbed_ball.radius))
			@grabbed_ball.body.v = vec2(0,0)
		end
		
		return if super
		
		if (@balls.length + @ball_queue.length) < 80
			if rand < 0.03
				@ball_queue << random_ball(AbstractBall::BallClasses[rand(AbstractBall::BallClasses.length)])
			else
				@ball_queue << random_ball(NormalBall)
			end
		end

		@overflow = @overflow < 0 ? 0 : @overflow
		@gamestate_delegate.game_over if @overflow > 80
		
		@bar.bar = @overflow*120/80
		add_on_timer
	end
	
	def add_on_timer
		rate = (20.0*1.4**(-@ticks/3600.0)).to_i
		
		if @ticks%rate == 0
			@overflow += 1;
		end
	end
	
	CLICKABLE_TYPES = [LockBall, PushBall, WildcardBall]
	def click_at(coord)
		ball = ball_at(coord)
		return unless ball
		
		if CLICKABLE_TYPES.include?(ball.class)
			click_out_ball(ball) if ball.clicked_on(self)
		else
			@grabbed_ball = ball
		end
	end
	
	def remove_ball(ball)
		super
		
		@grabbed_ball = nil if @grabbed_ball == ball
	end
	
	MOUSEUPABLE_TYPES = [StormBall, BombBall]
	def unclick
		if @grabbed_ball
			if MOUSEUPABLE_TYPES.include?(@grabbed_ball.class)
				click_out_ball(@grabbed_ball) if @grabbed_ball.clicked_on(self)
			end
			
			@grabbed_ball = nil
		end
	end
end

class GrabGameStateDelegate < AbstractGameStateDelegate

	def initialize
		super(GrabGameBoard.new(self))
		
		init_input
	end
		
	def mode
		:grab
	end

	def init_input
		@action_listener.action(AS::Event.source_for_mbutton(1)){|event|
			if event.state #ignore button up events
				@board.click_at(mouse_to_board(*event.pos))
			else
				@board.unclick
			end
		}
		
		@action_listener.action(AS::Event.source_for_mouse){|event|
			coord = mouse_to_board(*event.pos)
			@board.mouse_point = coord
		}
	end

end
