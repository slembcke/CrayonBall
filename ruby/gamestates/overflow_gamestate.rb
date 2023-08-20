class OverflowGameBoard < AbstractGameBoard
	def initialize(*args)
		super
		
		@overflow = 0
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
		super
		
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
		rate = (200.0*1.15**(-@ticks/3600.0)).to_i
		
		if @ticks%rate == 0
			@overflow += 1;
		end
	end
end

class OverflowGameStateDelegate < AbstractGameStateDelegate

	def initialize
		super(OverflowGameBoard.new(self))
		
		init_input
	end
		
	def mode
		:overflow
	end

	def init_input
		
		@action_listener.action(AS::Event.source_for_mbutton(1)){|event|
			next unless event.state #ignore button up events
			
			coord = mouse_to_board(*event.pos)
			ball = @board.ball_at(coord)
			
			if ball and ball.clicked_on(@board)
				@board.click_out_ball(ball)
			end
		}
		
	end

end
