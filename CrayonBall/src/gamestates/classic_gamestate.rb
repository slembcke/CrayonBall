class ClassicGameBoard < AbstractGameBoard
	def initialize(*args)
		super
				
		60.times{@ball_queue << random_ball(NormalBall)}
	end
	
	def update
		@bar.bar = ball_count
		super
		add_pieces_on_timer
		
		@gamestate_delegate.game_over if @balls.length > 120
	end
	
	def click_out_ball(ball)
		super
		4.times{add_ball(random_ball(NormalBall))}
	end
	
	# Add pieces to the gameboard occasionally. We pace on the elapsed time.
	def add_pieces_on_timer
		rate = rate(@gamestate_delegate.ticks/1800)
		
		if @gamestate_delegate.ticks%rate == 0 
			if rand < 0.20
				classes = AbstractBall::BallClasses
				@ball_queue << random_ball(classes[rand(classes.length)])
			else
				@ball_queue << random_ball(NormalBall)
			end
		end
		
	end
end

class ClassicGameStateDelegate < AbstractGameStateDelegate

	def initialize
		super(ClassicGameBoard.new(self))

		init_input
	end
	
	def mode
		:classic
	end
	
	def init_input
		@action_listener.action(AS::Event.source_for_mbutton(1)){|event|
			next unless event.state #ignore button up events
			
			coord = mouse_to_board(*event.pos)
			ball = @board.ball_at(coord)
			
			if ball # If there was a ball where the click happened
				if ball.clicked_on(@board)
					@board.click_out_ball(ball)
				end
			end
		}
	end
	
end
